#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/server/MinecraftServer.hpp"
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
#include "game/ui/UIScene_FullscreenProgress.hpp"
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

struct LoadMenuSceneData
{
    int playerId;
    int saveIndex;
    LevelGenerationOptions* generationOptions;
    _SaveListDetails* saveDetails;
    EMiniGameId gameMode;
};

}

IUIScene_LoadCreateJoin::IUIScene_LoadCreateJoin()
    : m_friendSessions(NULL), m_saveListDetails(NULL), m_saveDetails(NULL), m_joinSceneData(NULL),
      m_ignoreInput(false), m_onlinePlayAllowed(false), m_thumbnailRequested(false), m_saveInfoReady(false),
      m_thumbnailsComplete(false), m_closing(false), m_partyGamesAvailable(false), m_partyGamesOnly(false),
      m_miniGames(false), m_matchmakingButtonCount(0), m_pendingAction(ePendingAction_None), m_nextThumbnail(0),
      m_saveListRefreshState(0), m_buttonCount(0), m_texturePackWorldCount(0), m_selectedSaveIndex(0),
      m_selectedGameIndex(0), m_selectedCreateIndex(0), m_pressedList(0), m_sessionListMode(true),
      m_currentTab(0), m_returnToHomeMenu(false)
{
    InitializeCriticalSection(&m_lock);
}

IUIScene_LoadCreateJoin::~IUIScene_LoadCreateJoin()
{
    if (m_friendSessions != NULL)
    {
        for (std::vector<FriendSessionInfo*>::iterator it = m_friendSessions->begin();
             it < m_friendSessions->end(); ++it)
        {
            if (*it != NULL)
            {
                delete (*it)->m_displayName;
                delete *it;
            }
        }
        delete m_friendSessions;
    }
    delete m_joinSceneData;
    DeleteCriticalSection(&m_lock);
}

uint32_t IUIScene_LoadCreateJoin::GetCustomGameModeID()
{
    return m_miniGames ? eMiniGameId_Battle : eMiniGameId_Any;
}

