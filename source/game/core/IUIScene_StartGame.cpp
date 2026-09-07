#include "game/core/IUIScene_StartGame.hpp"

#include <algorithm>
#include <string>

#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/IUIPartial_OnlineRestricted.hpp"
#include "game/core/Minecraft.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/world/LevelGenerationOptions.hpp"

IUIScene_StartGame::IUIScene_StartGame(int sceneType, void*, UILayer* layer, bool customGame)
    : UIScene(sceneType, layer), m_bonusChestEnabled(false), m_bonusChestEnabledNet(false),
      m_bonusChestAllowed(false), m_loadDataFailed(false), m_hasSaveData(false), m_isTrial(false),
      m_isCreative(false), m_isBattle(false), m_isLobby(false), m_dlcMounting(false),
      m_touchBoxRebuild(false), m_signInPending(false), m_userIndex(0),
      m_selectedTexturePack(0), m_previousTexturePack(-1), m_pendingTexturePack(-1),
      m_customGameMode(0), m_texturePackId(0), m_texturePackState(0), m_textureSelectorState(-1),
      m_levelGenerationOptions(0), m_onlineRestricted(0)
{
    m_controls.push_back(&m_startButton);
    m_controls.push_back(&m_texturePackName);
    m_controls.push_back(&m_texturePackDescription);
    m_controls.push_back(&m_texturePackIcon);
    m_controls.push_back(&m_texturePackComparison);
    m_controls.push_back(&m_texturePackList);
    m_controls.push_back(&m_seedSlider);
    m_controls.push_back(&m_sizeSlider);
    m_controls.push_back(&m_bonusChest);
}

IUIScene_StartGame::~IUIScene_StartGame()
{
}

uint32_t IUIScene_StartGame::GetCustomGameModeID()
{
    return static_cast<uint32_t>(m_customGameMode);
}

uint32_t IUIScene_StartGame::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    if (m_movie == 0)
        return 0;
    fuiRenderNode* root = m_movie->getRootNode();
    m_startButton.setupControl(this, root, "MainPanel");
    m_texturePackName.setupControl(this, m_startButton.getFuiRenderNode(), "TexturePackName");
    m_texturePackDescription.setupControl(this, m_startButton.getFuiRenderNode(), "TexturePackDescription");
    m_texturePackIcon.setupControl(this, m_startButton.getFuiRenderNode(), "Icon");
    m_texturePackComparison.setupControl(this, m_startButton.getFuiRenderNode(), "ComparisonPic");
    return 1;
}

bool IUIScene_StartGame::SortTexturePacks(const TEXTUREPACKSORTSTRUCT& first,
                                          const TEXTUREPACKSORTSTRUCT& second)
{
    return second.values[0] < first.values[0];
}

void IUIScene_StartGame::UpdateTexturePackDescription(int packId)
{
    m_texturePackId = packId;
    m_previousTexturePack = m_selectedTexturePack;
}

void IUIScene_StartGame::OrganizeTexturePacks()
{
    m_texturePacks.clear();
    m_texturePackList.clearSlots();
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft == 0 || minecraft->mTexturePackRepository == 0)
        return;
    TexturePackRepository* repository = minecraft->mTexturePackRepository;
    int count = repository->getTexturePackCount();
    int i;
    for (i = 0; i < count; ++i)
    {
        TexturePackRepository::TexturePackIdNames* names = repository->getTexturePackIdNames();
        if (names != 0 && i < static_cast<int>(names->size()))
        {
            TEXTUREPACKSORTSTRUCT pack = {};
            pack.values[0] = i;
            pack.values[1] = static_cast<int>((*names)[i].first);
            m_texturePacks.push_back(pack);
            m_texturePackList.addPack(i, (*names)[i].second, false);
        }
    }
    std::sort(m_texturePacks.begin(), m_texturePacks.end(), SortTexturePacks);
    m_selectedTexturePack = 0;
}

void IUIScene_StartGame::_init()
{
    if (g_pApp != 0 && g_pApp->StartInstallDLCProcess(m_userIndex) != 0)
    {
        m_dlcMounting = true;
        return;
    }
    m_dlcMounting = false;
    if (m_levelGenerationOptions != 0 && m_levelGenerationOptions->isGameModeLobby())
        UpdateTexturePackDescription(0);
    else
        OrganizeTexturePacks();
}

uint64_t IUIScene_StartGame::RequestPrivMessageBox()
{
    int parameter = 0;
    gConsoleUIController.RequestAlertMessage(0x104F73E0, m_isTrial ? 0x456F1349 : 0x252AB13F,
        &parameter, 1, m_userIndex, 0, 0, 0, true);
    return 0;
}

void IUIScene_StartGame::FindAndJoinGame(unsigned int playerId)
{
    m_userIndex = static_cast<int>(playerId);
    CheckPermissionsAndContinue(false);
}

void IUIScene_StartGame::HostGame(unsigned int playerId)
{
    m_userIndex = static_cast<int>(playerId);
    CheckPermissionsAndContinue(false);
}

void IUIScene_StartGame::StartGame(unsigned int playerId)
{
    m_userIndex = static_cast<int>(playerId);
    LaunchGame();
}

