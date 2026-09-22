#include "game/core/IUIScene_LoadCreateJoin.hpp"

#include <algorithm>
#include <new>
#include <string>
#include <vector>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/TexturePack.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DLCManager.hpp"
#include "game/core/File.hpp"
#include "game/core/FileInputStream.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringFormat.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/ui/UIGroup.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIScene_LoadCreateJoinMenu.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelRuleset.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/generation/LevelGenerators.hpp"
#include "game/world/generation/Player.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/CStorage.hpp"
#include "vendor/_LoadingInputParams.hpp"
#include "vendor/_SaveListDetails.hpp"
#include "vendor/_NetworkGameInitData.hpp"
#include "vendor/DiskSaveBuffer.hpp"

extern CConsoleMinecraftApp gApp;
extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;
extern CProfile* gProfile;
extern CStorage* gStorage;

namespace
{
struct FullscreenProgressState
{
    bool showConfirmation;
    bool reservedFlag1;
    bool reservedFlag2;
    bool showTip;
    int closeMode;
    int playerId;
    EUIScene backScene;
};

struct LoadMenuSceneData
{
    int playerId;
    int saveIndex;
    void* saveData;
    int reserved;
    EMiniGameId gameMode;
};

std::vector<File*>& loadFileList(IUIScene_LoadCreateJoin* scene)
{
    return scene->GetLoadFileList();
}

int& selectedCustomGameModeId(IUIScene_LoadCreateJoin* scene)
{
    return scene->GetSelectedCustomGameModeId();
}

UIScene_LoadCreateJoinMenu* asMenu(IUIScene_LoadCreateJoin* scene)
{
    return static_cast<UIScene_LoadCreateJoinMenu*>(scene);
}

unsigned int texturePackIdAtSession(FriendSessionInfo* session)
{
    return session->m_gameSessionData.m_texturePackParentId;
}

}

IUIScene_LoadCreateJoin::IUIScene_LoadCreateJoin()
    : m_saveDetails(0), m_joinSceneData(0), m_ignoreInput(0),
      m_state19(0), m_state1a(0), m_state1b(0), m_state1c(0), m_state1d(0), m_state1e(0),
      m_state1f(0), m_customGame(0), m_state21(0), m_state22(0), m_state23(0), m_state24(0),
      m_state28(0), m_state2c(0), m_state30(0), m_state34(0), m_state38(0), m_state3c(0),
      m_state40(0), m_state44(0), m_state5c(0), m_saveListDetails(0), m_saveDetailsCount(0),
      m_state70(1), m_state71(0), m_state72(0),
      m_state73(0), m_state74(0), m_selectedCustomGameModeId(0), m_state79(0), m_state7a(0), m_state7b(0),
      m_state7c(0), m_state7d(0), m_state7e(0), m_state7f(0)
{
    InitializeCriticalSection(&m_lock);
}

IUIScene_LoadCreateJoin::~IUIScene_LoadCreateJoin()
{
    for (std::vector<FriendSessionInfo*>::iterator it = m_friendSessions.begin();
         it != m_friendSessions.end(); ++it)
    {
        if (*it != NULL)
            delete *it;
    }
    m_friendSessions.clear();
    delete m_joinSceneData;
    m_joinSceneData = 0;
    DeleteCriticalSection(&m_lock);
    m_levelGenerationOptions.clear();
    m_buttonIds.clear();
}

uint32_t IUIScene_LoadCreateJoin::GetCustomGameModeID()
{
    return m_customGame ? 1U : 15U;
}

