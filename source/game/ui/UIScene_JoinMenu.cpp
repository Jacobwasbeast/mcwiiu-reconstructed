#include "game/ui/UIScene_JoinMenu.hpp"
#include <cstdlib>
#include <cstring>
#include <cwchar>
#include "game/client/audio/SoundEvents.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameType.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
std::wstring formatwstr(const wchar_t*, ...);
namespace
{
struct JoinMenuData
{
    FriendSessionInfo* session;
    unsigned int unused;
    bool matchmaking;
};

struct JoinMenuSignInData
{
    unsigned int (*callback)(void*, bool, int);
    unsigned int callbackId;
    bool allowGuests;
    bool onlineOnly;
};
}

UIScene_JoinMenu::UIScene_JoinMenu(int sceneType, void* data, UILayer* layer)
    : UIScene(sceneType, layer), m_friendSessionInfo(NULL), m_matchmaking(false),
      m_initialised(false), m_friendSessionUpdated(false), m_error(false)
{
    initialiseMovie();
    m_gameInfo.init(UIString(L""));
    JoinMenuData* value = static_cast<JoinMenuData*>(data);
    if (value != NULL)
    {
        m_friendSessionInfo = value->session;
        m_matchmaking = value->matchmaking;
    }
}
UIScene_JoinMenu::~UIScene_JoinMenu()
{
}