void IUIScene_LoadCreateJoin::UpdateGamesList()
{
    TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
    if (m_ignoreInput != 0 || repository->getSelected()->isLoadingData() ||
        repository->needsUIUpdate() || gConsoleUIController.IsReloadingSkin() ||
        (m_saveInfoReady == 0 && m_miniGames == 0))
        return;

    bool hadSelection = false;
    unsigned int selectedSessionId = 0;
    if (DoesGamesListHaveFocus() && GetJoinListSize() != 0)
    {
        FriendSessionInfo* selected = m_friendSessions->at(GetJoinListSelection());
        if (selected != NULL)
        {
            selectedSessionId = selected->m_sessionId;
            hadSelection = true;
        }
    }

    ShowJoinListTimer(false);
    if (m_friendSessions != NULL)
    {
        for (std::vector<FriendSessionInfo*>::iterator it = m_friendSessions->begin();
             it < m_friendSessions->end(); ++it)
        {
            if (*it != NULL)
            {
                delete[] (*it)->m_displayName;
                delete *it;
            }
        }
        delete m_friendSessions;
    }

    m_friendSessions = CGameNetworkManager::sInstance.GetSessionList(GetPad(), m_sessionListMode, m_partyGamesOnly != 0);
    GetJoinListSize();
    unsigned int sessionCount = static_cast<unsigned int>(m_friendSessions->size());
    bool gamesListHadFocus = DoesGamesListHaveFocus();
    if (sessionCount == 0)
    {
        SetJoinListEnabled(false);
        ShowNoGamesLabel(true);
        ShowJoinListTimer(false);
        if (gamesListHadFocus)
            InitFocusJoinList(GetPad());
        ClearJoinList();
    }
    else
    {
        if (!IsJoinListEnabled())
        {
            SetJoinListEnabled(true);
            SetGamesListSelection(0);
        }
        ShowNoGamesLabel(false);
        ShowJoinListTimer(false);
        ClearJoinList();
        SetGamesListSelection(0);

        unsigned int index = 0;
        for (std::vector<FriendSessionInfo*>::iterator it = m_friendSessions->begin();
             it < m_friendSessions->end(); ++it, ++index)
        {
            FriendSessionInfo* session = *it;
            const bool lobbySession =
                session->m_gameSessionData.m_gameModeId == static_cast<unsigned int>(GameType::LOBBY->getId());
            if ((m_miniGames != 0) != lobbySession)
                continue;

            wchar_t gameIconName[64] = {};
            std::wstring gameIcon(L"");
            const unsigned int gameModeId = session->m_gameSessionData.m_gameModeId;
            if (gameModeId == static_cast<unsigned int>(GameType::SURVIVAL->getId()))
                gameIcon = L"Graphics\SurvivalIcon.png";
            else if (gameModeId == static_cast<unsigned int>(GameType::CREATIVE->getId()))
                gameIcon = L"Graphics\CreativeIcon.png";
            else if (gameModeId == static_cast<unsigned int>(GameType::ADVENTURE->getId()))
                gameIcon = L"Graphics\AdventureIcon.png";
            else if (lobbySession)
                gameIcon = MiniGameDef::GetCustomGameModeById(
                    static_cast<EMiniGameId>(session->m_gameSessionData.m_miniGameId), true)->getIcon();

            arrayWithLength<unsigned char> gameIconFile;
            gameIconFile.data = NULL;
            gameIconFile.length = 0;
            if (g_pApp->hasArchiveFile(gameIcon, false))
            {
                gameIconFile = g_pApp->getArchiveFile(gameIcon, false);
                if (gameIconFile.data != NULL && gameIconFile.length != 0)
                    std::swprintf(gameIconName, 64, L"gm%ls", session->m_displayName);
            }

            wchar_t packIconName[64] = {};
            unsigned char* packIcon = NULL;
            unsigned int packIconSize = 0;
            const unsigned int packId = session->m_gameSessionData.m_texturePackParentId;
            if (packId != 0)
            {
                TexturePack* pack = repository->getTexturePackById(packId);
                if (pack != NULL)
                {
                    packIcon = static_cast<unsigned char*>(const_cast<void*>(pack->getPackIcon(packIconSize)));
                }
                else
                {
                    unsigned char* tpdData = NULL;
                    unsigned int tpdSize = 0;
                    g_pApp->GetTPD(packId, &tpdData, &tpdSize);
                    g_pApp->GetFileFromTPD(eTPDFileType_Icon, tpdData, tpdSize, &packIcon, &packIconSize);
                }
            }
            if (packId == 0 || packIcon == NULL)
            {
                TexturePack* defaultPack = repository->getTexturePackByIndex(0);
                packIcon = static_cast<unsigned char*>(const_cast<void*>(defaultPack->getPackIcon(packIconSize)));
            }
            if (packIconSize != 0 && packIcon != NULL)
                std::swprintf(packIconName, 64, L"tp%ls", session->m_displayName);

            AddJoinListButton(session->m_displayName, static_cast<int>(index), packIcon, packIconSize,
                              packIconName, gameIconFile.data, gameIconFile.length, gameIconName);
            if (hadSelection && session->m_sessionId == selectedSessionId)
                SetGamesListSelection(index);
        }
    }
    updateTooltips();
}

void IUIScene_LoadCreateJoin::UpdateGamesListCallback(void* context)
{
    if (context != 0)
        static_cast<IUIScene_LoadCreateJoin*>(context)->UpdateGamesList();
}

void IUIScene_LoadCreateJoin::_init(bool miniGames)
{
    m_miniGames = miniGames;
    m_onlinePlayAllowed = gProfile->IsSignedInLive(GetPad()) && gProfile->AllowedToPlayMultiplayer(GetPad());

    CMinecraftApp::PngTagData imageData;
    imageData.flags |= CMinecraftApp::PngTagData::eFlag_HostOptions | CMinecraftApp::PngTagData::eFlag_ExtraData;
    gApp.SetCurrentImageTextData(&imageData);
    if (g_pApp->StartInstallDLCProcess(GetPad()) == 0 && g_pApp->DLCInstallPending() == 0)
        Initialise();
    else
        m_ignoreInput = true;
    UpdateGamesList();
    CGameNetworkManager::sInstance.SetSessionsUpdatedCallback(&IUIScene_LoadCreateJoin::UpdateGamesListCallback, this);
    m_joinSceneData = new JoinSceneData();
    MinecraftServer::s_haltRequested = false;
    MinecraftServer::s_saveOnExitAnswered = false;
    if (!m_ignoreInput)
        gApp.mDLCManager.checkForCorruptDLCAndAlert(true);
}