void IUIScene_LoadCreateJoin::UpdateGamesList()
{
    TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
    TexturePack* selectedPack = repository->getSelected();
    if (m_ignoreInput != 0 || selectedPack->isLoadingData() ||
        repository->needsUIUpdate() || gConsoleUIController.IsReloadingSkin() ||
        (m_state1b == 0 && m_customGame == 0))
        return;

    unsigned int selectedSession = 0;
    bool hadSelection = false;
    if (DoesGamesListHaveFocus() && GetJoinListSize() != 0)
    {
        unsigned int selection = GetJoinListSelection();
        if (selection < m_friendSessions.size() && m_friendSessions.at(selection) != 0)
        {
            selectedSession = m_friendSessions.at(selection)->m_sessionId;
            hadSelection = true;
        }
    }

    for (std::vector<FriendSessionInfo*>::iterator it = m_friendSessions.begin();
         it != m_friendSessions.end(); ++it)
    {
        if (*it != 0)
        {
            delete[] (*it)->m_displayName;
            delete *it;
        }
    }
    m_friendSessions.clear();

    std::vector<FriendSessionInfo*>* sessions =
        CGameNetworkManager::sInstance.GetSessionList(GetPad(), m_state1c, m_state1f);
    m_friendSessions.swap(*sessions);
    delete sessions;

    unsigned int sessionCount = static_cast<unsigned int>(m_friendSessions.size());
    if (sessionCount == 0)
    {
        SetJoinListEnabled(false);
        ShowNoGamesLabel(true);
        ShowJoinListTimer(false);
        if (DoesGamesListHaveFocus())
            InitFocusJoinList(GetPad());
        ClearJoinList();
        updateTooltips();
        return;
    }

    if (!IsJoinListEnabled())
    {
        SetJoinListEnabled(true);
        SetGamesListSelection(0);
    }
    ShowNoGamesLabel(false);
    ShowJoinListTimer(false);
    ClearJoinList();
    SetGamesListSelection(0);

    for (unsigned int index = 0; index < sessionCount; ++index)
    {
        FriendSessionInfo* session = m_friendSessions.at(index);
        unsigned int gameModeId = session->m_gameSessionData.m_gameModeId;
        bool customSession = gameModeId == static_cast<unsigned int>(GameType::NOT_SET->getId());
        if ((m_customGame != 0) != customSession)
            continue;
        std::wstring gameIcon;
        if (gameModeId == static_cast<unsigned int>(GameType::SURVIVAL->getId()))
            gameIcon = L"Graphics\\SurvivalIcon.png";
        else if (gameModeId == static_cast<unsigned int>(GameType::CREATIVE->getId()))
            gameIcon = L"Graphics\\CreativeIcon.png";
        else if (gameModeId == static_cast<unsigned int>(GameType::ADVENTURE->getId()))
            gameIcon = L"Graphics\\AdventureIcon.png";
        else
        {
            MiniGameDef* definition = MiniGameDef::GetCustomGameModeById(
                static_cast<EMiniGameId>(session->m_gameSessionData.m_miniGameId), true);
            gameIcon = definition->getIcon();
        }

        unsigned char* gameData = 0;
        unsigned int gameSize = 0;
        if (g_pApp->hasArchiveFile(gameIcon, false))
        {
            arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(gameIcon, false);
            gameData = file.data;
            gameSize = file.length;
        }

        unsigned char* packData = 0;
        unsigned int packSize = 0;
        std::wstring packName;
        unsigned int packId = texturePackIdAtSession(session);
        TexturePack* pack = packId == 0 ? repository->getTexturePackByIndex(0) :
                                         repository->getTexturePackById(packId);
        bool packMissing = pack == 0;
        if (pack != 0)
        {
            const void* icon = pack->getPackIcon(packSize);
            packData = const_cast<unsigned char*>(static_cast<const unsigned char*>(icon));
            if (packData != 0 && packSize != 0)
                packName = formatwstr(L"tpack_%08x", pack->getId());
        }
        if (packMissing && packId != 0)
        {
            unsigned char* tpdData = 0;
            unsigned int tpdSize = 0;
            unsigned char* iconData = 0;
            unsigned int iconSize = 0;
            g_pApp->GetTPD(packId, &tpdData, &tpdSize);
            g_pApp->GetFileFromTPD(static_cast<eTPDFileType>(1), tpdData, tpdSize,
                                   &iconData, &iconSize);
            packData = iconData;
            packSize = iconSize;
            if (packData != 0 && packSize != 0)
                packName = formatwstr(L"tpack_%08x", packId);
        }
        if (packData == 0 || packSize == 0)
        {
            pack = repository->getTexturePackByIndex(0);
            if (pack != 0)
            {
                const void* icon = pack->getPackIcon(packSize);
                packData = const_cast<unsigned char*>(static_cast<const unsigned char*>(icon));
                if (packData != 0 && packSize != 0)
                    packName = formatwstr(L"tpack_%08x", pack->getId());
            }
        }

        AddJoinListButton(session->m_displayName, static_cast<int>(index), gameData, gameSize,
                          gameIcon, packData, packSize, packName);
        if (hadSelection && session->m_sessionId == selectedSession)
            SetGamesListSelection(index);
    }
    updateTooltips();
}

