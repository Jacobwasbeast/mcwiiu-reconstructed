#include "game/core/IUIScene_StartGame.hpp"

#include <algorithm>
#include <cwchar>
#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/DLCTexturePack.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/DLCPack.hpp"
#include "game/core/Difficulty.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/RiderPositionOverrides.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/core/System.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/cDLCInfo.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/minigame/MiniGameOptionCache.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/util/Random.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRules.hpp"
#include "game/world/LevelRuleset.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "platform/wiiu/cCommerceNintendoWiiU.hpp"
#include "platform/wiiu/cNintendoDLCInfo.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_NetworkGameInitData.hpp"

extern CProfile* gProfile;
extern MiniGameOptionCache gMiniGameOptionCache;

namespace
{
struct StartGameSceneData
{
    int reserved0;
    int saveIndex;
    LevelGenerationOptions* generationOptions;
    int reserved12;
    EMiniGameId miniGameId;
};

struct SignInRequestData
{
    int (*callback)(void*, bool, int);
    void* context;
    bool onlineGame;
    bool reserved;
};

struct FullscreenProgressState
{
    bool showConfirmation;
    bool unknownState1;
    bool unknownState2;
    bool showTip;
    int closeMode;
    int playerId;
    EUIScene backScene;
};
}

IUIScene_StartGame::IUIScene_StartGame(int sceneType, void* data, UILayer* layer, bool customGame)
    : UIScene(sceneType, layer), m_texturePackList(), m_mainPanel(), m_texturePackName(),
      m_texturePackDescription(), m_texturePackIcon(), m_texturePackComparison(),
      m_difficultySlider(), m_gameModeSlider(), m_onlineGameCheckBox(), m_launchData(),
      m_actionPending(false), m_texturePackParentPack(NULL), m_levelGenerationOptions(NULL),
      m_texturePacks(), m_worldName(), m_difficulty(0), m_state614(0), m_gameTypeId(0),
      m_saveIndex(0), m_selectedSlot(0), m_customGame(customGame), m_saveCorrupt(false),
      m_signInPending(false), m_onlineEnabled(false), m_touchBoxRebuild(false), m_state629(true),
      m_isCreative(false), m_state62B(false), m_state62C(false), m_pendingSlot(-1), m_seed(0)
{
    const StartGameSceneData* sceneData = static_cast<const StartGameSceneData*>(data);
    m_worldName = L"";
    m_gameTypeId = GameType::SURVIVAL->getId();
    m_launchData.mFlag14 = true;
    m_launchData.mValue18 = sceneType;
    m_levelGenerationOptions = sceneData->generationOptions;
    m_saveIndex = sceneData->saveIndex;
    m_launchData.mFlagB = true;
    m_launchData.mFlag15 = true;
    m_launchData.mFlag3 = false;
    m_launchData.mFlagF = true;
    m_launchData.mFlag6 = false;
    m_launchData.mFlag13 = true;
    m_launchData.mFlag11 = true;
    m_launchData.mFlag10 = false;
    m_launchData.mFlag4 = true;
    m_launchData.mFlag5 = false;
    m_launchData.mFlag12 = true;
    m_launchData.mFlagC = false;
    m_launchData.mFlagA = true;
    m_launchData.mFlag8 = true;
    m_launchData.mFlag9 = true;

    unsigned int miniGameId = static_cast<unsigned int>(sceneData->miniGameId);
    if (miniGameId == 15 || m_levelGenerationOptions == NULL)
    {
        if (m_customGame && m_levelGenerationOptions != NULL)
        {
            m_launchData.mValue44 = 0;
        }
    }
    else
    {
        m_launchData.mFlag56 = true;
        unsigned int ruleType = 0;
        switch (miniGameId)
        {
        case 1:
            ruleType = 1;
            break;
        case 2:
            ruleType = 2;
            break;
        case 3:
            ruleType = 3;
            break;
        case 4:
            ruleType = 4;
            break;
        }
        gApp.getLevelRules()->getRulesByType(static_cast<LevelRules::ELevelRuleType>(ruleType),
                                            &m_launchData.mLevelRulesetGroups, true);
        if (miniGameId != 0)
        {
            unsigned int mapSelection = 0xFFFFFFFFU;
            if (miniGameId == 1)
            {
                mapSelection = GameSettings::GetBattleMiniGameMapSelection(m_playerId);
            }
            else if (miniGameId == 2)
            {
                mapSelection = GameSettings::GetTumbleMiniGameMapSelection();
            }
            else if (miniGameId == 3)
            {
                mapSelection = GameSettings::GetGlideMiniGameMapSelection(m_playerId);
            }
            if (mapSelection != 0xFFFFFFFFU)
            {
                std::vector<LevelRules::LevelRulesetGroup*>::iterator it =
                    m_launchData.mLevelRulesetGroups.begin();
                while (it != m_launchData.mLevelRulesetGroups.end())
                {
                    LevelRules::LevelRulesetGroup* group = *it;
                    LevelGenerationOptions* options = group->rulesets.front()->getGenerationOptions();
                    bool keep = (mapSelection & (1U << options->getThemeId())) != 0;
                    if (keep && !options->getPack()->hasPurchasedPack())
                    {
                        keep = gApp.mDLCManager.IsMiniGameTemporarilyFree(
                            static_cast<int>(group->rulesets.front()->getRuleType()));
                    }
                    if (keep)
                    {
                        ++it;
                    }
                    else
                    {
                        it = m_launchData.mLevelRulesetGroups.erase(it);
                    }
                }
                if (m_launchData.mLevelRulesetGroups.empty())
                {
                    m_launchData.mFlag56 = true;
                    gApp.getLevelRules()->getRulesByType(
                        static_cast<LevelRules::ELevelRuleType>(sceneData->miniGameId),
                        &m_launchData.mLevelRulesetGroups, true);
                }
            }
        }
        MiniGameDef::SetPlayerSettings(
            gMiniGameOptionCache.getPreviousOptions(m_playerId, sceneData->miniGameId));
    }

    unsigned int onlineSetting =
        GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x13));
    m_launchData.mFlag17 = false;
    bool allowedOnline = false;
    if (gProfile->IsSignedInLive(m_playerId))
    {
        if (gProfile->AllowedToPlayMultiplayer(m_playerId))
        {
            allowedOnline = true;
        }
    }
    m_onlineEnabled = allowedOnline;
    if (allowedOnline)
    {
        m_launchData.mFlag0 = onlineSetting != 0;
        if (onlineSetting != 0)
        {
            m_launchData.mFlag1 =
                GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x14)) != 0;
            m_launchData.mFlag2 =
                GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x15)) != 0;
            m_selectedSlot = 0;
            addTimer(0, 100);
        }
        else
        {
            m_selectedSlot = 0;
            m_launchData.mFlag2 = false;
            m_launchData.mFlag1 = false;
            addTimer(0, 100);
        }
    }
    else
    {
        m_launchData.mFlag1 = false;
        m_launchData.mFlag2 = false;
        m_launchData.mFlag0 = false;
        if (onlineSetting != 0)
        {
            m_launchData.mFlag17 = true;
        }
        m_selectedSlot = 0;
        addTimer(0, 100);
    }
}