void IUIScene_LoadCreateJoin::AddMatchmakingButton(std::wstring label, int id, std::wstring icon)
{
    AddCreateListButton(label, id);
    m_buttonIds.push_back(id);
    if (!icon.empty())
    {
        std::wstring path = formatwstr(L"Graphics\\%ls.png", icon.c_str());
        if (g_pApp->hasArchiveFile(path, false))
        {
            arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
            if (file.data != NULL && file.length != 0)
                SetCreateListIcon(m_matchmakingButtonCount, file.data, file.length, icon);
        }
    }
    ++m_matchmakingButtonCount;
}

void IUIScene_LoadCreateJoin::AddGameModeButton(std::wstring label, int id, std::wstring icon)
{
    AddLoadListButton(label, id);
    std::wstring path = formatwstr(L"Graphics\\%ls.png", icon.c_str());
    if (g_pApp->hasArchiveFile(path, false))
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(path, false);
        if (file.data != NULL && file.length != 0)
            SetLoadListIcon(m_buttonCount, file.data, file.length, icon);
    }
    ++m_buttonCount;
}

void IUIScene_LoadCreateJoin::AddDefaultButtons()
{
    if (m_buttonCount >= 1)
        return;

    m_texturePackWorldCount = 0;
    m_levelGenerationOptions.clear();
    m_buttonCount = 0;
    ClearCreateList();
    ClearLoadList();

    if (m_miniGames)
    {
        struct MiniGameButton
        {
            unsigned int label;
            const wchar_t* icon;
            EMiniGameId id;
        };
        static const MiniGameButton buttons[] = {
            { 0x42e0ac30, L"BattleModeIcon", eMiniGameId_Battle },
            { 0xb4e81b6b, L"MG02icon", eMiniGameId_Tumble },
            { 0x8ee5a102, L"MG03icon", eMiniGameId_Glide },
        };
        m_matchmakingButtonCount = 0;
        ShowSaveListTimer(false);
        for (int i = 0; i < 3; ++i)
        {
            std::wstring label = L10N::GetString(buttons[i].label);
            std::wstring icon = buttons[i].icon;
            AddMatchmakingButton(label, buttons[i].id, icon);
            AddGameModeButton(label, buttons[i].id, icon);
        }
        return;
    }

    AddCreateListButton(L10N::GetString(IDS_CREATE_NEW_WORLD), -1);
    ++m_buttonCount;
    std::wstring createWorldIcon = L"Graphics\\CreateWorldIcon.png";
    if (g_pApp->hasArchiveFile(createWorldIcon, false))
    {
        arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(createWorldIcon, false);
        if (file.data != NULL && file.length != 0)
            SetCreateListIcon(0, file.data, file.length, L"CreateWorldIcon");
    }

    std::vector<LevelGenerationOptions*>* generators = g_pApp->getLevelGenerators()->getLevelGenerators();
    for (std::vector<LevelGenerationOptions*>::iterator it = generators->begin(); it != generators->end(); ++it)
    {
        LevelGenerationOptions* options = *it;
        if (options->getRequiredGameRules() != NULL &&
            options->getRequiredGameRules()->getRuleType() != LevelRules::eLevelRuleType_Unknown)
            continue;
        if (!options->isGameModeLobby())
            m_levelGenerationOptions.push_back(options);
    }
    std::sort(m_levelGenerationOptions.begin(), m_levelGenerationOptions.end(),
              LevelGenerators::SortLevelGenerators);

    int id = 0;
    for (std::vector<LevelGenerationOptions*>::iterator it = m_levelGenerationOptions.begin();
         it != m_levelGenerationOptions.end(); ++it, ++id)
    {
        LevelGenerationOptions* options = *it;
        unsigned int texturePackId = options->getTexturePackId();
        AddCreateListButton(std::wstring(options->getWorldName()), id);
        if (texturePackId == 0)
        {
            if (options->isTutorial())
            {
                std::wstring tutorialIcon = L"Graphics\\TutorialIcon.png";
                if (g_pApp->hasArchiveFile(tutorialIcon, false))
                {
                    arrayWithLength<unsigned char> file = g_pApp->getArchiveFile(tutorialIcon, false);
                    if (file.data != NULL && file.length != 0)
                        SetCreateListIcon(m_buttonCount, file.data, file.length, L"TutorialIcon");
                }
            }
            ++m_buttonCount;
            continue;
        }

        ++m_texturePackWorldCount;
        TexturePack* pack = Minecraft::GetInstance()->mTexturePackRepository->getTexturePackById(
            options->getTexturePackId());
        unsigned int iconSize = 0;
        unsigned char* icon = static_cast<unsigned char*>(const_cast<void*>(pack->getPackIcon(iconSize)));
        if (iconSize != 0 && icon != NULL)
        {
            wchar_t name[64];
            std::swprintf(name, 64, L"tpack%08x", pack->getId());
            SetCreateListIcon(m_buttonCount, icon, iconSize, name);
        }
        ++m_buttonCount;
    }
}