char* IUIScene_LoadCreateJoin::GetSaveDataBase()
{
    return static_cast<char*>(static_cast<void*>(m_joinSceneData));
}

int& IUIScene_LoadCreateJoin::GetSelectedCustomGameModeId()
{
    return m_selectedCustomGameModeId;
}

void IUIScene_LoadCreateJoin::UpdateGamesListCallback(void* context)
{
    if (context != 0)
        static_cast<IUIScene_LoadCreateJoin*>(context)->UpdateGamesList();
}

void IUIScene_LoadCreateJoin::_init(bool customGame)
{
    m_customGame = customGame;
    int pad = GetPad();
    m_state19 = (gProfile->IsSignedInLive(pad) && gProfile->AllowedToPlayMultiplayer(pad)) ? 1 : 0;

    CMinecraftApp::PngTagData imageData;
    imageData.flags |= 0x28000000;
    gApp.SetCurrentImageTextData(&imageData);
    int installResult = g_pApp->StartInstallDLCProcess(pad);
    if (installResult == 0 && g_pApp->DLCInstallPending() == 0)
    {
        Initialise();
    }
    else
    {
        m_ignoreInput = 1;
    }
    UpdateGamesList();
    CGameNetworkManager::sInstance.SetSessionsUpdatedCallback(
        &IUIScene_LoadCreateJoin::UpdateGamesListCallback, this);
    m_joinSceneData = new JoinSceneData();
    if (m_ignoreInput == 0)
        gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

std::vector<File*>& IUIScene_LoadCreateJoin::GetLoadFileList()
{
    return *m_state5c;
}

uint64_t IUIScene_LoadCreateJoin::AddMatchmakingButton(std::wstring label, int id, std::wstring description)
{
    AddLoadListButton(label, id);
    m_buttonIds.push_back(id);
    std::wstring path = formatwstr(L"Graphics\\%ls.png", description.c_str());
    if (g_pApp->hasArchiveFile(path, false))
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
        if (file.data != 0 && file.length != 0)
            SetLoadListIcon(m_state24, file.data, file.length, description);
    }
    ++m_state24;
    return static_cast<uint64_t>(m_buttonIds.size());
}

uint64_t IUIScene_LoadCreateJoin::AddGameModeButton(std::wstring label, int id, std::wstring description)
{
    AddCreateListButton(label, id);
    std::wstring path = formatwstr(L"Graphics\\%ls.png", description.c_str());
    if (g_pApp->hasArchiveFile(path, false))
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
        if (file.data != 0 && file.length != 0)
            SetCreateListIcon(m_state2c, file.data, file.length, description);
    }
    ++m_state2c;
    return 0;
}