IUIScene_StartGame::~IUIScene_StartGame()
{
}

unsigned int IUIScene_StartGame::GetCustomGameModeID()
{
    return m_launchData.mValue5C;
}

unsigned int IUIScene_StartGame::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    m_mainPanel.setupControl(this, root, "MainPanel");
    m_controls.push_back(&m_mainPanel);
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_texturePackName.setupControl(this, panel, "TexturePackName");
    m_controls.push_back(&m_texturePackName);
    m_texturePackDescription.setupControl(this, panel, "TexturePackDescription");
    m_controls.push_back(&m_texturePackDescription);
    m_texturePackIcon.setupControl(this, panel, "Icon");
    m_controls.push_back(&m_texturePackIcon);
    m_texturePackComparison.setupControl(this, panel, "ComparisonPic");
    m_controls.push_back(&m_texturePackComparison);
    return 1;
}

bool IUIScene_StartGame::SortTexturePacks(const TEXTUREPACKSORTSTRUCT& first,
                                          const TEXTUREPACKSORTSTRUCT& second)
{
    return second.sortValue < first.sortValue;
}

void IUIScene_StartGame::UpdateTexturePackDescription(int packId)
{
    if (!m_launchData.mFlag56 && !m_launchData.mFlag57)
    {
        TexturePack* pack =
            Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(packId);
        if (pack != NULL)
        {
            m_texturePackName.setLabel(UIString(pack->getName()), false, false);
            m_texturePackDescription.setLabel(UIString(pack->getDesc1()), false, false);
            unsigned int size = 0;
            wchar_t buffer[64];
            pack->getPackIcon(size);
            swprintf(buffer, 0x40, L"tpack%08x", pack->getId());
            m_texturePackIcon.setTextureName(buffer, true);
            const void* comparison = pack->getPackComparison(size);
            if (size == 0 || comparison == NULL)
            {
                m_texturePackComparison.setTextureName(L"", true);
            }
            else
            {
                swprintf(buffer, 0x40, L"texturePackComparison%08x", pack->getId());
                registerSubstitutionTexture(
                    buffer, reinterpret_cast<unsigned char*>(const_cast<void*>(comparison)), size,
                    false);
                m_texturePackComparison.setTextureName(buffer, true);
            }
        }
    }
    else
    {
        Minecraft::GetInstance()->GetMiniGame();
        std::wstring image = MiniGameDef::getComparisonImageForGameMode(
            static_cast<EMiniGameId>(GetCustomGameModeID()));
        if (!gApp.hasArchiveFile(image, false))
        {
            m_texturePackComparison.setTextureName(L"", true);
        }
        else
        {
            arrayWithLength<unsigned char> file = gApp.getArchiveFile(image, false);
            if (file.length != 0 && file.data != NULL)
            {
                registerSubstitutionTexture(image, file.data, file.length, true);
            }
            m_texturePackComparison.setTextureName(image, true);
        }
    }
}

