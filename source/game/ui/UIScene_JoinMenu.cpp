#include "game/ui/UIScene_JoinMenu.hpp"
#include "game/ui/UIScene_QuadrantSignin.hpp"
#include "game/core/StringIds.hpp"
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/UnityClient6.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/IUIScene_LoadCreateJoin.hpp"
#include "game/core/StringFormat.hpp"

UIScene_JoinMenu::UIScene_JoinMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_friendSessionInfo(NULL), m_matchmaking(false),
      m_initialised(false), m_friendSessionUpdated(false), m_error(false)
{
    initialiseMovie();
    m_gameInfo.init(UIString(L""));
    JoinSceneData* value = static_cast<JoinSceneData*>(data);
    if (value != NULL)
    {
        m_friendSessionInfo = value->session;
        m_matchmaking = value->miniGames;
    }
}
UIScene_JoinMenu::~UIScene_JoinMenu()
{
}

unsigned int UIScene_JoinMenu::GetCustomGameModeID()
{
    return m_friendSessionInfo->m_gameSessionData.m_miniGameId;
}
unsigned int UIScene_JoinMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_joinButton.setupControl(this, root, std::string("JoinGame"));
    m_controls.push_back(&m_joinButton);
    m_gamePlayers.setupControl(this, root, std::string("GamePlayers"));
    m_controls.push_back(&m_gamePlayers);
    m_gameInfo.setupControl(this, root, std::string("GameInfo"));
    m_controls.push_back(&m_gameInfo);
    return 1;
}
void UIScene_JoinMenu::friendSessionUpdated(bool updated, void* data)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(data);
    if (scene == NULL)
        return;
    gConsoleUIController.NavigateBack(scene->m_playerId, false, EUIScene_None,
                                      EUILayer_Count);
    if (updated)
        scene->m_friendSessionUpdated = true;
    else
        scene->m_error = true;
}
void UIScene_JoinMenu::AddDescriptionForOption(std::vector<HtmlString>& descriptions, eGameHostOption option, int options)
{
    unsigned int title = 0;
    const wchar_t* value = L"";
    unsigned int state;
    if (option == eGameHostOption_Difficulty)
    {
        state = g_pApp->GetGameHostOption((unsigned int)options, option);
        title = IDS_LABEL_DIFFICULTY;
        value = L10N::GetString(state == 1 ? 0xb558efd9 : state == 2 ? 0x5d131124 :
                                state == 3 ? 0xf70f3270 : 0x4e52b99a);
    }
    else if (option == eGameHostOption_Gamertags)
    {
        state = g_pApp->GetGameHostOption((unsigned int)options, option);
        title = 0x8ca04f68;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else if (option == eGameHostOption_LevelType)
    {
        state = g_pApp->GetGameHostOption((unsigned int)options, option);
        title = IDS_LABEL_LEVEL_TYPE;
        value = L10N::GetString(state ? 0xf37d7beb : 0x1494c206);
    }
    else if (option == eGameHostOption_Structures)
    {
        state = g_pApp->GetGameHostOption((unsigned int)options, option);
        title = IDS_LABEL_STRUCTURES;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else if (option == eGameHostOption_PvP)
    {
        state = g_pApp->GetGameHostOption((unsigned int)options, option);
        title = 0x550744a3;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else if (option == eGameHostOption_TrustPlayers ||
             option == eGameHostOption_TNT ||
             option == eGameHostOption_FireSpreads)
    {
        state = g_pApp->GetGameHostOption((unsigned int)options, option);
        title = option == eGameHostOption_TrustPlayers ? IDS_LABEL_TRUST :
                option == eGameHostOption_TNT ? IDS_LABEL_TNT : IDS_LABEL_FIRE_SPREADS;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else
        return;
    descriptions.push_back(
        HtmlString(formatwstr(L"%ls %ls", L10N::GetString(title), value),
                   eMinecraftColour_NOTSET, false, false, 0));
}
int UIScene_JoinMenu::ErrorDialogReturned(void* data, int, C4JStorage::EMessageResult)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(data);
    if (scene != NULL)
        gConsoleUIController.NavigateBack(scene->m_playerId, false,
                                          EUIScene_None,
                                          EUILayer_Count);
    return 0;
}
void UIScene_JoinMenu::tick()
{
    if (!m_initialised)
    {
        gConsoleUIController.NavigateToScene(m_playerId, EUIScene_Timer, NULL,
                                             EUILayer_Scene,
                                             EUIGroup_PlayerDefault);
        CGameNetworkManager::sInstance.GetFullFriendSessionInfo(
            m_friendSessionInfo, friendSessionUpdated, this);
        m_initialised = true;
    }
    if (m_friendSessionUpdated)
    {
        m_friendSessionUpdated = false;
        m_joinButton.init(UIString(L10N::GetString(IDS_JOIN_GAME)), 0);
        m_gamePlayers.init(1);
        if (m_friendSessionInfo != NULL)
        {
            for (int i = 0; i < 8; ++i)
            {
                const char* gamertag = m_friendSessionInfo->m_gameSessionData.m_playerGamertags[i].m_name;
                if (gamertag[0] == 0)
                    break;
                wchar_t name[32];
                std::mbstowcs(name, gamertag, 31);
                name[31] = 0;
                if (std::strcmp(gamertag, m_friendSessionInfo->m_gameSessionData.m_hostGamertag.m_name) == 0)
                    m_gamePlayers.addItem(formatwstr(L"%ls %ls", L10N::GetString(0x566a7b93), name));
                else
                    m_gamePlayers.addItem(std::wstring(name));
            }

            std::vector<HtmlString> descriptions;
            GameSessionData& sessionData = m_friendSessionInfo->m_gameSessionData;
            if (sessionData.m_gameModeId == static_cast<unsigned int>(GameType::LOBBY->getId()))
            {
                MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
                    static_cast<EMiniGameId>(sessionData.m_miniGameId), true);
                miniGame->GetGameSettingsDescription(
                    descriptions, sessionData.m_customGameModeHostOptions);
            }
            else
            {
                const wchar_t* gameMode = L10N::GetString(IDS_SURVIVAL);
                if (sessionData.m_gameModeId == static_cast<unsigned int>(GameType::CREATIVE->getId()))
                    gameMode = L10N::GetString(IDS_CREATIVE);
                else if (sessionData.m_gameModeId == static_cast<unsigned int>(GameType::ADVENTURE->getId()))
                    gameMode = L10N::GetString(IDS_ADVENTURE);
                descriptions.push_back(
                    HtmlString(formatwstr(L"%ls %ls", L10N::GetString(IDS_LABEL_GAME_TYPE), gameMode),
                               eMinecraftColour_NOTSET, false, false, 0));
                AddDescriptionForOption(descriptions, eGameHostOption_Difficulty,
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, eGameHostOption_Gamertags,
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, eGameHostOption_LevelType,
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, eGameHostOption_PvP,
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, eGameHostOption_TrustPlayers,
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, eGameHostOption_TNT,
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, eGameHostOption_FireSpreads,
                                        sessionData.m_gameHostOptions);
            }
            m_gameInfo.setLabel(UIString(HtmlString::Compose(&descriptions)), false, false);
            m_joining = false;
            g_pApp->SetLiveLinkRequired(true);
            addTimer(0, 30000);
        }
    }
    if (m_error)
    {
        m_joinButton.init(UIString(L10N::GetString(IDS_JOIN_GAME)), 0);
        m_gamePlayers.init(1);
        m_gameInfo.setLabel(UIString(L""), false, false);
        m_error = false;
        int parameter = IDS_CONFIRM_OK;
        gConsoleUIController.RequestErrorMessage(
            IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, &parameter, 1, m_playerId,
            ErrorDialogReturned, this, NULL, true);
    }
    UIScene::tick();
}
void UIScene_JoinMenu::updateTooltips()
{
    int tooltip = getControlFocus() == 1 ? -1 : static_cast<int>(IDS_TOOLTIPS_SELECT);
    gConsoleUIController.SetTooltips(0, tooltip, IDS_TOOLTIPS_BACK, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}
void UIScene_JoinMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, EUIScene_Panorama, true);
    m_layer->showComponent(m_playerId, EUIScene_Logo, true);
}
std::wstring UIScene_JoinMenu::getMoviePath()
{
    return std::wstring(L"JoinMenu");
}
void UIScene_JoinMenu::handleInput(int, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    if (m_joining)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    switch (action)
    {
    case EControllerActions_MenuOk:
        if (getControlFocus() != 1)
            sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        sendInputToMovie(action, repeat, released, pressed);
        handled = true;
        break;
    case EControllerActions_MenuCancel:
        if (released)
        {
            navigateBack();
            handled = true;
        }
        break;
    }
}
void UIScene_JoinMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 1)
        m_gamePlayers.updateChildFocus(childFocus);
    UIScene::handleFocusChange(focus, childFocus);
}
void UIScene_JoinMenu::JoinGame(UIScene_JoinMenu* scene)
{
    bool multiplayerRestricted = false;
    int playerMask = 0;
    bool allPlayersSignedInLive = true;

    gProfile->SetLockedProfile(0);
    if (scene->m_friendSessionInfo != NULL)
        CGameNetworkManager::sInstance.m_joiningMiniGameId = static_cast<EMiniGameId>(scene->GetCustomGameModeID());

    if (!g_pApp->IsLocalMultiplayerAvailable())
    {
        if (gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
        {
            multiplayerRestricted = !gProfile->AllowedToPlayMultiplayer(gProfile->GetPrimaryPad());
            playerMask = CGameNetworkManager::GetLocalPlayerMask(gProfile->GetPrimaryPad());
            allPlayersSignedInLive = gProfile->IsSignedInLive(gProfile->GetPrimaryPad());
        }
    }
    else
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (!gProfile->IsSignedIn(userIndex))
                continue;
            if (!gProfile->AllowedToPlayMultiplayer(userIndex))
                multiplayerRestricted = true;
            playerMask |= CGameNetworkManager::GetLocalPlayerMask(userIndex);
            allPlayersSignedInLive = allPlayersSignedInLive && gProfile->IsSignedInLive(userIndex);
        }
    }

    if (!allPlayersSignedInLive)
    {
        int parameter = IDS_CONFIRM_OK;
        scene->m_joining = false;
        gConsoleUIController.RequestErrorMessage(0x8a21894b, 0xb837024e, &parameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return;
    }
    if (scene->IsUGCRestrictedMode())
    {
        scene->setVisible(true);
        scene->m_joining = false;
        gConsoleUIController.RequestUGCMessageBox(IDS_CONNECTION_FAILED, 0x4d669d55, -1, NULL, NULL);
        return;
    }
    if (multiplayerRestricted)
    {
        int parameter = IDS_CONFIRM_OK;
        scene->setVisible(true);
        scene->m_joining = false;
        gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &parameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        return;
    }

    if (scene->GetCustomGameModeID() != eMiniGameId_Any)
    {
        g_pApp->SetGameHostOption(eGameHostOption_MiniGame, 1);
        g_pApp->SetFirstMiniGameType(static_cast<EMiniGameId>(scene->GetCustomGameModeID()));
    }

    int result = CGameNetworkManager::sInstance.JoinGame(scene->m_friendSessionInfo, playerMask,
                                                         scene->m_matchmaking);
    g_pApp->SetLiveLinkRequired(false);

    int parameter = IDS_CONFIRM_OK;
    switch (result)
    {
    case 0:
        telemetryManagerInstance->PrepareMiniGameHostOptions(scene->m_playerId, NULL,
                                                             eTelemetryJoinMethod_FriendsList);
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        break;
    case 2:
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, 0xb86be603, &parameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        break;
    case 3:
        gConsoleUIController.RequestErrorMessage(IDS_CONNECTION_FAILED, IDS_ERROR_NETWORK, &parameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        break;
    case 4:
        gConsoleUIController.RequestErrorMessage(0x252ab13f, 0xcc5a590f, &parameter, 1,
                                                 gProfile->GetPrimaryPad(), NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        break;
    default:
        gConsoleUIController.NavigateBack(scene->m_playerId, false, EUIScene_None, EUILayer_Count);
        break;
    }
}
int UIScene_JoinMenu::WiiUDRCWarningReturned(void* data, int,
                                             C4JStorage::EMessageResult result)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(data);
    if (result == C4JStorage::EMessageResult_Accept)
        JoinGame(scene);
    if (scene != NULL)
        scene->m_joining = false;
    return 0;
}
int UIScene_JoinMenu::StartGame_SignInReturned(void* context, bool signedIn, int userIndex)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(reinterpret_cast<unsigned int>(context)));
    if (scene != NULL)
    {
        if (signedIn && gProfile->IsSignedIn(userIndex))
            JoinGame(scene);
        else
            scene->m_joining = false;
    }
    return 0;
}
void UIScene_JoinMenu::StartSharedLaunchFlow()
{
    if (!g_pApp->IsLocalMultiplayerAvailable())
    {
        JoinGame(this);
        return;
    }
    if (gApp.IsDrcPreventingSplitscreen())
    {
        int parameters[2] = { 0x1a65d275, 0x7a458ed2 };
        gConsoleUIController.RequestAlertMessage(
            0x337f2cfc, 0x6f86265b, parameters, 2, m_playerId,
            WiiUDRCWarningReturned, this, NULL, true);
        return;
    }
    UIScene_QuadrantSignin::SignInInfo signInData;
    signInData.callback = StartGame_SignInReturned;
    signInData.context = reinterpret_cast<void*>(GetCallbackUniqueId());
    signInData.onlineGame = true;
    signInData.returnToHome = false;
    gConsoleUIController.NavigateToScene(
        gProfile->GetPrimaryPad(), EUIScene_QuadrantSignin, &signInData,
        EUILayer_Scene, EUIGroup_PlayerDefault);
}
void UIScene_JoinMenu::handlePress(int controlId, int)
{
    if (controlId == 0)
    {
        m_joining = true;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
        StartSharedLaunchFlow();
    }
}
int UIScene_JoinMenu::getSceneType()
{
    return EUIScene_LoadMenu;
}