void IUIScene_LoadCreateJoin::AddDefaultButtons()
{
    if (m_state34 >= 1)
        return;

    m_state38 = 0;
    m_levelGenerationOptions.clear();
    m_state34 = 0;
    ClearCreateList();
    ClearLoadList();

    if (m_customGame == 0)
    {
        AddCreateListButton(L10N::GetString(0xb0b877e6), -1);
        ++m_state34;

        std::wstring path = L"Graphics\\CreateWorldIcon.png";
        if (g_pApp->hasArchiveFile(path, false))
        {
            arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
            if (file.data != 0 && file.length != 0)
                SetCreateListIcon(0, file.data, file.length, L"CreateWorldIcon");
        }

        std::vector<LevelGenerationOptions*>* generators =
            g_pApp->getLevelGenerators()->getLevelGenerators();
        for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin();
             it != generators->end(); ++it)
        {
            LevelGenerationOptions* options = *it;
            LevelRuleset* rules = options->getRequiredGameRules();
            if ((rules == 0 || rules->getRuleType() == 0) && !options->isGameModeLobby())
                m_levelGenerationOptions.push_back(options);
        }
        std::sort(m_levelGenerationOptions.begin(), m_levelGenerationOptions.end(),
                  LevelGenerators::SortLevelGenerators);

        int id = 0;
        for (std::vector<LevelGenerationOptions*>::iterator it = m_levelGenerationOptions.begin();
             it != m_levelGenerationOptions.end(); ++it)
        {
            LevelGenerationOptions* options = *it;
            AddCreateListButton(std::wstring(options->getWorldName()), id++);
            unsigned int texturePackId = options->getTexturePackId();
            if (texturePackId == 0)
            {
                if (options->isTutorial())
                {
                    path = L"Graphics\\TutorialIcon.png";
                    if (g_pApp->hasArchiveFile(path, false))
                    {
                        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
                        if (file.data != 0 && file.length != 0)
                            SetCreateListIcon(m_state34, file.data, file.length, L"TutorialIcon");
                    }
                }
                ++m_state34;
            }
            else
            {
                ++m_state38;
                TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(texturePackId);
                unsigned int size = 0;
                const void* icon = pack->getPackIcon(size);
                unsigned int packId = pack->getId();
                if (icon != 0 && size != 0)
                {
                    std::wstring name = formatwstr(L"tpack_%08x", packId);
                    SetCreateListIcon(m_state34, const_cast<unsigned char*>(static_cast<const unsigned char*>(icon)),
                                      size, name);
                }
                ++m_state34;
            }
        }
    }
    else
    {
        ShowSaveListTimer(false);
        m_state24 = 0;
        static const unsigned int labels[] = { 0x42e0ac30U, 0xb4e81b6bU, 0x8ee5a102U };
        static const wchar_t* icons[] = { L"BattleModeIcon", L"MG02icon", L"MG03icon" };
        for (int i = 0; i < 3; ++i)
        {
            std::wstring label = L10N::GetString(labels[i]);
            AddMatchmakingButton(label, i + 1, std::wstring(icons[i]));
            AddGameModeButton(label, i + 1, std::wstring(icons[i]));
        }
    }
}

int IUIScene_LoadCreateJoin::HandleTelemetrySavesReturned(void* context, SAVE_DETAILS* details, bool complete)
{
    (void)complete;
    telemetryManagerInstance->RecordSavesInformation(gProfile->GetPrimaryPad(), details, 0);
    (void)context;
    return 0;
}

void IUIScene_LoadCreateJoin::GetSaveInfo()
{
    if (m_customGame == 0)
    {
        if (g_pApp->DebugSettingsOn() != 0 && g_pApp->GetLoadSavesFromFolderEnabled() != 0)
        {
            std::wstring path = g_pApp->GetDebugRootPath() + L"Saves";
            File directory(path);
            AddDefaultButtons();
            if (directory.exists())
            {
                m_state5c = directory.listFiles();
                for (std::vector<File*>::iterator it = m_state5c->begin(); it != m_state5c->end(); ++it)
                    AddLoadListButton((*it)->getName(), -1);
            }
            m_state1c = 1;
            m_ignoreInput = 0;
            m_state1b = 1;
            ScrollLoadListToSelection(0);
            return;
        }
        m_state1b = 0;
        m_state24 = 0;
        ClearLoadList();
        ShowSaveListTimer(true);
        m_saveDetails = gStorage->ReturnSavesInfo();
        if (m_saveDetails == 0)
            gStorage->GetSavesInfo(GetPad(), &IUIScene_LoadCreateJoin::HandleTelemetrySavesReturned, 0, "save");
        else
            HandleTelemetrySavesReturned(this, m_saveDetails, false);
    }
}