void IUIScene_StartGame::OrganizeTexturePacks()
{
    TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
    int count = repository->getTexturePackCount();
    TEXTUREPACKSORTSTRUCT empty;
    empty.sortValue = 0;
    empty.packId = 0;
    m_texturePacks.resize(count, empty);
    m_texturePacks[0].sortValue = 1000;
    m_texturePacks[0].packId = 0;
    int sortValue = 999;
    int index;
    for (index = 1; index < count; ++index)
    {
        TexturePack* pack = repository->getTexturePackByIndex(index);
        unsigned int packId = pack->getId();
        DLC_INFO* info = gApp.GetDLCInfoInstance()->GetDLCInfoForPackID(packId);
        if (info != NULL)
        {
            m_texturePacks[index].sortValue = info->SortIndex;
            m_texturePacks[index].packId = packId;
        }
        else
        {
            DLCPack* parent = static_cast<DLCTexturePack*>(pack)->getDLCInfoParentPack();
            m_texturePacks[index].sortValue = sortValue;
            --sortValue;
            m_texturePacks[index].packId = parent->GetPackId();
        }
    }
    std::sort(m_texturePacks.begin(), m_texturePacks.end(), SortTexturePacks);
    for (index = 0; index < count; ++index)
    {
        TexturePack* pack = repository->getTexturePackById(m_texturePacks[index].packId);
        if (m_texturePacks[index].packId == m_launchData.mValue1C)
        {
            m_selectedSlot = index;
        }
        unsigned int size = 0;
        const void* icon = pack->getPackIcon(size);
        if (size != 0 && icon != NULL)
        {
            wchar_t buffer[64];
            swprintf(buffer, 0x40, L"tpack%08x", pack->getId());
            registerSubstitutionTexture(
                buffer, reinterpret_cast<unsigned char*>(const_cast<void*>(icon)), size, false);
            m_texturePackList.addPack(index, buffer, false);
        }
    }
    UpdateTexturePackDescription(m_launchData.mValue1C);
    m_texturePackList.selectSlot(m_selectedSlot);
}

void IUIScene_StartGame::_init()
{
    if (gApp.StartInstallDLCProcess(m_playerId) != 0)
    {
        m_actionPending = true;
        return;
    }
    m_actionPending = false;
    if (m_levelGenerationOptions != NULL && m_levelGenerationOptions->isGameModeLobby())
    {
        UpdateTexturePackDescription(0);
        return;
    }
    OrganizeTexturePacks();
}

