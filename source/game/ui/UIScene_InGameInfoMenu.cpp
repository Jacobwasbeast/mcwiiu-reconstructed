#include "game/ui/UIScene_InGameInfoMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/UIControl_PlayerButtonList.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_XPARTY_USER_INFO.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/PIXNamedEvents.hpp"

extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

struct InGamePlayerOptionsData
{
    int playerId;
    unsigned char smallId;
    unsigned int privileges;
};


UIScene_InGameInfoMenu::UIScene_InGameInfoMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), IUIScene_PlayerList(), m_isHost(false), m_players(),
      m_optionsButton(), m_playerList(), m_titleLabel()
{
    initialiseMovie();
    m_optionsButton.init(UIString(L10N::GetString(IDS_HOST_OPTIONS)), 0);
    m_titleLabel.init(UIString(L10N::GetString(0xfa81145)));
    m_playerList.init(1);

    int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
        if (player != NULL)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(player, info);
            m_players.push_back(info);
            m_playerList.addItem(info->name, static_cast<int>(info->colour),
                                 static_cast<int>(info->voiceStatus), true);
        }
    }
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(
        GetPad(), IUIScene_PlayerList::OnPlayerChanged, this);
    INetworkPlayer* localNetworkPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(GetPad());
    m_isHost = false;
    if (localNetworkPlayer != NULL)
        m_isHost = localNetworkPlayer->IsHost();
    if (!m_isHost && !Minecraft::GetInstance()->mLocalPlayers[GetPad()]->isModerator())
        removeControl(&m_optionsButton, false);
    updateTooltips();
    gConsoleUIController.HidePressStart();
}

UIScene_InGameInfoMenu::~UIScene_InGameInfoMenu()
{
    for (std::vector<PlayerInfo*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
        delete *it;
}

int UIScene_InGameInfoMenu::GetPlayerCount()
{
    return static_cast<int>(m_players.size());
}

PlayerInfo* UIScene_InGameInfoMenu::GetPlayer(int index)
{
    return m_players[index];
}

bool UIScene_InGameInfoMenu::RemovePlayer(unsigned char smallId)
{
    int index = FindPlayer(smallId);
    if (index < 0)
        return false;
    delete m_players[index];
    m_players.erase(m_players.begin() + index);
    return true;
}

void UIScene_InGameInfoMenu::AddPlayer(unsigned char smallId)
{
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
    if (player == NULL)
        return;
    PlayerInfo* info = CreatePlayerInfo();
    BuildPlayerInfo(player, info);
    m_players.push_back(info);
    m_playerList.addItem(info->name, static_cast<int>(info->colour),
                         static_cast<int>(info->voiceStatus), true);
}

UIControl_ButtonList* UIScene_InGameInfoMenu::GetPlayerList()
{
    return static_cast<UIControl_ButtonList*>(static_cast<void*>(&m_playerList));
}

void UIScene_InGameInfoMenu::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    IUIScene_PlayerList::UpdatePlayer(index, oldInfo, newInfo);
    if (newInfo->voiceStatus != oldInfo->voiceStatus || hasAPlayerChanged())
    {
        oldInfo->voiceStatus = newInfo->voiceStatus;
        m_playerList.setVOIPIcon(index, static_cast<int>(newInfo->voiceStatus));
    }
}

bool UIScene_InGameInfoMenu::hasSomethingToEdit(unsigned char smallId)
{
    INetworkPlayer* local = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(GetPad());
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
    if (local == player)
        return g_pApp->GetGameHostOption(eGameHostOption_CheatsEnabled) != 0;
    if (player == NULL)
        return true;
    if (g_pApp->GetGameHostOption(eGameHostOption_TrustPlayers) == 0)
        return true;
    if (local != NULL && local->IsHost())
        return true;
    if (player->IsHost())
        return false;
    return true;
}

void UIScene_InGameInfoMenu::updateTooltips()
{
    int selected = static_cast<int>(m_playerList.getCurrentSelection());
    int tooltip = -1;
    int partyTooltip = -1;
    _XPARTY_USER_LIST users;
    users.count = 0;
    if (XPartyGetUserList(&users) != 0 && users.count > 1 && !CGameNetworkManager::sInstance.IsLocalGame())
        partyTooltip = static_cast<int>(IDS_TOOLTIPS_INVITE_PARTY);
    if (selected >= 0 && selected < GetPlayerCount())
    {
        PlayerInfo* info = GetPlayer(selected);
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(info->smallId);
        if (m_optionsButton.hasFocus())
            tooltip = static_cast<int>(IDS_TOOLTIPS_SELECT);
        else if (player != NULL && hasSomethingToEdit(info->smallId))
            tooltip = static_cast<int>(IDS_TOOLTIPS_PRIVILEGES);
        else if (player != NULL && !player->IsHost())
            tooltip = static_cast<int>(IDS_TOOLTIPS_KICK);
    }
    gConsoleUIController.SetTooltips(GetPad(), tooltip, static_cast<int>(IDS_TOOLTIPS_BACK), partyTooltip,
                                     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                     false, false);
}