void IUIScene_LoadCreateJoin::_tick()
{
    if (m_state28 == 1)
    {
        m_state30 = 0;
        m_state2c = 0;
        m_ignoreInput = 0;
        m_state1a = 0;
        m_state1c = 0;
        m_state28 = 0;
        GetSaveInfo();
    }
    else if (m_state28 == 2)
    {
        m_state1b = 0;
        m_state2c = 0;
        m_state1a = 0;
        m_state30 = 0;
        m_ignoreInput = 0;
        m_state1c = 0;
        m_state24 = 0;
        ClearLoadList();
        GetSaveInfo();
        m_state28 = 0;
    }
    else if (m_state28 != 0 && m_state28 <= 4)
    {
        m_state30 = 0;
        m_state1a = 0;
        m_state24 = 0;
        m_state2c = 0;
        m_state1b = 0;
        m_ignoreInput = 0;
        m_state1c = 0;
        m_state3c = 0;
        ClearLoadList();
        gStorage->ClearSavesInfo();
        GetSaveInfo();
        m_state28 = 0;
    }
}

void IUIScene_LoadCreateJoin::_updateTooltips()
{
    UIScene_LoadCreateJoinMenu* menu = asMenu(this);
    int tooltip3 = -1;
    int tooltip4 = -1;
    if (menu->getSubSceneType() == 0 || menu->GetPad() == 0)
    {
        if (m_customGame == 0 && menu->DoesLoadListHaveFocus() && menu->DoesCreateListHaveFocus())
        {
            if (gStorage->GetSaveDisabled() == 0 && gStorage->EnoughSpaceForAMinSaveGame() != 0)
                tooltip4 = 0x0aa027cc;
            else
                tooltip4 = 0x0e3265e5;
        }
        else if (menu->GetHideLoadTab())
            tooltip4 = 0x0f131e015;
    }
    else
        tooltip4 = 0x0aec93217;

    if (m_state1e != 0 && menu->getSubSceneType() != 0)
        tooltip3 = m_state1f != 0 ? 0x060f9906e : 0x051758df1;

    if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()) && gStorage->GetSaveDisabled() != 0)
        menu->DoesLoadListHaveFocus();

    gConsoleUIController.SetTooltips(0, 0x8d6323ed, 0x4f6bcf5e, tooltip3, tooltip4, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void IUIScene_LoadCreateJoin::handleTimer(int timer)
{
    if (timer != 0)
        return;

    UIScene_LoadCreateJoinMenu* menu = asMenu(this);
    unsigned char signedLive = 0;
    int pad = menu->GetPad();
    if (gProfile->IsSignedIn(pad) && gProfile->IsSignedInLive(pad))
        signedLive = 1;
    if (signedLive == m_state19)
        return;
    if (signedLive == 0)
    {
        m_state1e = 0;
        menu->handleReload();
        menu->ShowSaveListTimer(false);
        menu->ShowJoinListTimer(true);
        menu->ShowNoGamesLabel(false);
    }
    m_state19 = signedLive;
}

uint32_t IUIScene_LoadCreateJoin::DeleteSaveDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    IUIScene_LoadCreateJoin* scene = static_cast<IUIScene_LoadCreateJoin*>(context);
    if (result == C4JStorage::EMessageResult_5 &&
        (g_pApp->DebugSettingsOn() == 0 || g_pApp->GetLoadSavesFromFolderEnabled() == 0))
    {
        UIScene_LoadCreateJoinMenu* menu = asMenu(scene);
        menu->DeleteSaveFile(scene->m_state3c);
        menu->ShowSaveListTimer(true);
    }
    else
    {
        scene->m_state1e = 0;
    }
    return 0;
}