void IUIScene_StartGame::RequestPrivMessageBox()
{
    int parameters[1];
    parameters[0] = -0x465E40B3;
    if (m_launchData.mFlag57)
    {
        gConsoleUIController.RequestAlertMessage(0x456F1349, 0x2FAC754A, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return;
    }
    gConsoleUIController.RequestAlertMessage(0x252AB13F, -0x3B844EA3, parameters, 1,
                                             gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
}

void IUIScene_StartGame::FindAndJoinGame(unsigned int playerMask)
{
    EMiniGameId miniGameId = static_cast<EMiniGameId>(m_launchData.mValue5C);
    if (m_launchData.mValue5C != 15)
    {
        gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1f), 1);
        gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(
            MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_launchData.mValue5C),
                                               true)
                ->GetId()));
    }
    unsigned int* values = NULL;
    unsigned int* ids = NULL;
    unsigned int count =
        MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_launchData.mValue5C), true)
            ->CalculateJoiningMatchmakingData(&m_launchData, &values, &ids);
    unsigned int result = CGameNetworkManager::sInstance.FindAndJoinGame(
        static_cast<int>(playerMask), miniGameId, count, values, ids);
    delete values;
    delete ids;
    gApp.SetLiveLinkRequired(false);
    if (result == 0)
    {
        telemetryManagerInstance->PrepareMiniGameHostOptions(m_playerId, &m_launchData,
                                                             eTelemetryJoinMethod_Matchmaking);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        return;
    }
    if (result < 2)
    {
        gConsoleUIController.NavigateBack(m_playerId, false, static_cast<EUIScene>(0x53),
                                          static_cast<EUILayer>(8));
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        return;
    }
    int parameters[1];
    parameters[0] = -0x465E40B3;
    if (result == 2)
    {
        gConsoleUIController.RequestErrorMessage(0x4BB9D6AE, -0x479419FD, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        return;
    }
    if (result < 4)
    {
        gConsoleUIController.RequestErrorMessage(0x4BB9D6AE, -0x554D4923, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        return;
    }
    if (result == 4)
    {
        gConsoleUIController.RequestErrorMessage(0x252AB13F, -0x33A5A6F1, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        return;
    }
    gConsoleUIController.NavigateBack(m_playerId, false, static_cast<EUIScene>(0x53),
                                      static_cast<EUILayer>(8));
    Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
}

void IUIScene_StartGame::HostGame(unsigned int playerMask)
{
    _NetworkGameInitData* initData = new _NetworkGameInitData();
    if (m_customGame)
    {
        if (m_levelGenerationOptions == NULL)
        {
            initData->m_flag2 = 1;
            gApp.ClearTerrainFeaturePosition();
        }
        else
        {
            gStorage->ResetSaveData();
            gStorage->SetSaveTitle(m_levelGenerationOptions->getDefaultSaveName().c_str());
            gApp.ClearTerrainFeaturePosition();
        }
    }
    else
    {
        std::wstring saveName;
        saveName.assign(m_worldName, 0, std::wstring::npos);
        gStorage->ResetSaveData();
        gStorage->SetSaveTitle(saveName.c_str());
        std::wstring seedText;
        if (m_launchData.mDisplayName.size() == 0)
        {
            seedText.assign(L"", wcslen(L""));
        }
        else
        {
            seedText.assign(m_launchData.mDisplayName, 0, std::wstring::npos);
        }
        if (seedText.size() == 0)
        {
            if (m_launchData.mFlag40)
            {
                initData->m_flag0 = 1;
            }
            else
            {
                Random random(System::processTimeInNanoSecs());
                m_seed = random.nextLong();
            }
        }
        else
        {
            bool numeric = true;
            unsigned int i;
            for (i = 0; i < seedText.size(); ++i)
            {
                wchar_t character = seedText.at(i);
                if ((character < L'0' || character > L'9') &&
                    (i != 0 || seedText.at(0) != L'-'))
                {
                    numeric = false;
                    break;
                }
            }
            if (numeric)
            {
                m_seed = _fromString<long long>(seedText);
            }
            else
            {
                int hash = 0;
                for (i = 0; i < seedText.size(); ++i)
                {
                    hash = hash * 31 + seedText.at(i);
                }
                m_seed = hash;
            }
        }
        gApp.ClearTerrainFeaturePosition();
    }

    bool onlineGame = false;
    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()) &&
        (m_launchData.mFlag0 || m_launchData.mFlag58) && !m_launchData.mFlag68)
    {
        onlineGame = true;
    }
    initData->m_seed = m_seed;
    initData->m_value5 = m_levelGenerationOptions;
    initData->m_value6 = m_launchData.mValue1C;
    initData->m_value2 = 0;
    initData->m_value9 = m_gameTypeId;
    Minecraft::GetInstance()->mTexturePackRepository->selectTexturePackById(m_launchData.mValue1C,
                                                                            false);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0), m_difficulty);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(3), m_launchData.mFlag2);
    gApp.SetGameHostOption(
        static_cast<eGameHostOption>(4),
        GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0xc)));
    gApp.SetGameHostOption(static_cast<eGameHostOption>(6), m_gameTypeId);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(7), m_launchData.mFlag5);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(8), m_launchData.mFlag4);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(9), m_launchData.mFlag6);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0xb), m_launchData.mFlag8);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0xc), m_launchData.mFlag9);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0xe), m_launchData.mFlagA);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0xd), m_launchData.mFlagB);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x10), m_launchData.mFlagC);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x11), m_launchData.mFlagC);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x12), m_launchData.mFlagC);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x18), m_launchData.mFlagF);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x19), m_launchData.mFlag10);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1a), m_launchData.mFlag11);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1b), m_launchData.mFlag12);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1c), m_launchData.mFlag13);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1d), m_launchData.mFlag14);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1e), m_launchData.mFlag15);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(0x15), m_launchData.mFlag16);
    if (m_launchData.mFlag5)
    {
        initData->m_value10 = new SuperflatConfig(m_launchData.mSuperflatConfig);
    }
    if (!m_customGame)
    {
        initData->m_value7 = 0x36;
        initData->m_mode = 3;
        initData->m_flag1 = m_launchData.mFlagD4;
        initData->m_value11 = m_launchData.mValueD8;
        initData->m_value12 = m_launchData.mValueDC;
    }
    gApp.SetResetNether(m_launchData.mFlagD);
    gApp.SetResetEnd(m_launchData.mFlagE);
    gApp.SetGameHostOption(static_cast<eGameHostOption>(6), m_gameTypeId);
    unsigned char gameType = static_cast<unsigned char>(m_gameTypeId);
    if (m_launchData.mFlag56 || m_launchData.mFlag57)
    {
        gApp.SetGameHostOption(static_cast<eGameHostOption>(0x1f), 1);
        gameType = static_cast<unsigned char>(GameType::LOBBY->getId());
    }
    EMiniGameId hostMiniGameId = static_cast<EMiniGameId>(15);
    if (m_launchData.mValue5C != 15)
    {
        hostMiniGameId = static_cast<EMiniGameId>(m_launchData.mValue5C);
    }
    unsigned int matchmakingCount = 0;
    unsigned int* matchmakingValues = NULL;
    if (m_launchData.mValue5C != 15)
    {
        MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
            static_cast<EMiniGameId>(m_launchData.mValue5C), true);
        gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(definition->GetId()));
        Minecraft::GetInstance()->GetClientMasterGameMode()->Setup(
            static_cast<EMiniGameId>(definition->GetId()));
        definition->SetGameSettingsFromMatchmakingFilter(&m_launchData);
        if (m_launchData.mFlag58)
        {
            matchmakingCount =
                definition->CalculateHostingMatchmakingData(&m_launchData, &matchmakingValues);
        }
        initData->m_ruleGroups.insert(initData->m_ruleGroups.begin(),
                                      m_launchData.mLevelRulesetGroups.begin(),
                                      m_launchData.mLevelRulesetGroups.end());
    }
    CGameNetworkManager::sInstance.HostGame(static_cast<int>(playerMask), onlineGame,
                                            m_launchData.mFlag1, gameType, hostMiniGameId,
                                            matchmakingCount, matchmakingValues);
    delete matchmakingValues;
    initData->m_value3 = gApp.GetGameHostOption(static_cast<eGameHostOption>(0x20));
    initData->m_value4 =
        MiniGameDef::GetCustomGameModeById(static_cast<EMiniGameId>(m_launchData.mValue5C), true)
            ->GetHostOption(static_cast<eCustomGameModeHostOption>(10));
    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_firstParameter = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_secondParameter = initData;
    if (m_launchData.mValue5C != 15)
    {
        params->m_timeout = m_launchData.mValue5C;
    }
    gApp.SetAutosaveTimerTime();
    FullscreenProgressState* state = new FullscreenProgressState();
    state->showConfirmation = false;
    state->backScene = static_cast<EUIScene>(0x53);
    state->showTip = true;
    state->unknownState1 = true;
    state->unknownState2 = true;
    state->closeMode = 3;
    params->m_thirdParameter = state;
    state->playerId = 0;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(4), params,
                                         static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    telemetryManagerInstance->PrepareMiniGameHostOptions(gProfile->GetPrimaryPad(), &m_launchData,
                                                         eTelemetryJoinMethod_Host);
    Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
}