int IUIScene_LoadCreateJoin::HandleTelemetrySavesReturned(void*, SAVE_DETAILS* details, bool)
{
    telemetryManagerInstance->RecordSavesInformation(gProfile->GetPrimaryPad(), details, 0);
    return 0;
}

void IUIScene_LoadCreateJoin::GetSaveInfo()
{
    if (m_miniGames)
        return;

    if (g_pApp->DebugSettingsOn() != 0 && g_pApp->GetLoadSavesFromFolderEnabled() != 0)
    {
        File directory(g_pApp->GetDebugRootPath() + L"Saves");
        unsigned int fileCount = 0;
        if (directory.exists())
        {
            m_debugSaveFiles = directory.listFiles();
            fileCount = static_cast<unsigned int>(m_debugSaveFiles->size());
        }
        AddDefaultButtons();
        for (unsigned int i = 0; i < fileCount; ++i)
            AddLoadListButton(m_debugSaveFiles->at(i)->getName(), -1);
        m_thumbnailsComplete = true;
        m_ignoreInput = false;
        m_saveInfoReady = true;
        ScrollLoadListToSelection(0);
        return;
    }

    m_saveInfoReady = false;
    ClearLoadList();
    m_matchmakingButtonCount = 0;
    ShowSaveListTimer(true);
    m_saveDetails = gStorage->ReturnSavesInfo();
    if (m_saveDetails == NULL)
        gStorage->GetSavesInfo(GetPad(), &IUIScene_LoadCreateJoin::HandleTelemetrySavesReturned, NULL, "save");
    else
        HandleTelemetrySavesReturned(NULL, m_saveDetails, false);
}

void IUIScene_LoadCreateJoin::_tick()
{
    switch (m_pendingAction)
    {
    case ePendingAction_None:
        break;
    case ePendingAction_GetSaveInfo:
        m_saveListRefreshState = 0;
        m_nextThumbnail = 0;
        m_ignoreInput = false;
        m_thumbnailsComplete = false;
        m_pendingAction = ePendingAction_None;
        m_thumbnailRequested = false;
        GetSaveInfo();
        break;
    case ePendingAction_RebuildSaveList:
        m_saveInfoReady = false;
        m_nextThumbnail = 0;
        m_thumbnailsComplete = false;
        m_thumbnailRequested = false;
        m_saveListRefreshState = 0;
        m_ignoreInput = false;
        m_matchmakingButtonCount = 0;
        ClearLoadList();
        GetSaveInfo();
        m_pendingAction = ePendingAction_None;
        break;
    case ePendingAction_ReloadSaveInfo:
    case ePendingAction_ReloadSaveInfoUnused:
        m_saveListRefreshState = 0;
        m_ignoreInput = false;
        m_thumbnailsComplete = false;
        m_selectedSaveIndex = 0;
        m_matchmakingButtonCount = 0;
        m_nextThumbnail = 0;
        m_saveInfoReady = false;
        m_thumbnailRequested = false;
        ClearLoadList();
        gStorage->ClearSavesInfo();
        GetSaveInfo();
        m_pendingAction = ePendingAction_None;
        break;
    }
}