int IUIScene_LoadCreateJoin::SaveOptionsDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    asMenu(static_cast<IUIScene_LoadCreateJoin*>(context))
        ->handleSaveOptionsDialogReturned(userIndex, result);
    return 0;
}

uint32_t IUIScene_LoadCreateJoin::handleKeyDown(int key, int action, bool repeat)
{
    if (m_ignoreInput != 0 || (m_state1b == 0 && m_customGame == 0))
        return 0;
    if (action == 1 || action == 21)
    {
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None,
                                          static_cast<EUILayer>(8));
        return 1;
    }
    if (action == 3)
    {
        if (!m_customGame && DoesLoadListHaveFocus() && GetLoadListSize() != 0)
        {
            m_ignoreInput = 1;
            int parameters[5] = { static_cast<int>(0xf96217da), 0x0e3265e5, 0x0e3265e5, 0, 0 };
            unsigned int parameterCount = 2;
            if (gApp.HasNoSpaceForSaveQuota() == 0)
            {
                parameters[2] = 0x490209b8;
                parameterCount = 3;
            }
            if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
                parameters[parameterCount++] = 0x7e7c9aba;
            gConsoleUIController.RequestAlertMessage(
                0x0aa027cc, 0x5524ecc7, parameters, parameterCount,
                static_cast<unsigned int>(key), &IUIScene_LoadCreateJoin::SaveOptionsDialogReturned,
                this, 0, true);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
        }
        return 0;
    }
    if (action == 6 || action == 10)
    {
        ++m_state74;
        int tabCount = GetHideLoadTab() ? 2 : 3;
        if (m_state74 >= tabCount)
            m_state74 = 0;
        SelectTab(m_state74);
        asMenu(this)->updateTooltips();
        return 0;
    }
    if (action == 7 || action == 11)
    {
        --m_state74;
        int tabCount = GetHideLoadTab() ? 2 : 3;
        if (m_state74 < 0)
            m_state74 = tabCount - 1;
        SelectTab(m_state74);
        asMenu(this)->updateTooltips();
    }
    return 0;
}