unsigned int UIScene_JoinMenu::GetCustomGameModeID()
{
    return m_friendSessionInfo == NULL ? 0 : m_friendSessionInfo->m_gameSessionData.m_miniGameId;
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
    gConsoleUIController.NavigateBack(scene->m_playerId, false, static_cast<EUIScene>(0x53),
                                      static_cast<EUILayer>(8));
    if (updated)
        scene->m_friendSessionUpdated = true;
    else
        scene->m_error = true;
}
void UIScene_JoinMenu::AddDescriptionForOption(std::vector<HtmlString>& descriptions, eGameHostOption option, int options)
{
    unsigned int title = 0;
    const wchar_t* value = L"";
    unsigned int state = g_pApp->GetGameHostOption((unsigned int)options, option);
    if (option == static_cast<eGameHostOption>(0))
    {
        title = 0xb55c15d6;
        value = L10N::GetString(state == 1 ? 0xb558efd9 : state == 2 ? 0x5d131124 :
                                state == 3 ? 0xf70f3270 : 0x4e52b99a);
    }
    else if (option == static_cast<eGameHostOption>(4))
    {
        title = 0x8ca04f68;
        value = L10N::GetString(state ? 0xc8e108fd : 0xf74dab3b);
    }
    else if (option == static_cast<eGameHostOption>(7))
    {
        title = 0xb0408364;
        value = L10N::GetString(state ? 0xf37d7beb : 0x1494c206);
    }
    else if (option == static_cast<eGameHostOption>(8))
    {
        title = 0xa79e83bb;
        value = L10N::GetString(state ? 0xc8e108fd : 0xf74dab3b);
    }
    else if (option == static_cast<eGameHostOption>(0xb))
    {
        title = 0x550744a3;
        value = L10N::GetString(state ? 0xc8e108fd : 0xf74dab3b);
    }
    else if (option == static_cast<eGameHostOption>(0xc) ||
             option == static_cast<eGameHostOption>(0xd) ||
             option == static_cast<eGameHostOption>(0xe))
    {
        title = option == static_cast<eGameHostOption>(0xc) ? 0x0927db69 :
                option == static_cast<eGameHostOption>(0xd) ? 0xd6b53f5b : 0xc412c970;
        value = L10N::GetString(state ? 0xc8e108fd : 0xf74dab3b);
    }
    else
        return;
    descriptions.push_back(
        HtmlString(formatwstr(L"%ls %ls", L10N::GetString(title), value),
                   static_cast<eMinecraftColour>(0), false, false, 0));
}
int UIScene_JoinMenu::ErrorDialogReturned(void* data, int, C4JStorage::EMessageResult)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(data);
    if (scene != NULL)
        gConsoleUIController.NavigateBack(scene->m_playerId, false,
                                          static_cast<EUIScene>(0x53),
                                          static_cast<EUILayer>(8));
    return 0;
}
void UIScene_JoinMenu::tick()
{
    if (!m_initialised)
    {
        gConsoleUIController.NavigateToScene(m_playerId, static_cast<EUIScene>(0x4c), NULL,
                                             static_cast<EUILayer>(6),
                                             static_cast<EUIGroup>(7));
        CGameNetworkManager::sInstance.GetFullFriendSessionInfo(
            m_friendSessionInfo, friendSessionUpdated, this);
        m_initialised = true;
    }
    if (m_friendSessionUpdated)
    {
        m_friendSessionUpdated = false;
        m_joinButton.init(UIString(L10N::GetString(0x5483e75f)), 0);
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
                const wchar_t* gameMode = L10N::GetString(0x15aa0e94);
                if (sessionData.m_gameModeId == static_cast<unsigned int>(GameType::CREATIVE->getId()))
                    gameMode = L10N::GetString(0xb7c2fcaf);
                else if (sessionData.m_gameModeId == static_cast<unsigned int>(GameType::ADVENTURE->getId()))
                    gameMode = L10N::GetString(0xa48e97fa);
                descriptions.push_back(
                    HtmlString(formatwstr(L"%ls %ls", L10N::GetString(0x586b2084), gameMode),
                               static_cast<eMinecraftColour>(0), false, false, 0));
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(0),
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(4),
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(7),
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(0xb),
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(0xc),
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(0xd),
                                        sessionData.m_gameHostOptions);
                AddDescriptionForOption(descriptions, static_cast<eGameHostOption>(0xe),
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
        m_joinButton.init(UIString(L10N::GetString(0x5483e75f)), 0);
        m_gamePlayers.init(1);
        m_gameInfo.setLabel(UIString(L""), false, false);
        m_error = false;
        int parameter = 0xb9a1bf4d;
        gConsoleUIController.RequestErrorMessage(
            0x386c3a1a, 0xaab2b6dd, &parameter, 1, m_playerId,
            ErrorDialogReturned, this, NULL, true);
    }
    UIScene::tick();
}
void UIScene_JoinMenu::updateTooltips()
{
    int tooltip = getControlFocus() == 1 ? -1 : static_cast<int>(0x8d6323ed);
    gConsoleUIController.SetTooltips(0, tooltip, 0x4f6bcf5e, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
}
void UIScene_JoinMenu::updateComponents()
{
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x42), true);
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x43), true);
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
    if (action < 4 || (action > 5 && action < 8))
        return;
    if (action > 9 && action != 0x14)
    {
        if (action != 0x15 || !released)
            return;
        navigateBack();
        handled = true;
        return;
    }
    if (action == 0x14 && getControlFocus() == 1)
    {
        handled = true;
        return;
    }
    sendInputToMovie(action, repeat, released, pressed);
    handled = true;
}
void UIScene_JoinMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 1)
        m_gamePlayers.updateChildFocus(childFocus);
    UIScene::handleFocusChange(focus, childFocus);
}
void UIScene_JoinMenu::JoinGame(UIScene_JoinMenu* scene)
{
    if (scene == NULL || scene->m_friendSessionInfo == NULL)
        return;

    int playerMask = 0;
    bool multiplayerRestricted = false;
    bool allPlayersSignedInLive = true;
    if (!g_pApp->IsLocalMultiplayerAvailable())
    {
        int primaryPad = gProfile->GetPrimaryPad();
        if (gProfile->IsSignedIn(primaryPad))
        {
            multiplayerRestricted = !gProfile->AllowedToPlayMultiplayer(primaryPad);
            playerMask = CGameNetworkManager::GetLocalPlayerMask(primaryPad);
            allPlayersSignedInLive = gProfile->IsSignedInLive(primaryPad);
        }
    }
    else
    {
        for (int userIndex = 0; userIndex < 4; ++userIndex)
        {
            if (!gProfile->IsSignedIn(userIndex))
                continue;
            multiplayerRestricted |= !gProfile->AllowedToPlayMultiplayer(userIndex);
            playerMask |= CGameNetworkManager::GetLocalPlayerMask(userIndex);
            allPlayersSignedInLive &= gProfile->IsSignedInLive(userIndex);
        }
    }

    if (!allPlayersSignedInLive)
    {
        int parameter = 0xb9a1bf4d;
        scene->m_joining = false;
        gConsoleUIController.RequestErrorMessage(
            0x8a21894b, 0xb837024e, &parameter, 1, gProfile->GetPrimaryPad(),
            NULL, NULL, NULL, true);
        return;
    }
    if (scene->IsUGCRestrictedMode())
    {
        scene->setVisible(true);
        scene->m_joining = false;
        gConsoleUIController.RequestUGCMessageBox(0x4bb9d6ae, 0x4d669d55, -1, NULL, NULL);
        return;
    }
    if (multiplayerRestricted)
    {
        int parameter = 0xb9a1bf4d;
        scene->setVisible(true);
        scene->m_joining = false;
        gConsoleUIController.RequestErrorMessage(
            0x252ab13f, 0xcc5a590f, &parameter, 1, gProfile->GetPrimaryPad(),
            NULL, NULL, NULL, true);
        return;
    }

    unsigned int miniGameId = scene->GetCustomGameModeID();
    if (miniGameId != eMiniGameId_Any)
    {
        g_pApp->SetGameHostOption(static_cast<eGameHostOption>(0x1f), 1);
        g_pApp->SetFirstMiniGameType(static_cast<EMiniGameId>(miniGameId));
    }

    int result = CGameNetworkManager::sInstance.JoinGame(
        scene->m_friendSessionInfo, playerMask, scene->m_matchmaking);
    g_pApp->SetLiveLinkRequired(false);
    if (result == 0)
    {
        Minecraft::GetInstance()->SetJoiningFromFriendsTabOrInvite(false);
        return;
    }
    if (result == 1)
    {
        gConsoleUIController.NavigateBack(scene->m_playerId, false,
                                          static_cast<EUIScene>(0x53),
                                          static_cast<EUILayer>(8));
        return;
    }

    int title = 0x4bb9d6ae;
    int message = result == 2 ? 0xb86be603 :
                  result == 3 ? 0xaab2b6dd : 0xcc5a590f;
    if (result == 4)
        title = 0x252ab13f;
    if (result >= 2 && result <= 4)
    {
        int parameter = 0xb9a1bf4d;
        gConsoleUIController.RequestErrorMessage(
            title, message, &parameter, 1, gProfile->GetPrimaryPad(),
            NULL, NULL, NULL, true);
        gConsoleUIController.NavigateToHomeMenu(false);
        return;
    }
    gConsoleUIController.NavigateBack(scene->m_playerId, false,
                                      static_cast<EUIScene>(0x53),
                                      static_cast<EUILayer>(8));
}
int UIScene_JoinMenu::WiiUDRCWarningReturned(void* data, int,
                                             C4JStorage::EMessageResult result)
{
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(data);
    if (result == static_cast<C4JStorage::EMessageResult>(4))
        JoinGame(scene);
    if (scene != NULL)
        scene->m_joining = false;
    return 0;
}
unsigned int UIScene_JoinMenu::StartGame_SignInReturned(void* data, bool signedIn, int error)
{
    unsigned int callbackId = (unsigned int)(unsigned long)data;
    UIScene_JoinMenu* scene = static_cast<UIScene_JoinMenu*>(
        gConsoleUIController.GetSceneFromCallbackId(callbackId));
    if (scene != NULL)
    {
        if (signedIn && gProfile->IsSignedIn(error))
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
    JoinMenuSignInData signInData;
    signInData.callback = StartGame_SignInReturned;
    signInData.callbackId = GetCallbackUniqueId();
    signInData.allowGuests = true;
    signInData.onlineOnly = false;
    gConsoleUIController.NavigateToScene(
        gProfile->GetPrimaryPad(), static_cast<EUIScene>(0x4a), &signInData,
        static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
}
void UIScene_JoinMenu::handlePress(int first, int)
{
    if (first == 0)
    {
        m_joining = true;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BUTTON_CLICK);
        StartSharedLaunchFlow();
    }
}
int UIScene_JoinMenu::getSceneType()
{
    return 0x23;
}