void IUIScene_LoadCreateJoin::_updateTooltips()
{
    int partyTooltip = -1;
    int listTooltip = -1;
    if (DoesGamesListHaveFocus() && GetJoinListSize() != 0)
    {
        listTooltip = IDS_TOOLTIPS_VIEW_GAMERCARD;
    }
    else if (!m_miniGames && DoesLoadListHaveFocus() && GetLoadListSize() != 0)
    {
        if (!gStorage->GetSaveDisabled() && gStorage->EnoughSpaceForAMinSaveGame())
            listTooltip = IDS_TOOLTIPS_SAVEOPTIONS;
        else
            listTooltip = IDS_TOOLTIPS_DELETESAVE;
    }
    else if (DoesMashUpWorldHaveFocus())
    {
        listTooltip = IDS_TOOLTIPS_HIDE;
    }
    else
    {
        DoesCreateListHaveFocus();
    }

    if (m_partyGamesAvailable && DoesGamesListHaveFocus())
        partyTooltip = m_partyGamesOnly ? IDS_TOOLTIPS_ALL_GAMES : IDS_TOOLTIPS_PARTY_GAMES;

    if (gProfile->IsFullVersion() && gStorage->GetSaveDisabled())
        DoesLoadListHaveFocus();

    gConsoleUIController.SetTooltips(0, IDS_TOOLTIPS_SELECT, IDS_TOOLTIPS_BACK, partyTooltip, listTooltip, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}

void IUIScene_LoadCreateJoin::handleTimer(int timer)
{
    if (timer != 0)
        return;

    bool onlinePlayAllowed = gProfile->IsSignedInLive(GetPad()) && gProfile->AllowedToPlayMultiplayer(GetPad());
    if (onlinePlayAllowed == m_onlinePlayAllowed)
        return;
    if (!onlinePlayAllowed)
    {
        m_partyGamesAvailable = false;
        ClearJoinList();
        SetJoinListEnabled(false);
        ShowJoinListTimer(true);
        ShowNoGamesLabel(false);
    }
    m_onlinePlayAllowed = onlinePlayAllowed;
}

int IUIScene_LoadCreateJoin::DeleteSaveDialogReturned(void* context, int, C4JStorage::EMessageResult result)
{
    IUIScene_LoadCreateJoin* scene = static_cast<IUIScene_LoadCreateJoin*>(context);
    if (result == C4JStorage::EMessageResult_Decline &&
        (!g_pApp->DebugSettingsOn() || !g_pApp->GetLoadSavesFromFolderEnabled()))
    {
        scene->DeleteSaveFile(scene->m_selectedSaveIndex);
        scene->ShowSaveListTimer(true);
    }
    else
    {
        scene->m_ignoreInput = false;
    }
    return 0;
}

int IUIScene_LoadCreateJoin::SaveOptionsDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    static_cast<IUIScene_LoadCreateJoin*>(context)->handleSaveOptionsDialogReturned(userIndex, result);
    return 0;
}

uint32_t IUIScene_LoadCreateJoin::handleKeyDown(int pad, int action, bool)
{
    if (m_ignoreInput || (!m_saveInfoReady && !m_miniGames))
        return 0;

    switch (action)
    {
    case EControllerActions_MenuB:
    case EControllerActions_MenuCancel:
        gConsoleUIController.NavigateBack(GetPad(), false, EUIScene_None, EUILayer_Count);
        return 1;
    case EControllerActions_MenuY:
        if (!m_miniGames && DoesLoadListHaveFocus() && GetLoadListSize() != 0)
        {
            m_ignoreInput = true;
            if (gStorage->GetSaveDisabled() || !gStorage->EnoughSpaceForAMinSaveGame())
            {
                int options[] = { IDS_CONFIRM_CANCEL, IDS_CONFIRM_OK };
                gConsoleUIController.RequestAlertMessage(IDS_TOOLTIPS_DELETESAVE, IDS_TEXT_DELETE_SAVE, options, 2,
                                                         pad, &IUIScene_LoadCreateJoin::DeleteSaveDialogReturned,
                                                         this, 0, true);
                gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
                return 0;
            }
            int options[4] = { IDS_TITLE_RENAMESAVE, IDS_TOOLTIPS_DELETESAVE, IDS_TOOLTIPS_DELETESAVE };
            unsigned int optionCount = 2;
            if (!gApp.HasNoSpaceForSaveQuota())
            {
                optionCount = 3;
                options[2] = IDS_COPYSAVE;
            }
            if (gProfile->IsSignedInLive(gProfile->GetPrimaryPad()))
                options[optionCount++] = IDS_TOOLTIPS_SAVETRANSFER_TO_NX;
            gConsoleUIController.RequestAlertMessage(IDS_TOOLTIPS_SAVEOPTIONS, IDS_TEXT_SAVEOPTIONS, options,
                                                     optionCount, pad,
                                                     &IUIScene_LoadCreateJoin::SaveOptionsDialogReturned, this, 0,
                                                     true);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
            return 0;
        }
        if (DoesMashUpWorldHaveFocus())
        {
            if (m_selectedSaveIndex != 0)
            {
                LevelGenerationOptions* options = m_levelGenerationOptions.at(m_selectedSaveIndex - 1);
                if (!options->isTutorial() && options->requiresTexturePack())
                {
                    unsigned int texturePackId = options->getTexturePackId();
                    m_ignoreInput = true;
                    GameSettings::HideMashupPackWorld(GetPad(), texturePackId);
                    m_pendingAction = ePendingAction_RebuildSaveList;
                }
            }
            gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        }
        return 0;
    case EControllerActions_MenuRight:
    case EControllerActions_MenuRightScroll:
    {
        ++m_currentTab;
        int tabCount = GetHideLoadTab() ? 2 : 3;
        if (m_currentTab >= tabCount)
            m_currentTab = 0;
        SelectTab(m_currentTab);
        updateTooltips();
        return 0;
    }
    case EControllerActions_MenuLeft:
    case EControllerActions_MenuLeftScroll:
    {
        --m_currentTab;
        int tabCount = GetHideLoadTab() ? 2 : 3;
        if (m_currentTab < 0)
            m_currentTab = tabCount - 1;
        SelectTab(m_currentTab);
        updateTooltips();
        return 0;
    }
    }
    return 0;
}