uint32_t IUIScene_StartGame::StartGame_SignInReturned(void* context, bool signedIn, int userIndex)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0)
    {
        scene->m_signInPending = false;
        if (signedIn)
            scene->StartGame(static_cast<unsigned int>(userIndex));
        else
            scene->m_dlcMounting = false;
    }
    return 0;
}

void IUIScene_StartGame::tick()
{
    UIScene::tick();
    if (m_pendingTexturePack >= 0)
    {
        UpdateTexturePackDescription(m_pendingTexturePack);
        m_selectedTexturePack = m_pendingTexturePack;
        m_pendingTexturePack = -1;
    }
}

void IUIScene_StartGame::handleReload()
{
    if (m_selectedTexturePack >= 0)
        m_texturePackList.selectSlot(m_selectedTexturePack);
}

void IUIScene_StartGame::handleSelectionChanged(int selection)
{
    m_pendingTexturePack = selection;
}

void IUIScene_StartGame::handleSliderMove(int, int value)
{
    m_textureSelectorState = value;
}

void IUIScene_StartGame::handleTouchBoxRebuild()
{
    m_touchBoxRebuild = true;
}

void IUIScene_StartGame::handleTimerComplete(int timerId)
{
    if (m_touchBoxRebuild)
    {
        gConsoleUIController.TouchBoxRebuild(this);
        m_touchBoxRebuild = false;
    }
    if (timerId == 0)
    {
        m_bonusChestAllowed = GameSettings::GetGameSettings(m_userIndex, static_cast<eGameSetting>(19)) != 0;
        m_bonusChest.SetEnable(m_bonusChestAllowed);
        m_bonusChest.setChecked(m_bonusChestEnabled);
    }
}

void IUIScene_StartGame::HandleDLCMountingComplete()
{
    m_dlcMounting = false;
    OrganizeTexturePacks();
    UpdateTexturePackDescription(m_selectedTexturePack);
    gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

void IUIScene_StartGame::UpdateCurrentTexturePack(int packIndex)
{
    m_selectedTexturePack = packIndex;
    if (packIndex >= 0 && packIndex < static_cast<int>(m_texturePacks.size()))
        m_texturePackId = m_texturePacks[packIndex].values[1];
}

uint32_t IUIScene_StartGame::TrialTexturePackWarningReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0 && result != C4JStorage::EMessageResult_4)
        scene->m_dlcMounting = false;
    return 0;
}

uint64_t IUIScene_StartGame::UnlockTexturePackReturned(void* context, int, C4JStorage::EMessageResult)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0)
        scene->m_dlcMounting = false;
    return 0;
}

uint32_t IUIScene_StartGame::TexturePackDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0 && result == C4JStorage::EMessageResult_4)
        scene->UpdateCurrentTexturePack(scene->m_selectedTexturePack);
    return 0;
}

uint32_t IUIScene_StartGame::TexturePackDialogBattleMiniGameReturned(void* context, int userIndex,
                                                                 C4JStorage::EMessageResult result)
{
    return TexturePackDialogReturned(context, userIndex, result);
}

void IUIScene_StartGame::CheckPermissionsAndContinue(bool fromLoad)
{
    if (m_onlineRestricted != 0 && m_onlineRestricted->IsUGCRestrictedMode())
    {
        m_dlcMounting = false;
        gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, 0, 0);
        return;
    }
    if (fromLoad)
        StartGame(static_cast<unsigned int>(m_userIndex));
    else
        StartSharedLaunchFlow();
}

uint64_t IUIScene_StartGame::WiiUDRCWarningReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0 && result == C4JStorage::EMessageResult_4)
        scene->CheckPermissionsAndContinue(false);
    return 0;
}

void IUIScene_StartGame::StartSharedLaunchFlow()
{
    m_signInPending = true;
}

uint32_t IUIScene_StartGame::DeleteSaveDataReturned(void* context, bool)
{
    if (g_pApp != 0)
        g_pApp->SetCorruptSaveDeleted(true);
    UIScene* scene = static_cast<UIScene*>(context);
    if (scene != 0)
        scene->navigateBack();
    return 0;
}

uint32_t IUIScene_StartGame::DeleteSaveDialogReturned(void* context, int userIndex,
                                                  C4JStorage::EMessageResult result)
{
    if (result == C4JStorage::EMessageResult_4)
        DeleteSaveDataReturned(context, true);
    return 0;
}

uint32_t IUIScene_StartGame::LoadDataComplete(void* context)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0)
        scene->CheckPermissionsAndContinue(true);
    return 0;
}

uint32_t IUIScene_StartGame::LoadSaveDataReturned(void* context, bool loaded, bool complete)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0)
    {
        scene->m_hasSaveData = loaded;
        if (complete)
            LoadDataComplete(context);
        else
            scene->m_dlcMounting = false;
    }
    return 0;
}

uint32_t IUIScene_StartGame::CheckCreativeReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (scene != 0 && result == C4JStorage::EMessageResult_4)
        scene->CheckPermissionsAndContinue(true);
    return 0;
}

void IUIScene_StartGame::LaunchGame()
{
    CheckCreativeReturned(this, m_userIndex, C4JStorage::EMessageResult_4);
}

int IUIScene_StartGame::getCurrentTextureSelectorIndex(int packId)
{
    int i;
    for (i = 0; i < static_cast<int>(m_texturePacks.size()); ++i)
        if (m_texturePacks[i].values[1] == packId)
            return i;
    return 0;
}