void IUIScene_StartGame::StartGame(unsigned int playerMask)
{
    if (m_launchData.mFlag57)
    {
        FindAndJoinGame(playerMask);
        return;
    }
    HostGame(playerMask);
}

int IUIScene_StartGame::StartGame_SignInReturned(void* context, bool signedIn, int)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (!signedIn)
    {
        scene->m_actionPending = false;
        return 0;
    }
    if (!gProfile->IsSignedIn(scene->m_playerId))
    {
        return 0;
    }
    int primaryPad = gProfile->GetPrimaryPad();
    bool multiplayerBlocked = false;
    unsigned int playerMask = 0;
    bool allSignedInLive = gProfile->IsSignedInLive(primaryPad);
    bool onlineGame = false;
    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()) &&
        (scene->m_launchData.mFlag0 || scene->m_launchData.mFlag58) &&
        !scene->m_launchData.mFlag68)
    {
        onlineGame = true;
    }
    bool localMultiplayer = gApp.IsLocalMultiplayerAvailable() != 0;
    int index;
    for (index = 0; index < 4; ++index)
    {
        if (gProfile->IsSignedIn(index) && (localMultiplayer || index == primaryPad))
        {
            if (allSignedInLive)
            {
                gProfile->IsSignedInLive(index);
            }
            if (!gProfile->AllowedToPlayMultiplayer(index))
            {
                multiplayerBlocked = true;
            }
            playerMask |= CGameNetworkManager::GetLocalPlayerMask(index);
            allSignedInLive = allSignedInLive && gProfile->IsSignedInLive(index);
        }
    }
    if (onlineGame && !allSignedInLive)
    {
        scene->m_actionPending = false;
        int parameters[1];
        parameters[0] = -0x465E40B3;
        gConsoleUIController.RequestAlertMessage(-0x75DE76B5, -0x47C8FDB2, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return 0;
    }
    bool ugcRestricted = scene->IsUGCRestrictedMode();
    if (!allSignedInLive || !onlineGame || (!multiplayerBlocked && !ugcRestricted))
    {
        scene->StartGame(playerMask);
        return 0;
    }
    scene->m_actionPending = false;
    scene->setVisible(true);
    if (!ugcRestricted)
    {
        scene->RequestPrivMessageBox();
        return 0;
    }
    gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
    return 0;
}

void IUIScene_StartGame::tick()
{
    UIScene::tick();
    if (m_pendingSlot >= 0)
    {
        UpdateTexturePackDescription(m_texturePacks[m_pendingSlot].packId);
        int slot = m_pendingSlot;
        m_pendingSlot = -1;
        m_selectedSlot = slot;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_SCROLL);
    }
    if (m_signInPending)
    {
        SignInRequestData request;
        request.callback = StartGame_SignInReturned;
        request.context = this;
        request.onlineGame = m_launchData.mFlag0;
        request.reserved = false;
        m_signInPending = false;
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(),
                                             static_cast<EUIScene>(0x4a), &request,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    }
}

void IUIScene_StartGame::handleReload()
{
    if (m_selectedSlot != -1)
    {
        UpdateTexturePackDescription(m_texturePacks[m_selectedSlot].packId);
        m_texturePackList.selectSlot(m_selectedSlot);
    }
}

void IUIScene_StartGame::handleSelectionChanged(int selection)
{
    m_pendingSlot = selection;
}

void IUIScene_StartGame::handleSliderMove(int id, int value)
{
    if (id == 1)
    {
        m_gameModeSlider.handleSliderMove(value);
        if (value == 0)
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(-0x64587D5A)), false, false);
            m_isCreative = false;
            m_gameTypeId = GameType::SURVIVAL->getId();
            return;
        }
        if (value == 1)
        {
            m_gameModeSlider.setLabel(UIString(L10N::GetString(0x12D696DD)), false, false);
            m_gameTypeId = GameType::CREATIVE->getId();
            m_isCreative = true;
            return;
        }
        if (value != 2)
        {
            return;
        }
        m_gameModeSlider.setLabel(UIString(L10N::GetString(-0x066EACA8)), false, false);
        m_isCreative = false;
        m_gameTypeId = GameType::ADVENTURE->getId();
        return;
    }
    if (id == 2)
    {
        m_difficulty = value;
        m_difficultySlider.handleSliderMove(value);
        if (!m_customGame)
        {
            GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(3),
                                          static_cast<unsigned char>(value));
        }
        wchar_t buffer[256];
        swprintf(buffer, 0x100, L"%ls: %ls", L10N::GetString(-0x2CDBBD73),
                 L10N::GetString(Difficulty::byId(value)->getKey()));
        m_difficultySlider.setLabel(UIString(buffer), false, false);
    }
}