unsigned int UIScene_InGameInfoMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_optionsButton.setupControl(this, root, std::string("GameOptions"));
    m_controls.push_back(&m_optionsButton);
    m_playerList.setupControl(this, root, std::string("GamePlayers"));
    m_controls.push_back(&m_playerList);
    m_titleLabel.setupControl(this, root, std::string("Title"));
    m_controls.push_back(&m_titleLabel);
    return 1;
}

void UIScene_InGameInfoMenu::handleReload()
{
    int playerCount = CGameNetworkManager::sInstance.GetPlayerCount();
    for (unsigned int i = 0; i < m_players.size(); ++i)
        delete m_players[i];
    m_players.clear();
    for (int i = 0; i < playerCount; ++i)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerByIndex(i);
        if (player != NULL)
        {
            PlayerInfo* info = CreatePlayerInfo();
            BuildPlayerInfo(player, info);
            m_players.push_back(info);
            m_playerList.addItem(info->name, static_cast<int>(info->colour),
                                 static_cast<int>(info->voiceStatus), true);
        }
    }

    INetworkPlayer* localNetworkPlayer = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(GetPad());
    m_isHost = false;
    if (localNetworkPlayer != NULL)
        m_isHost = localNetworkPlayer->IsHost();
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[GetPad()];
    if (!m_isHost && !localPlayer->isModerator())
        removeControl(&m_optionsButton, false);
    updateTooltips();
    if (controlHasFocus(1))
        m_playerList.setCurrentSelection(getControlChildFocus(), false, true);
}

std::wstring UIScene_InGameInfoMenu::getMoviePath()
{
    return std::wstring(g_pApp->GetLocalPlayerCount() < 2 ? L"InGameInfoMenu" : L"InGameInfoMenuSplit");
}

void UIScene_InGameInfoMenu::handleInput(int playerId, int action, bool repeat, bool pressed,
                                         bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(GetPad(), action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
    case EControllerActions_MenuPageUp:
    case EControllerActions_MenuPageDown:
    case EControllerActions_MenuOk:
        sendInputToMovie(action, repeat, pressed, released);
        break;
    case EControllerActions_MenuCancel:
        if (pressed && !repeat)
        {
            gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
            navigateBack();
        }
        break;
    default:
        break;
    }
}

void UIScene_InGameInfoMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(
            GetPad(), IUIScene_PlayerList::OnPlayerChanged, this);
}

void UIScene_InGameInfoMenu::handlePress(int controlId, int childId)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (controlId == 0)
    {
        gConsoleUIController.NavigateToScene(GetPad(), EUIScene_InGameHostOptionsMenu, NULL,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (controlId != 1)
        return;

    unsigned char smallId = m_players[childId]->smallId;
    INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(smallId);
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = Minecraft::GetInstance()->mLocalPlayers[GetPad()];
    if (!m_isHost && !localPlayer->isModerator())
        return;
    if (networkPlayer == NULL)
        return;

    if (hasSomethingToEdit(smallId))
    {
        InGamePlayerOptionsData* data = new InGamePlayerOptionsData;
        data->playerId = GetPad();
        data->smallId = smallId;
        data->privileges = g_pApp->GetPlayerPrivileges(smallId);
        gConsoleUIController.NavigateToScene(GetPad(), EUIScene_InGamePlayerOptionsMenu, data,
                                             EUILayer_Scene, EUIGroup_PlayerDefault);
        return;
    }
    if (networkPlayer->IsLocal() || networkPlayer->IsSameSystem(CGameNetworkManager::sInstance.GetHostPlayer()))
        return;

    unsigned char* kickedSmallId = new unsigned char(smallId);
    int parameters[2] = {static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL)};
    gConsoleUIController.RequestAlertMessage(
        IDS_UNLOCK_KICK_PLAYER_TITLE,
        Minecraft::InMiniGame(eMiniGameId_Any, true) ? IDS_UNLOCK_KICK_PLAYER_MINIGAMES : IDS_UNLOCK_KICK_PLAYER,
        parameters, 2, GetPad(), IUIScene_PlayerList::KickPlayerReturned, kickedSmallId, 0, true);
}

void UIScene_InGameInfoMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(
        GetPad(), IUIScene_PlayerList::OnPlayerChanged, this);
    if (m_layer != NULL)
        m_layer->showComponent(GetPad(), EUIScene_MenuBackground, false);
}

void UIScene_InGameInfoMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 1)
        m_playerList.updateChildFocus(childFocus);
    updateTooltips();
}

void UIScene_InGameInfoMenu::tick()
{
    UIScene::tick();
    IUIScene_PlayerList::tick();
}

int UIScene_InGameInfoMenu::GetPad()
{
    return m_playerId;
}

int UIScene_InGameInfoMenu::getSceneType()
{
    return EUIScene_InGameInfoMenu;
}
