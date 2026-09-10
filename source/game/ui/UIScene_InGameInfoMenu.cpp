#include "game/ui/UIScene_InGameInfoMenu.hpp"

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
#include "game/world/generation/Player.hpp"
#include "vendor/_XPARTY_USER_INFO.hpp"

extern CMinecraftApp* g_pApp;
extern ConsoleUIController gConsoleUIController;

struct _XPARTY_USER_LIST
{
    unsigned int count;
    _XPARTY_USER_INFO users[4];
};

struct InGamePlayerOptionsData
{
    int playerId;
    unsigned char smallId;
    unsigned int privileges;
};

extern unsigned int XPartyGetUserList(_XPARTY_USER_LIST* users);

UIScene_InGameInfoMenu::UIScene_InGameInfoMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), IUIScene_PlayerList(), m_localPlayerIsModerator(false), m_players(),
      m_optionsButton(), m_playerList(), m_titleLabel()
{
    initialiseMovie();
    m_optionsButton.init(UIString(L10N::GetString(0x888510b3)), 0);
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
    boost::shared_ptr<Player> localPlayer = Minecraft::GetInstance()->GetPlayerByPlayerIndex(GetPad());
    m_localPlayerIsModerator = localPlayer != 0 && localPlayer->isModerator();
    if (!m_localPlayerIsModerator)
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
        return g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xf)) != 0;
    if (player == NULL)
        return true;
    if (g_pApp->GetGameHostOption(static_cast<eGameHostOption>(0xc)) == 0)
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
        partyTooltip = static_cast<int>(0xe7e99795);
    if (selected >= 0 && selected < GetPlayerCount())
    {
        PlayerInfo* info = GetPlayer(selected);
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(info->smallId);
        if (m_optionsButton.hasFocus())
            tooltip = static_cast<int>(0x8d6323ed);
        else if (player != NULL && hasSomethingToEdit(info->smallId))
            tooltip = static_cast<int>(0x6fb35f79);
        else if (player != NULL && !player->IsHost())
            tooltip = static_cast<int>(0x3e96698f);
    }
    gConsoleUIController.SetTooltips(GetPad(), tooltip, static_cast<int>(0x4f6bcf5e), partyTooltip,
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
    UIScene::handleReload();
    for (std::vector<PlayerInfo*>::iterator it = m_players.begin(); it != m_players.end(); ++it)
        delete *it;
    m_players.clear();
    m_playerList.clearList();
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
    boost::shared_ptr<Player> localPlayer = Minecraft::GetInstance()->GetPlayerByPlayerIndex(GetPad());
    m_localPlayerIsModerator = localPlayer != 0 && localPlayer->isModerator();
    if (!m_localPlayerIsModerator)
        removeControl(&m_optionsButton, false);
    updateTooltips();
}

std::wstring UIScene_InGameInfoMenu::getMoviePath()
{
    return std::wstring(g_pApp->GetLocalPlayerCount() < 2 ? L"InGameInfoMenu" : L"InGameInfoMenuSplit");
}

void UIScene_InGameInfoMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                         bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(GetPad(), action, repeat, released, pressed);
    if (action == 0x15 && pressed && !repeat)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
        navigateBack();
        return;
    }
    UIScene::handleInput(playerId, action, repeat, released, pressed, handled);
}

void UIScene_InGameInfoMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(
            GetPad(), IUIScene_PlayerList::OnPlayerChanged, this);
}

void UIScene_InGameInfoMenu::handlePress(int first, int second)
{
    gConsoleUIController.PlayUISFX(SoundEvents::UI_PRESS);
    if (first == 0)
    {
        gConsoleUIController.NavigateToScene(GetPad(), static_cast<EUIScene>(0x28), NULL,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
        return;
    }
    if (first != 1 || second < 0 || second >= GetPlayerCount())
        return;
    PlayerInfo* info = GetPlayer(second);
    if (info != NULL && m_localPlayerIsModerator && hasSomethingToEdit(info->smallId))
    {
        InGamePlayerOptionsData* data = new InGamePlayerOptionsData;
        data->playerId = GetPad();
        data->smallId = info->smallId;
        data->privileges = g_pApp->GetPlayerPrivileges(info->smallId);
        gConsoleUIController.NavigateToScene(GetPad(), static_cast<EUIScene>(0x29), data,
                                             static_cast<EUILayer>(6), static_cast<EUIGroup>(7));
    }
}

void UIScene_InGameInfoMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(
        GetPad(), IUIScene_PlayerList::OnPlayerChanged, this);
    if (m_layer != NULL)
        m_layer->showComponent(GetPad(), static_cast<EUIScene>(0x48), false);
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
    return 30;
}
