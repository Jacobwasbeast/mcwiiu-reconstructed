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
    : UIScene(sceneType, layer)
{
    initialiseMovie();
    m_gameInfo.init(UIString(L""));
    m_initialised = false;
    m_friendSessionUpdated = false;
    m_error = false;
    JoinSceneData* value = static_cast<JoinSceneData*>(data);
    m_friendSessionInfo = value->session;
    m_matchmaking = value->miniGames;
}

unsigned int UIScene_JoinMenu::GetCustomGameModeID()
{
    return m_friendSessionInfo->m_gameSessionData.m_miniGameId;
}
unsigned int UIScene_JoinMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    {
        std::string name("JoinGame");
        m_joinButton.setupControl(this, root, name);
    }
    m_controls.push_back(&m_joinButton);
    {
        std::string name("GamePlayers");
        m_gamePlayers.setupControl(this, root, name);
    }
    m_controls.push_back(&m_gamePlayers);
    {
        std::string name("GameInfo");
        m_gameInfo.setupControl(this, root, name);
    }
    m_controls.push_back(&m_gameInfo);
    return 1;
}
void UIScene_JoinMenu::friendSessionUpdated(bool updated, void* data)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(data);
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
        state = gApp.GetGameHostOption((unsigned int)options, option);
        title = IDS_LABEL_DIFFICULTY;
        value = L10N::GetString(state == 1 ? IDS_DIFFICULTY_TITLE_EASY : state == 2 ? IDS_DIFFICULTY_TITLE_NORMAL :
                                state == 3 ? IDS_DIFFICULTY_TITLE_HARD : IDS_DIFFICULTY_TITLE_PEACEFUL);
    }
    else if (option == eGameHostOption_Gamertags)
    {
        state = gApp.GetGameHostOption((unsigned int)options, option);
        title = 0x8ca04f68;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else if (option == eGameHostOption_LevelType)
    {
        state = gApp.GetGameHostOption((unsigned int)options, option);
        title = IDS_LABEL_LEVEL_TYPE;
        value = L10N::GetString(state ? 0xf37d7beb : 0x1494c206);
    }
    else if (option == eGameHostOption_Structures)
    {
        state = gApp.GetGameHostOption((unsigned int)options, option);
        title = IDS_LABEL_STRUCTURES;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else if (option == eGameHostOption_PvP)
    {
        state = gApp.GetGameHostOption((unsigned int)options, option);
        title = 0x550744a3;
        value = L10N::GetString(state ? IDS_ON : IDS_OFF);
    }
    else if (option == eGameHostOption_TrustPlayers ||
             option == eGameHostOption_TNT ||
             option == eGameHostOption_FireSpreads)
    {
        state = gApp.GetGameHostOption((unsigned int)options, option);
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
        for (int i = 0; i < GameSessionData::MAX_SESSION_PLAYERS; ++i)
        {
            GameSessionData::Gamertag emptyGamertag;
            bool slotEmpty = strcmp(m_friendSessionInfo->m_gameSessionData.m_playerGamertags[i].m_name,
                                    emptyGamertag.m_name) == 0;
            if (slotEmpty)
                break;
            wchar_t displayName[32];
            if (strcmp(m_friendSessionInfo->m_gameSessionData.m_playerGamertags[i].m_name,
                       m_friendSessionInfo->m_gameSessionData.m_hostGamertag.m_name) == 0)
            {
                char gamertag[GameSessionData::MAX_GAMERTAG_LENGTH + 2];
                memcpy(gamertag, m_friendSessionInfo->m_gameSessionData.m_playerGamertags[i].m_name,
                       GameSessionData::MAX_GAMERTAG_LENGTH);
                gamertag[sizeof(gamertag) - 1] = 0;
                wchar_t hostName[32];
                mbstowcs(hostName, gamertag, 32);
                swprintf(displayName, 32, L"%ls %ls", L10N::GetString(0x566a7b93), hostName);
            }
            else
            {
                char gamertag[GameSessionData::MAX_GAMERTAG_LENGTH + 2];
                memcpy(gamertag, m_friendSessionInfo->m_gameSessionData.m_playerGamertags[i].m_name,
                       GameSessionData::MAX_GAMERTAG_LENGTH);
                gamertag[sizeof(gamertag) - 1] = 0;
                mbstowcs(displayName, gamertag, 32);
            }
            m_gamePlayers.addItem(std::wstring(displayName));
        }

        if (m_friendSessionInfo->m_gameSessionData.m_gameModeId == static_cast<unsigned int>(GameType::LOBBY->getId()))
        {
            std::vector<HtmlString> descriptions;
            MiniGameDef* miniGame = MiniGameDef::GetCustomGameModeById(
                static_cast<EMiniGameId>(m_friendSessionInfo->m_gameSessionData.m_miniGameId), true);
            miniGame->GetGameSettingsDescription(descriptions, m_friendSessionInfo->m_gameSessionData.m_customGameModeHostOptions);
            m_gameInfo.setLabel(UIString(HtmlString::Compose(&descriptions)), false, false);
        }
        else
        {
            unsigned int hostOptions = m_friendSessionInfo->m_gameSessionData.m_gameHostOptions;
            std::vector<HtmlString> descriptions;
            std::wstring gameModeName = L"";
            if (m_friendSessionInfo->m_gameSessionData.m_gameModeId == static_cast<unsigned int>(GameType::CREATIVE->getId()))
                gameModeName = L10N::GetString(IDS_CREATIVE);
            else if (m_friendSessionInfo->m_gameSessionData.m_gameModeId == static_cast<unsigned int>(GameType::ADVENTURE->getId()))
                gameModeName = L10N::GetString(IDS_ADVENTURE);
            else
                gameModeName = L10N::GetString(IDS_SURVIVAL);
            descriptions.push_back(
                HtmlString(formatwstr(L"%ls %ls", L10N::GetString(IDS_LABEL_GAME_TYPE), gameModeName.c_str()),
                           eMinecraftColour_NOTSET, false, false, 0));
            AddDescriptionForOption(descriptions, eGameHostOption_Difficulty, hostOptions);
            AddDescriptionForOption(descriptions, eGameHostOption_Gamertags, hostOptions);
            AddDescriptionForOption(descriptions, eGameHostOption_LevelType, hostOptions);
            AddDescriptionForOption(descriptions, eGameHostOption_PvP, hostOptions);
            AddDescriptionForOption(descriptions, eGameHostOption_TrustPlayers, hostOptions);
            AddDescriptionForOption(descriptions, eGameHostOption_TNT, hostOptions);
            AddDescriptionForOption(descriptions, eGameHostOption_FireSpreads, hostOptions);
            m_gameInfo.setLabel(UIString(HtmlString::Compose(&descriptions)), false, false);
        }
        m_joining = false;
        gApp.SetLiveLinkRequired(true);
        addTimer(0, 30000);
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
void UIScene_JoinMenu::handleInput(int, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    if (m_joining)
        return;
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuCancel:
        if (pressed)
        {
            navigateBack();
            handled = true;
        }
        break;
    case EControllerActions_MenuOk:
        if (getControlFocus() == 1)
        {
            handled = true;
            break;
        }
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
        sendInputToMovie(action, repeat, pressed, released);
        handled = true;
        break;
    }
}
void UIScene_JoinMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 1)
        m_gamePlayers.updateChildFocus(childFocus);
    updateTooltips();
}
void UIScene_JoinMenu::JoinGame(UIScene_JoinMenu* scene)
{
    bool multiplayerRestricted = false;
    int playerMask = 0;
    bool allPlayersSignedInLive = true;

    gProfile->SetLockedProfile(0);
    if (scene->m_friendSessionInfo != NULL)
        CGameNetworkManager::sInstance.m_joiningMiniGameId = static_cast<EMiniGameId>(scene->m_friendSessionInfo->m_gameSessionData.m_miniGameId);

    if (!gApp.IsLocalMultiplayerAvailable())
    {
        if (gProfile->IsSignedIn(gProfile->GetPrimaryPad()))
        {
            if (!gProfile->AllowedToPlayMultiplayer(gProfile->GetPrimaryPad()))
                multiplayerRestricted = true;
            playerMask = CGameNetworkManager::GetLocalPlayerMask(gProfile->GetPrimaryPad());
            allPlayersSignedInLive = gProfile->IsSignedInLive(gProfile->GetPrimaryPad());
        }
    }
    else
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (gProfile->IsSignedIn(userIndex))
            {
                if (allPlayersSignedInLive)
                    gProfile->IsSignedInLive(userIndex);
                if (!gProfile->AllowedToPlayMultiplayer(userIndex))
                    multiplayerRestricted = true;
                playerMask |= CGameNetworkManager::GetLocalPlayerMask(userIndex);
                allPlayersSignedInLive = allPlayersSignedInLive && gProfile->IsSignedInLive(userIndex);
            }
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

    if (scene->m_friendSessionInfo->m_gameSessionData.m_miniGameId != eMiniGameId_Any)
    {
        gApp.SetGameHostOption(eGameHostOption_MiniGame, 1);
        gApp.SetFirstMiniGameType(static_cast<EMiniGameId>(scene->m_friendSessionInfo->m_gameSessionData.m_miniGameId));
    }

    int result = CGameNetworkManager::sInstance.JoinGame(scene->m_friendSessionInfo, playerMask,
                                                         scene->m_matchmaking);
    gApp.SetLiveLinkRequired(false);

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
    if (result != C4JStorage::EMessageResult_Decline)
    {
        if (result == C4JStorage::EMessageResult_Accept)
            JoinGame(scene);
    }
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
    if (!gApp.IsLocalMultiplayerAvailable())
    {
        JoinGame(this);
        return;
    }
    if (gApp.IsDrcPreventingSplitscreen())
    {
        int parameters[2] = { 0x1a65d275, 0x7a458ed2 };
        gConsoleUIController.RequestAlertMessage(
            0x337f2cfc, 0x6f86265b, parameters, 2, gProfile->GetPrimaryPad(),
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