void IUIScene_LoadCreateJoin::LoadLevelGen(LevelGenerationOptions* options)
{
    gApp.ClearTerrainFeaturePosition();
    gStorage->ResetSaveData();
    gStorage->SetSaveTitle(options->getDefaultSaveName().c_str());

    boost::shared_ptr<Player> player;
    unsigned int hostOption = gApp.GetGameHostOption(eGameHostOption_All);
    if (gApp.GetTutorialMode(player) != 0)
        hostOption = gApp.GetGameHostOption(eGameHostOption_Tutorial);
    unsigned int hostOption6 =
        gApp.GetGameHostOption(hostOption, eGameHostOption_GameType);
    CGameNetworkManager::sInstance.HostGame(0, false, false,
                                             static_cast<unsigned char>(hostOption6),
                                             eMiniGameId_Any, 0, 0);

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_saveData = 0;
    initData->m_hostOptions = hostOption;
    initData->m_miniGameHostOptions = 0;
    initData->m_levelGenerationOptions = options;
    initData->m_texturePackId = options->getTexturePackId();
    initData->m_gameType = static_cast<unsigned char>(hostOption6);
    if (options->requiresTexturePack())
    {
        Minecraft::GetInstance()->mTexturePackRepository->selectTexturePackById(
            static_cast<unsigned int>(initData->m_texturePackId), false);
    }

    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();
    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_threadParam = initData;

    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->showTip = true;
    progressState->closeMode = 3;
    progressState->playerId = 0;
    progressState->backScene = EUIScene_None;
    params->m_completionData = progressState;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress,
                                         params, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}