void IUIScene_StartGame::handleTouchBoxRebuild()
{
    m_touchBoxRebuild = true;
}

void IUIScene_StartGame::handleTimerComplete(int timerId)
{
    if (m_touchBoxRebuild)
    {
        GetMainPanel()->UpdateControl();
        gConsoleUIController.TouchBoxRebuild(this);
        m_touchBoxRebuild = false;
    }
    if (timerId == 0)
    {
        bool allowedOnline = false;
        if (gProfile->IsSignedInLive(m_playerId) &&
            gProfile->AllowedToPlayMultiplayer(m_playerId))
        {
            allowedOnline = true;
        }
        if (allowedOnline != m_onlineEnabled)
        {
            if (!allowedOnline)
            {
                m_launchData.mFlag2 = false;
                m_launchData.mFlag1 = false;
                m_launchData.mFlag0 = false;
                m_onlineGameCheckBox.SetEnable(false);
                m_onlineGameCheckBox.setChecked(m_launchData.mFlag0);
                m_onlineEnabled = false;
            }
            else
            {
                unsigned int onlineSetting =
                    GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x13));
                m_launchData.mFlag0 = onlineSetting != 0;
                if (onlineSetting != 0)
                {
                    m_launchData.mFlag1 =
                        GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x14)) !=
                        0;
                    m_launchData.mFlag2 =
                        GameSettings::GetGameSettings(m_playerId, static_cast<eGameSetting>(0x15)) !=
                        0;
                    m_onlineGameCheckBox.SetEnable(allowedOnline);
                    m_onlineGameCheckBox.setChecked(m_launchData.mFlag0);
                    m_onlineEnabled = allowedOnline;
                }
                else
                {
                    m_launchData.mFlag2 = false;
                    m_launchData.mFlag1 = false;
                    m_onlineGameCheckBox.SetEnable(allowedOnline);
                    m_onlineGameCheckBox.setChecked(m_launchData.mFlag0);
                    m_onlineEnabled = allowedOnline;
                }
            }
        }
    }
}

void IUIScene_StartGame::HandleDLCMountingComplete()
{
    if ((m_levelGenerationOptions == NULL || !m_levelGenerationOptions->isGameModeLobby()) &&
        m_texturePacks.empty())
    {
        OrganizeTexturePacks();
        m_selectedSlot = 0;
    }
    else
    {
        TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
        m_texturePackList.clearSlots();
        int count = repository->getTexturePackCount();
        int index;
        for (index = 0; index < count; ++index)
        {
            TexturePack* pack = repository->getTexturePackByIndex(index);
            unsigned int size = 0;
            const void* icon = pack->getPackIcon(size);
            if (size != 0 && icon != NULL)
            {
                wchar_t buffer[64];
                swprintf(buffer, 0x40, L"tpack%08x", pack->getId());
                registerSubstitutionTexture(
                    buffer, reinterpret_cast<unsigned char*>(const_cast<void*>(icon)), size, false);
                m_texturePackList.addPack(index, buffer, false);
            }
        }
        m_state614 = 0;
        m_selectedSlot = 0;
    }
    if (m_texturePacks.empty())
    {
        UpdateTexturePackDescription(0);
    }
    else
    {
        UpdateTexturePackDescription(m_texturePacks[0].packId);
    }
    m_texturePackList.selectSlot(m_selectedSlot);
    m_actionPending = false;
    gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

void IUIScene_StartGame::UpdateCurrentTexturePack(int slot)
{
    m_selectedSlot = slot;
    TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
        m_texturePacks[slot].packId);
    if (pack != NULL)
    {
        m_launchData.mValue1C = pack->getId();
    }
}

