#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ClientScoreboard.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/FJ_GameModeScoreboard.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIControl_List.hpp"

UIScene_GameModeScoreboardMenu::UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer)
    : IUIScene_PlayerList(), UIScene(type, layer), m_playerUid(0), m_hasStats(0), m_unknown109(0),
      m_unknown10a(0), m_unknown10b(0), m_state(), m_hasPlayerBeenKilled(0), m_scoreboardReady(0),
      m_unknown127(0), m_scoreboardLayout(0), m_scoreboardType(0), m_state12d(), m_killBarValue(0),
      m_state168(), m_hasFocusSelection(0), m_state179(), m_numPlayers(0), m_selectedPlayer(-1),
      m_numColumns(0), m_state194(), m_columnCount(0), m_awardPanel(), m_mainPanel(), m_killBar(),
      m_stat1Label(), m_stat2Label(), m_stat3Label(), m_stat4Label(), m_stat5Label(),
      m_playerTitleLabel(), m_killedByLabel(), m_gamertagLabel(), m_playerList()
{
    (void)data;
    m_scoreboardLayout = 0;
}

UIScene_GameModeScoreboardMenu::~UIScene_GameModeScoreboardMenu()
{
    delete m_scoreboardLayout;
    m_scoreboardLayout = 0;
}

std::wstring UIScene_GameModeScoreboardMenu::getMoviePath()
{
    if (g_pApp != 0 && g_pApp->GetLocalPlayerCount() < 2)
        return L"GameModeScoreboard";
    return L"GameModeScoreboardSplit";
}

unsigned int UIScene_GameModeScoreboardMenu::mapElementsAndNames()
{
    return UIScene::mapElementsAndNames();
}

PlayerInfo* UIScene_GameModeScoreboardMenu::GetPlayer()
{
    return m_scoreboardLayout == 0 ? 0 : m_scoreboardLayout->GetPlayer(m_selectedPlayer);
}

ClientScoreboard* UIScene_GameModeScoreboardMenu::GetScoreboard()
{
    return 0;
}

void UIScene_GameModeScoreboardMenu::registerTexture(int index)
{
    const wchar_t* icon = IScoreboardLayout::getIcon(index);
    if (icon == 0)
        return;
    std::wstring name(icon);
    if (m_movie != 0 && g_pApp != 0 && g_pApp->hasArchiveFile(name, false))
    {
        g_pApp->getArchiveFile(name, false);
    }
}

void UIScene_GameModeScoreboardMenu::UpdateStats(const StatsUID& uid)
{
    if (m_scoreboardLayout == 0)
        return;
    (void)uid;
}

void UIScene_GameModeScoreboardMenu::SetStatIconTextureName(int index, const std::wstring& name)
{
    (void)index;
    (void)name;
}

void UIScene_GameModeScoreboardMenu::SetTitleIconTextureName(int index, const std::wstring& name)
{
    (void)index;
    (void)name;
}

void UIScene_GameModeScoreboardMenu::InitialiseScoreboard(INetworkPlayer* player)
{
    if (m_scoreboardLayout != 0)
        m_scoreboardLayout->ClearPlayersInfo();
    m_scoreboardReady = player != 0;
}

void UIScene_GameModeScoreboardMenu::SetHasPlayerBeenKilled(bool killed)
{
    m_hasPlayerBeenKilled = killed;
}

void UIScene_GameModeScoreboardMenu::tick()
{
    IUIScene_PlayerList::tick();
    if (m_scoreboardLayout != 0)
        m_numPlayers = m_scoreboardLayout->GetPlayerCount();
    updateTooltips();
}

bool UIScene_GameModeScoreboardMenu::CanKickPlayer(INetworkPlayer* player)
{
    return player != 0 && player->IsHost() == false;
}

void UIScene_GameModeScoreboardMenu::updateTooltips()
{
    return;
}

void UIScene_GameModeScoreboardMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        return;
    IUIScene_PlayerList::tick();
}

unsigned int UIScene_GameModeScoreboardMenu::GetMainPanel()
{
    return 0;
}

void UIScene_GameModeScoreboardMenu::customDraw(const char* name, fuiRect* rect)
{
    (void)name;
    (void)rect;
}

void UIScene_GameModeScoreboardMenu::handleInput(int playerId, int action, bool repeat, bool released,
                                                  bool pressed, bool& handled)
{
    (void)playerId;
    (void)action;
    (void)repeat;
    (void)released;
    (void)pressed;
    handled = false;
}

void UIScene_GameModeScoreboardMenu::handleDestroy()
{
    IUIScene_PlayerList::tick();
}

void UIScene_GameModeScoreboardMenu::setScoreboardType(int type)
{
    m_scoreboardType = static_cast<unsigned char>(type);
}

void UIScene_GameModeScoreboardMenu::initNumColumns(int columns)
{
    m_numColumns = columns;
}

void UIScene_GameModeScoreboardMenu::handleReload()
{
    InitialiseScoreboard(0);
    registerTexture(0);
}

void UIScene_GameModeScoreboardMenu::handlePress(int first, int second)
{
    (void)first;
    if (second >= 0 && m_scoreboardLayout != 0 && second < m_scoreboardLayout->GetPlayerCount())
        m_selectedPlayer = second;
}

void UIScene_GameModeScoreboardMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
        m_playerList.updateChildFocus(childFocus);
    updateTooltips();
}

uint64_t UIScene_GameModeScoreboardMenu::CheckReloadMovie()
{
    if (m_scoreboardLayout != 0)
        m_columnCount = m_scoreboardLayout->getNumberOfColumns();
    return 0;
}

void UIScene_GameModeScoreboardMenu::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    if (m_scoreboardLayout != 0)
        m_scoreboardLayout->BuildPlayerInfo(player, info);
}

PlayerInfo* UIScene_GameModeScoreboardMenu::CreatePlayerInfo()
{
    return m_scoreboardLayout == 0 ? 0 : m_scoreboardLayout->GetPlayer(0);
}

int UIScene_GameModeScoreboardMenu::GetPlayerCount()
{
    return m_scoreboardLayout == 0 ? 0 : m_scoreboardLayout->GetPlayerCount();
}

PlayerInfo* UIScene_GameModeScoreboardMenu::GetPlayer(int index)
{
    return m_scoreboardLayout == 0 ? 0 : m_scoreboardLayout->GetPlayer(index);
}

bool UIScene_GameModeScoreboardMenu::RemovePlayer(unsigned char smallId)
{
    int index = FindPlayer(smallId);
    return m_scoreboardLayout != 0 && m_scoreboardLayout->RemovePlayer(index);
}

void UIScene_GameModeScoreboardMenu::AddPlayer(unsigned char smallId)
{
    (void)smallId;
}

UIControl_ButtonList* UIScene_GameModeScoreboardMenu::GetPlayerList()
{
    return 0;
}

void UIScene_GameModeScoreboardMenu::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    IUIScene_PlayerList::UpdatePlayer(index, oldInfo, newInfo);
}

int UIScene_GameModeScoreboardMenu::getSceneType()
{
    return 60;
}

int UIScene_GameModeScoreboardMenu::GetPad()
{
    return 0;
}