void IUIScene_LoadCreateJoin::LoadSaveFromDisk(File* file, ESavePlatform platform)
{
    gStorage->ResetSaveData();
    std::wstring saveName = file->getName();
    gStorage->SetSaveTitle(saveName.c_str());

    FileInputStream input(*file);
    unsigned int fileLength = static_cast<unsigned int>(file->length());
    arrayWithLength<unsigned char> fileData;
    fileData.data = new unsigned char[fileLength];
    fileData.length = fileLength;
    input.read(fileData);
    input.close();

    bool hostOnline = gProfile->IsSignedInLive(gProfile->GetPrimaryPad());
    boost::shared_ptr<Player> player;
    if (g_pApp->GetTutorialMode(player) != 0)
        hostOnline = false;

    g_pApp->SetGameHostOption(eGameHostOption_GameType, GameType::SURVIVAL->getId());
    unsigned char gameType =
        static_cast<unsigned char>(g_pApp->GetGameHostOption(eGameHostOption_GameType));
    CGameNetworkManager::sInstance.HostGame(0, hostOnline, false, gameType,
                                            eMiniGameId_Any, 0, 0);

    DiskSaveBuffer* saveBuffer = new DiskSaveBuffer();
    saveBuffer->data = fileData.data;
    saveBuffer->length = fileLength;
    saveBuffer->name = saveName;

    _NetworkGameInitData* initData = new _NetworkGameInitData();
    initData->m_saveData = saveBuffer;
    initData->m_hostOptions = g_pApp->GetGameHostOption(eGameHostOption_All);
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    initData->m_miniGameHostOptions =
        miniGame->GetHostOption(eCustomGameModeHostOption_All);
    initData->m_savePlatform = platform;
    initData->m_gameType = g_pApp->GetGameHostOption(eGameHostOption_GameType);

    CGameNetworkManager::sInstance.FakeLocalPlayerJoined();

    _LoadingInputParams* params = new _LoadingInputParams();
    params->m_threadProc = &CGameNetworkManager::RunNetworkGameThreadProc;
    params->m_threadParam = initData;

    FullscreenProgressState* progressState = new FullscreenProgressState();
    progressState->showConfirmation = false;
    progressState->reservedFlag1 = true;
    progressState->reservedFlag2 = true;
    progressState->showTip = true;
    progressState->closeMode = 3;
    progressState->playerId = 0;
    progressState->backScene = EUIScene_None;
    params->m_completionData = progressState;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_FullscreenProgress,
                                           params, EUILayer_Scene,
                                           EUIGroup_PlayerDefault);
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
    sceneData->generationOptions = lobbyOptions;
    sceneData->saveDetails = NULL;
    sceneData->gameMode = gameMode;

    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_LoadMenu,
                                       sceneData, EUILayer_Scene,
                                       EUIGroup_PlayerDefault);
}

void IUIScene_LoadCreateJoin::HandleLoadListPress(int index)
{
    m_ignoreInput = true;
    if (m_miniGames)
    {
        int gameMode = m_buttonIds[index];
        if (gameMode > eMiniGameId_Build)
            gameMode = eMiniGameId_Build;
        m_selectedMiniGame = static_cast<EMiniGameId>(gameMode);
        LoadGameMode(m_selectedMiniGame);
        return;
    }

    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (g_pApp->DebugSettingsOn() && g_pApp->GetLoadSavesFromFolderEnabled())
    {
        g_pApp->SetTutorialMode(false, boost::shared_ptr<Player>());
        LoadSaveFromDisk(m_debugSaveFiles->at(index), eSavePlatform_WIIU);
        return;
    }

    g_pApp->SetTutorialMode(false, boost::shared_ptr<Player>());
    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->gameMode = eMiniGameId_Any;
    sceneData->playerId = GetPad();
    sceneData->saveIndex = index;
    sceneData->generationOptions = NULL;
    sceneData->saveDetails = &m_saveListDetails[index];
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_LoadMenu, sceneData, EUILayer_Scene,
                                         EUIGroup_PlayerDefault);
}