void IUIScene_LoadCreateJoin::LoadLevelGen(LevelGenerationOptions* options)
{
    gApp.ClearTerrainFeaturePosition();
    gStorage->ResetSaveData();
    gStorage->SetSaveTitle(options->getDefaultSaveName().c_str());

    boost::shared_ptr<Player> player;
    unsigned int hostOption = gApp.GetGameHostOption(static_cast<eGameHostOption>(0x20));
    if (gApp.GetTutorialMode(player) != 0)
        hostOption = gApp.GetGameHostOption(static_cast<eGameHostOption>(5));
    unsigned int hostOption6 =
        gApp.GetGameHostOption(hostOption, static_cast<eGameHostOption>(6));
    CGameNetworkManager::sInstance.HostGame(0, false, false,
                                             static_cast<unsigned char>(hostOption6),
                                             eMiniGameId_Any, 0, 0);

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_value2 = 0;
    initData->m_value3 = hostOption;
    initData->m_value4 = 0;
    initData->m_value5 = options;
    initData->m_value6 = options->getTexturePackId();
    initData->m_value9 = static_cast<unsigned char>(hostOption6);
    if (options->requiresTexturePack())
    {
        Minecraft::GetInstance()->mTexturePackRepository->selectTexturePackById(
            static_cast<unsigned int>(initData->m_value6), false);
    }

    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_firstParameter = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_secondParameter = initData;

    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->showTip = true;
    progressState->closeMode = 3;
    progressState->playerId = 0;
    progressState->backScene = EUIScene_None;
    params->m_thirdParameter = progressState;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress,
                                         params, static_cast<EUILayer>(6),
                                         static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::LoadSaveFromDisk(File* file, ESavePlatform platform)
{
    gStorage->Tick();
    std::wstring saveName = file->getName();
    gStorage->SetSaveTitle(saveName.c_str());

    FileInputStream input(*file);
    unsigned int fileLength = static_cast<unsigned int>(file->length());
    arrayWithLength<unsigned char> fileData;
    fileData.data = new unsigned char[fileLength];
    fileData.length = fileLength;
    input.read(fileData);
    input.close();

    boost::shared_ptr<Player> player;
    bool hostOnline = false;
    if (gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
        hostOnline = true;
    g_pApp->SetTutorialMode(false, player);
    if (g_pApp->GetTutorialMode(player) != 0)
        hostOnline = false;

    g_pApp->SetGameHostOption(static_cast<eGameHostOption>(6), GameType::SURVIVAL->getId());
    unsigned char gameType =
        static_cast<unsigned char>(g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6)));
    CGameNetworkManager::sInstance.HostGame(0, hostOnline, false, gameType,
                                            eMiniGameId_Any, 0, 0);

    DiskSaveBuffer* saveBuffer = new DiskSaveBuffer();
    saveBuffer->data = fileData.data;
    saveBuffer->length = fileLength;
    saveBuffer->name = saveName;

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_value2 = saveBuffer;
    initData->m_value3 = g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0x20));
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    initData->m_value4 =
        miniGame->GetHostOption(static_cast<eCustomGameModeHostOption>(10));
    initData->m_value8 = static_cast<int>(platform);
    initData->m_value9 = g_pApp->GetGameHostOption(static_cast<eGameHostOption>(6));

    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();

    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_firstParameter = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_secondParameter = initData;

    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->showTip = true;
    progressState->closeMode = 3;
    progressState->playerId = 0;
    progressState->backScene = EUIScene_None;
    params->m_thirdParameter = progressState;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress,
                                           params, static_cast<EUILayer>(6),
                                           static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::LoadGameMode(EMiniGameId gameMode)
{
    LevelGenerationOptions* lobbyOptions = 0;
    std::vector<LevelGenerationOptions*>* generators = g_pApp->getLevelGenerators()->getLevelGenerators();
    if (generators != 0)
    {
        for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin();
             it != generators->end(); ++it)
        {
            if ((*it) != 0 && (*it)->isGameModeLobby())
                lobbyOptions = *it;
        }
    }

    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->playerId = GetPad();
    sceneData->saveIndex = -1;
    sceneData->saveData = lobbyOptions;
    sceneData->reserved = 0;
    sceneData->gameMode = gameMode;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_LoadMenu,
                                       sceneData, static_cast<EUILayer>(6),
                                       static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::HandleLoadListPress(int index)
{
    m_state1a = 1;
    if (m_customGame != 0)
    {
        int* gameModeIds = m_buttonIds.empty() ? NULL : &m_buttonIds[0];
        int gameMode = gameModeIds[index];
        if (gameMode > 4)
            gameMode = 4;
        selectedCustomGameModeId(this) = gameMode;
        LoadGameMode(static_cast<EMiniGameId>(gameMode));
        return;
    }

    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    if (g_pApp->DebugSettingsOn() != 0 && g_pApp->GetLoadSavesFromFolderEnabled() != 0)
    {
        boost::shared_ptr<Player> player;
        g_pApp->SetTutorialMode(false, player);
        File* loadFile = loadFileList(this).at(index);
        LoadSaveFromDisk(loadFile, static_cast<ESavePlatform>(0x55494957));
        return;
    }

    boost::shared_ptr<Player> player;
    g_pApp->SetTutorialMode(false, player);

    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->playerId = GetPad();
    sceneData->saveIndex = index;
        sceneData->saveData = GetSaveDataBase() + index * 300;
    sceneData->reserved = 0;
    sceneData->gameMode = eMiniGameId_Any;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_LoadMenu,
                                       sceneData, static_cast<EUILayer>(6),
                                       static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::HandleNewListPress(int index, int variant)
{
    m_ignoreInput = 1;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
    ShowJoinListTimer(false);
    if (m_customGame != 0)
        return;
    if (index == 0)
    {
        boost::shared_ptr<Player> player;
        g_pApp->SetTutorialMode(false, player);
        g_pApp->SetCorruptSaveDeleted(false);
        LoadMenuSceneData* sceneData = new LoadMenuSceneData();
        sceneData->playerId = GetPad();
        sceneData->saveIndex = -1;
        sceneData->saveData = 0;
        sceneData->reserved = 0;
        sceneData->gameMode = eMiniGameId_Any;
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x1a),
                                             sceneData, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (variant < 0 || static_cast<unsigned int>(variant) >= m_levelGenerationOptions.size())
        return;
    LevelGenerationOptions* options = m_levelGenerationOptions.at(static_cast<unsigned int>(variant));
    g_pApp->SetBaseSaveName(options->getBaseSavePath());
    boost::shared_ptr<Player> player;
    g_pApp->SetTutorialMode(options->isTutorial(), player);
    g_pApp->SetAutosaveTimerTime();
    if (options->isTutorial())
    {
        g_pApp->SetGameHostOption(static_cast<eGameHostOption>(4),
                                  GameSettings::GetGameSettings(gProfile->GetPrimaryPad(),
                                                                static_cast<eGameSetting>(0xc)));
        LoadLevelGen(options);
        return;
    }
    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->playerId = GetPad();
    sceneData->saveIndex = -1;
    sceneData->saveData = options;
    sceneData->reserved = 0;
    sceneData->gameMode = eMiniGameId_Any;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_LoadMenu,
                                         sceneData, static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    return;
}

int IUIScene_LoadCreateJoin::TexturePackDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    asMenu(static_cast<IUIScene_LoadCreateJoin*>(context))
        ->handleTexturePackDialogReturned(userIndex, result);
    return 0;
}