int IUIScene_StartGame::TrialTexturePackWarningReturned(void* context, int,
                                                        C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_4)
    {
        scene->checkStateAndStartGame();
        return 0;
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::UnlockTexturePackReturned(void* context, int userIndex,
                                                   C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_4)
    {
        gProfile->IsSignedIn(userIndex);
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::TexturePackDialogReturned(void* context, int userIndex,
                                                   C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_4 && gProfile->IsSignedIn(userIndex))
    {
        int parameters[1];
        parameters[0] = -0x465E40B3;
        if (!gProfile->IsSignedInLive(userIndex))
        {
            gConsoleUIController.RequestErrorMessage(-0x75DE76B5, -0x3CB3F489, parameters, 1,
                                                     userIndex, NULL, NULL, NULL, true);
        }
        else if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(scene->m_launchData.mValue1C))
        {
            gConsoleUIController.RequestErrorMessage(-0x625FB562, -0x345A830D, parameters, 1,
                                                     userIndex, NULL, NULL, NULL, true);
        }
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::TexturePackDialogBattleMiniGameReturned(void* context, int userIndex,
                                                                 C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_4 && gProfile->IsSignedIn(userIndex))
    {
        int parameters[1];
        parameters[0] = -0x465E40B3;
        if (!gProfile->IsSignedInLive(userIndex))
        {
            gConsoleUIController.RequestErrorMessage(-0x75DE76B5, -0x3CB3F489, parameters, 1,
                                                     userIndex, NULL, NULL, NULL, true);
        }
        else if (!gApp.GetCommerceInstance()->PurchaseDLCByPackID(scene->m_launchData.mValue1C))
        {
            gConsoleUIController.RequestErrorMessage(-0x625FB562, -0x345A830D, parameters, 1,
                                                     userIndex, NULL, NULL, NULL, true);
        }
    }
    scene->m_actionPending = false;
    return 0;
}

void IUIScene_StartGame::CheckPermissionsAndContinue(bool fromLoad)
{
    int primaryPad = gProfile->GetPrimaryPad();
    bool multiplayerBlocked = false;
    bool allSignedInLive = true;
    bool onlineGame = false;
    if ((m_launchData.mFlag0 || m_launchData.mFlag58) && !m_launchData.mFlag68)
    {
        onlineGame = true;
    }
    bool localMultiplayer = gApp.IsLocalMultiplayerAvailable() != 0;
    int index;
    for (index = 0; index < 4; ++index)
    {
        if (gProfile->IsSignedIn(index) && (localMultiplayer || index == primaryPad))
        {
            if (allSignedInLive)
            {
                gProfile->IsSignedInLive(index);
            }
            if (!gProfile->AllowedToPlayMultiplayer(index))
            {
                multiplayerBlocked = true;
            }
            allSignedInLive = allSignedInLive && gProfile->IsSignedInLive(index);
        }
    }
    if (onlineGame && !allSignedInLive)
    {
        m_actionPending = false;
        int parameters[1];
        parameters[0] = -0x465E40B3;
        gConsoleUIController.RequestAlertMessage(-0x75DE76B5, -0x47C8FDB2, parameters, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return;
    }
    bool ugcRestricted = IsUGCRestrictedMode();
    if (onlineGame)
    {
        if (ugcRestricted)
        {
            setVisible(true);
            gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
            m_actionPending = false;
            return;
        }
    }
    else if (!m_launchData.mFlag57)
    {
        StartGame(0);
        return;
    }
    if (allSignedInLive && gApp.IsLocalMultiplayerAvailable() && fromLoad)
    {
        if (ugcRestricted)
        {
            m_actionPending = false;
            gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
            return;
        }
        if (!gApp.IsDrcPreventingSplitscreen())
        {
            m_signInPending = true;
            return;
        }
        int parameters[2];
        parameters[0] = 0x1A65D275;
        parameters[1] = 0x7A458ED2;
        gConsoleUIController.RequestAlertMessage(0x337F2CFC, 0x6F86265B, parameters, 2,
                                                 gProfile->GetPrimaryPad(), WiiUDRCWarningReturned,
                                                 this, NULL, true);
        return;
    }
    if (onlineGame && allSignedInLive && (ugcRestricted || multiplayerBlocked))
    {
        m_actionPending = false;
        setVisible(true);
        if (!ugcRestricted)
        {
            RequestPrivMessageBox();
            return;
        }
        gConsoleUIController.RequestUGCMessageBox(-1, -1, -1, NULL, NULL);
        return;
    }
    StartGame(0);
}

int IUIScene_StartGame::WiiUDRCWarningReturned(void* context, int,
                                                C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result != C4JStorage::EMessageResult_5 && result == C4JStorage::EMessageResult_4)
    {
        scene->CheckPermissionsAndContinue(false);
    }
    scene->m_actionPending = false;
    return 0;
}

void IUIScene_StartGame::StartSharedLaunchFlow()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    m_actionPending = true;
    if (!m_launchData.mFlag56 && !m_launchData.mFlag57)
    {
        if (!m_launchData.mFlag17)
        {
            unsigned char online = 0;
            if (m_launchData.mFlag0 || m_launchData.mFlag58)
            {
                online = 1;
            }
            GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(0x13), online);
        }
        GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(0x14),
                                      m_launchData.mFlag1);
        GameSettings::SetGameSettings(m_playerId, static_cast<eGameSetting>(0x15),
                                      m_launchData.mFlag2);
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
    }
    else
    {
        gMiniGameOptionCache.updateCachedOptions(m_playerId,
                                                 static_cast<EMiniGameId>(m_launchData.mValue5C),
                                                 m_launchData.mPlayerSettings);
        GameSettings::CheckGameSettingsChanged(true, m_playerId);
    }
    if (m_customGame)
    {
        gApp.SetGameHostOption(static_cast<eGameHostOption>(0x17), m_state629 ? 0 : 1);
    }
    RiderPositionOverrides::ClearOverrides();
    if (m_launchData.mValue1C != 0)
    {
        TexturePack* pack =
            minecraft->mTexturePackRepository->getTexturePackById(m_launchData.mValue1C);
        if (pack != NULL)
        {
            m_texturePackParentPack = static_cast<DLCTexturePack*>(pack)->getDLCInfoParentPack();
            if (m_texturePackParentPack != NULL &&
                !m_texturePackParentPack->hasPurchasedPack())
            {
                int parameters[2];
                parameters[0] = -0x465E40B3;
                parameters[1] = 0x4FCBC7E7;
                if (m_launchData.mFlag56)
                {
                    gConsoleUIController.RequestAlertMessage(0x3BB3096E, -0x0AF6FB3A, parameters, 2,
                                                             m_playerId, TexturePackDialogReturned,
                                                             this, NULL, true);
                    gApp.SetExtraGameData(static_cast<eGameExtraData>(2), 1);
                    return;
                }
                gConsoleUIController.RequestAlertMessage(0x3BB3096E, 0x1CFE436F, parameters, 2,
                                                         m_playerId,
                                                         TrialTexturePackWarningReturned, this,
                                                         NULL, true);
                gApp.SetExtraGameData(static_cast<eGameExtraData>(2), 1);
                return;
            }
        }
    }
    if (!gProfile->IsFullVersion())
    {
        gApp.SetExtraGameData(static_cast<eGameExtraData>(2), 0);
    }
    checkStateAndStartGame();
}