void IUIScene_LoadCreateJoin::HandleNewListPress(int index, int variant)
{
    m_ignoreInput = 1;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (m_miniGames == 0)
    {
        if (index == 0)
        {
            ShowJoinListTimer(false);
            boost::shared_ptr<Player> player;
            g_pApp->SetTutorialMode(false, player);
            g_pApp->SetCorruptSaveDeleted(false);
            LoadMenuSceneData* sceneData = new LoadMenuSceneData();
            sceneData->playerId = GetPad();
            sceneData->saveIndex = -1;
            sceneData->generationOptions = NULL;
            sceneData->saveDetails = NULL;
            sceneData->gameMode = eMiniGameId_Any;
            gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_CreateWorldMenu,
                                                 sceneData, EUILayer_Scene, EUIGroup_PlayerDefault);
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
            g_pApp->SetGameHostOption(eGameHostOption_Gamertags,
                                      GameSettings::GetGameSettings(gProfile->GetPrimaryPad(),
                                                                    eGameSetting_DisplayMii));
            LoadLevelGen(options);
            return;
        }
        LoadMenuSceneData* sceneData = new LoadMenuSceneData();
        sceneData->playerId = GetPad();
        sceneData->saveIndex = -1;
        sceneData->generationOptions = options;
        sceneData->saveDetails = NULL;
        sceneData->gameMode = eMiniGameId_Any;
        gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_LoadMenu,
                                             sceneData, EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }

    const int primaryPad = gProfile->GetPrimaryPad();
    if (!gProfile->IsSignedInLive(primaryPad))
    {
        int parameter = IDS_CONFIRM_OK;
        gConsoleUIController.RequestAlertMessage(0x456f1349, 0x2cd8b529, &parameter, 1,
                                                 primaryPad, 0, 0, 0, true);
        m_ignoreInput = 0;
        return;
    }

    static bool settingsPromptShown = false;
    int promptCount = GameSettings::GetGameSettings(primaryPad, eGameSetting_PublicMiniGamePrompts);
    if (!settingsPromptShown && promptCount > 0)
    {
        GameSettings::SetGameSettings(primaryPad, eGameSetting_PublicMiniGamePrompts,
                                      static_cast<unsigned char>(promptCount - 1));
        GameSettings::CheckGameSettingsChanged(true, primaryPad);
        int parameter = IDS_CONFIRM_OK;
        gConsoleUIController.RequestAlertMessage(IDS_MG_PUBLIC_GAME_TITLE, IDS_MG_PUBLIC_GAME_TEXT, &parameter, 1,
                                                 primaryPad, 0, 0, 0, true);
        m_ignoreInput = 0;
        settingsPromptShown = true;
        return;
    }

    LoadMenuSceneData* sceneData = new LoadMenuSceneData();
    sceneData->playerId = GetPad();
    sceneData->saveIndex = -1;
    sceneData->generationOptions = NULL;
    sceneData->saveDetails = NULL;
    sceneData->gameMode = static_cast<EMiniGameId>(m_buttonIds[index]);
    gConsoleUIController.NavigateToScene(primaryPad, EUIScene_LoadMenu,
                                         sceneData, EUILayer_Scene, EUIGroup_PlayerDefault);
}

int IUIScene_LoadCreateJoin::TexturePackDialogReturned(void* context, int userIndex, C4JStorage::EMessageResult result)
{
    static_cast<IUIScene_LoadCreateJoin*>(context)->handleTexturePackDialogReturned(userIndex, result);
    return 0;
}

void IUIScene_LoadCreateJoin::CheckAndJoinGame(int index)
{
    if (GetJoinListSize() == 0)
        return;
    if (m_friendSessions->empty() || static_cast<unsigned int>(index) >= m_friendSessions->size())
        return;

    m_joinSceneData->userIndex = GetPad();
    FriendSessionInfo* session = m_friendSessions->at(index);
    m_joinSceneData->miniGames = m_miniGames;
    m_joinSceneData->session = session;

    unsigned int texturePackId = session->m_gameSessionData.m_texturePackParentId;
    if (texturePackId != 0)
    {
        TexturePackRepository* repository = Minecraft::GetInstance()->mTexturePackRepository;
        unsigned int packCount = repository->getTexturePackCount();
        bool texturePackFound = false;
        for (unsigned int packIndex = 0; packIndex < packCount; ++packIndex)
        {
            if (repository->getTexturePackByIndex(packIndex)->getId() ==
                m_joinSceneData->session->m_gameSessionData.m_texturePackParentId)
            {
                texturePackFound = true;
                break;
            }
        }
        if (!texturePackFound)
        {
            int options[2];
            options[0] = IDS_TEXTUREPACK_FULLVERSION;
            options[1] = IDS_CONFIRM_CANCEL;
            gConsoleUIController.RequestAlertMessage(IDS_DLC_TEXTUREPACK_NOT_PRESENT_TITLE,
                                                     IDS_DLC_TEXTUREPACK_NOT_PRESENT, options, 2, GetPad(),
                                                     &IUIScene_LoadCreateJoin::TexturePackDialogReturned, this, 0,
                                                     true);
            return;
        }
    }

    ShowJoinListTimer(false);
    m_ignoreInput = true;
    gConsoleUIController.NavigateToScene(gProfile->GetPrimaryPad(), EUIScene_JoinMenu, m_joinSceneData,
                                         EUILayer_Scene, EUIGroup_PlayerDefault);
}

void IUIScene_LoadCreateJoin::HandleGamesListPressed(int index)
{
    m_ignoreInput = true;
    m_pressedList = 2;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    m_selectedGameIndex = index;
    CheckAndJoinGame(index);
}

void IUIScene_LoadCreateJoin::SetIgnoreInput(bool ignore)
{
    m_ignoreInput = ignore;
}