void IUIScene_LoadCreateJoin::CheckAndJoinGame(int index)
{
    UIScene_LoadCreateJoinMenu* menu = asMenu(this);
    if (GetPad() == 0)
        return;
    if (m_friendSessions.empty())
        return;
    if (index < 0 || static_cast<unsigned int>(index) >= m_friendSessions.size())
        return;

    JoinSceneData* joinData = m_joinSceneData;

    joinData->userIndex = GetPad();
    FriendSessionInfo* session = m_friendSessions.at(index);
    joinData->customGame = m_customGame;
    joinData->session = session;

    if (texturePackIdAtSession(session) != 0)
    {
        TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
        unsigned int packCount = repository->getTexturePackCount();
        bool texturePackFound = false;
        for (unsigned int packIndex = 0; packIndex < packCount; ++packIndex)
        {
            TexturePack* pack = repository->getTexturePackByIndex(packIndex);
            if (pack != 0 && pack->getId() == texturePackIdAtSession(session))
            {
                texturePackFound = true;
                break;
            }
        }
        if (!texturePackFound)
        {
            int parameters[2];
            parameters[0] = 0xeac05ed4;
            parameters[1] = 0x4fcbc7e7;
            gConsoleUIController.RequestAlertMessage(
                0x1e8ee39f, 0x5ab46118, parameters, 2, GetPad(),
                TexturePackDialogReturned, this, 0, true);
            return;
        }
    }
    menu->InitFocusJoinList(0);
    m_ignoreInput = 1;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x1c),
                                       joinData, static_cast<EUILayer>(6),
                                       static_cast<EUIGroup>(7));
}

void IUIScene_LoadCreateJoin::HandleGamesListPressed(int index)
{
    m_ignoreInput = 1;
    m_state2c = 2;
    CheckAndJoinGame(index);
}

void IUIScene_LoadCreateJoin::SetIgnoreInput(bool ignore)
{
    m_ignoreInput = ignore;
}