int IUIScene_StartGame::DeleteSaveDataReturned(void* context, bool)
{
    gApp.SetCorruptSaveDeleted(true);
    static_cast<IUIScene_StartGame*>(context)->navigateBack();
    return 0;
}

int IUIScene_StartGame::DeleteSaveDialogReturned(void* context, int,
                                                  C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_5)
    {
        SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
        gStorage->DeleteSaveData(&details->saves[scene->m_saveIndex], DeleteSaveDataReturned,
                                 scene);
        return 0;
    }
    scene->m_actionPending = false;
    return 0;
}

int IUIScene_StartGame::LoadDataComplete(void* context)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (!scene->m_saveCorrupt)
    {
        scene->CheckPermissionsAndContinue(true);
        return 0;
    }
    int parameters[2];
    parameters[0] = 0x4FCBC7E7;
    scene->m_actionPending = false;
    parameters[1] = -0x465E40B3;
    gConsoleUIController.RequestAlertMessage(-0x75879076, -0x637D3399, parameters, 2,
                                             scene->m_playerId, DeleteSaveDialogReturned, scene,
                                             NULL, true);
    return 0;
}

int IUIScene_StartGame::LoadSaveDataReturned(void* context, bool corrupt, bool complete)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    scene->m_saveCorrupt = corrupt;
    if (!complete)
    {
        scene->m_actionPending = false;
        return 0;
    }
    LoadDataComplete(context);
    return 0;
}

int IUIScene_StartGame::CheckCreativeReturned(void* context, int,
                                               C4JStorage::EMessageResult result)
{
    IUIScene_StartGame* scene = static_cast<IUIScene_StartGame*>(context);
    if (result == C4JStorage::EMessageResult_4)
    {
        if (scene->m_levelGenerationOptions == NULL)
        {
            if (!scene->m_customGame || scene->m_launchData.mFlag57)
            {
                scene->CheckPermissionsAndContinue(true);
                return 0;
            }
            SAVE_DETAILS* details = gStorage->ReturnSavesInfo();
            gStorage->LoadSaveData(&details->saves[scene->m_saveIndex], LoadSaveDataReturned,
                                   scene);
            return 0;
        }
        scene->m_saveCorrupt = false;
        LoadDataComplete(scene);
        return 0;
    }
    scene->m_actionPending = false;
    return 0;
}

void IUIScene_StartGame::LaunchGame()
{
    int parameters[2];
    parameters[0] = -0x465E40B3;
    parameters[1] = 0x4FCBC7E7;
    bool creativeConfirmed = false;
    if (!m_isCreative)
    {
        if (m_state62C)
        {
            gConsoleUIController.RequestAlertMessage(-0x20B338A6, 0x7BC65289, parameters, 2,
                                                     m_playerId, CheckCreativeReturned, this, NULL,
                                                     true);
            return;
        }
        creativeConfirmed = false;
        if (m_state62B)
        {
            gConsoleUIController.RequestAlertMessage(-0x20B338A6, -0x0057B1B1, parameters, 2,
                                                     m_playerId, CheckCreativeReturned, this, NULL,
                                                     true);
            return;
        }
    }
    else
    {
        creativeConfirmed = m_state62B;
        if (!creativeConfirmed || m_state62C)
        {
            gConsoleUIController.RequestAlertMessage(-0x20B338A6, -0x6FF856BB, parameters, 2,
                                                     m_playerId, CheckCreativeReturned, this, NULL,
                                                     true);
            return;
        }
    }
    if (m_launchData.mFlag5 && !creativeConfirmed)
    {
        gConsoleUIController.RequestAlertMessage(-0x20B338A6, -0x6C429A80, parameters, 2,
                                                 m_playerId, CheckCreativeReturned, this, NULL,
                                                 true);
        return;
    }
    if (m_launchData.mFlagC)
    {
        gConsoleUIController.RequestAlertMessage(-0x20B338A6, 0x020D77D5, parameters, 2,
                                                 m_playerId, CheckCreativeReturned, this, NULL,
                                                 true);
        return;
    }
    CheckCreativeReturned(this, m_playerId, C4JStorage::EMessageResult_4);
}

int IUIScene_StartGame::getCurrentTextureSelectorIndex(int packId)
{
    if (m_texturePacks.empty())
    {
        return 0;
    }
    int count = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackCount();
    int index;
    for (index = 0; index < count; ++index)
    {
        if (m_texturePacks[index].packId == packId)
        {
            return index;
        }
    }
    return 0;
}
