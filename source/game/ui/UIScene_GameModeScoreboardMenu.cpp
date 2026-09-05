#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"

#include <stdio.h>
#include <wchar.h>

#include "game/core/Minecraft.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/MG01ScoreboardLayout.hpp"
#include "game/core/MG02ScoreboardLayout.hpp"
#include "game/core/MG03ScoreboardLayout.hpp"
#include "game/core/MG04ScoreboardLayout.hpp"
#include "game/core/StatsUID.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/FJ_GameModeScoreboard.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"

UIScene_GameModeScoreboardMenu::UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer)
    : IUIScene_PlayerList(), UIScene(type, layer), m_playerUid(0), m_playerAvailable(0),
      m_canKickPlayers(0), m_kickVoteEnabled(0), m_playerStatsValid(0),
      m_scoreboardInitialised(0), m_playerSelectionState(0), m_playerSelectionPadding(0),
      m_scoreboardLayout(0), m_scoreboardMode(0), m_registeredTextures(), m_killBarValue(0),
      m_unusedState(), m_hasFocusSelection(0), m_unusedFocusState(), m_numPlayers(0),
      m_selectedPlayer(-1), m_numColumns(0), m_unusedColumnState(), m_columnCount(0),
      m_awardPanel(), m_mainPanel(), m_killBar(), m_playerTitleLabel(), m_stat1Label(),
      m_stat2Label(), m_stat3Label(), m_stat4Label(), m_stat5Label(), m_killedByLabel(),
      m_gamertagLabel(), m_playerList()
{
    int* options = static_cast<int*>(data);
    initialiseMovie();
    if (options != 0)
    {
        m_scoreboardMode = static_cast<unsigned char>(options[1]);
        delete options;
    }
    if (m_scoreboardMode == 1 && GetScoreboard() != 0 && GetScoreboard()->HasStats() == false)
        m_scoreboardMode = 0;
    if (Minecraft::InMiniGame(static_cast<EMiniGameId>(1), true))
        m_scoreboardLayout = new MG01ScoreboardLayout(static_cast<EScoreboardType>(0));
    else if (Minecraft::InMiniGame(static_cast<EMiniGameId>(2), true))
        m_scoreboardLayout = new MG02ScoreboardLayout();
    else if (Minecraft::InMiniGame(static_cast<EMiniGameId>(3), true))
        m_scoreboardLayout = new MG03ScoreboardLayout(static_cast<EScoreboardType>(0));
    else
        m_scoreboardLayout = new MG04ScoreboardLayout();
    handleReload();
}

UIScene_GameModeScoreboardMenu::~UIScene_GameModeScoreboardMenu()
{
    delete m_scoreboardLayout;
    m_scoreboardLayout = 0;
}

std::wstring UIScene_GameModeScoreboardMenu::getMoviePath()
{
    if (g_pApp->GetLocalPlayerCount() < 2)
        return std::wstring(L"GameModeScoreboard");
    return std::wstring(L"GameModeScoreboardSplit");
}

unsigned int UIScene_GameModeScoreboardMenu::mapElementsAndNames()
{
    fuiRenderNode* root = m_movie->getRootNode();
    m_awardPanel.setupControl(this, root, std::string("PanelAward"));
    m_mainPanel.setupControl(this, root, std::string("PanelScoreboard"));
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_playerTitleLabel.setupControl(this, panel, std::string("PlayerTitle"));
    m_stat1Label.setupControl(this, panel, std::string("Stat1Label"));
    m_stat2Label.setupControl(this, panel, std::string("Stat2Label"));
    m_stat3Label.setupControl(this, panel, std::string("Stat3Label"));
    m_stat4Label.setupControl(this, panel, std::string("Stat4Label"));
    m_stat5Label.setupControl(this, panel, std::string("Stat5Label"));
    m_killBar.setupControl(this, panel, std::string("KillBar"));
    m_killedByLabel.setupControl(this, panel, std::string("KilledByLabel"));
    m_gamertagLabel.setupControl(this, panel, std::string("GamertagLabel"));
    m_playerList.setupControl(this, panel, std::string("Scoreboard"));
    return 1;
}

boost::shared_ptr<MultiplayerLocalPlayer> UIScene_GameModeScoreboardMenu::GetPlayer()
{
    return Minecraft::GetInstance()->mLocalPlayers[m_resolution];
}

ClientScoreboard* UIScene_GameModeScoreboardMenu::GetScoreboard()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = GetPlayer();
    if (player == 0 || player->getLevel() == 0)
        return 0;
    return player->getLevel()->GetClientScoreboard();
}

void UIScene_GameModeScoreboardMenu::registerTexture(int index)
{
    if (index < 0 || index >= 119 || m_registeredTextures[index - 12] != 0)
        return;
    wchar_t path[64];
    swprintf(path, 64, L"Graphics_GameModeGraphics_%ls.png", IScoreboardLayout::getIcon(index));
    std::wstring name(path);
    if (g_pApp->hasArchiveFile(name, false))
    {
        arrayWithLength<unsigned char> data = g_pApp->getArchiveFile(name, false);
        if (data.data != 0 && data.length != 0)
        {
            registerSubstitutionTexture(std::wstring(IScoreboardLayout::getIcon(index)), data.data,
                                        data.length, true);
            m_registeredTextures[index - 12] = 1;
        }
    }
}

void UIScene_GameModeScoreboardMenu::UpdateStats(const StatsUID& uid)
{
    StatsUID empty = StatsUID::EMPTY_SUID();
    if (uid != empty)
    {
        int medal = GetScoreboard()->GetMedal(uid);
        m_stat1Label.setLabel(UIString(medal == 0 ? 0xcf021a6 : medal), false, false);
        m_stat2Label.setLabel(UIString(m_scoreboardLayout->getLabel2(uid)), false, false);
        m_stat3Label.setLabel(UIString(m_scoreboardLayout->getLabel3(uid)), false, false);
        m_stat4Label.setLabel(UIString(m_scoreboardLayout->getLabel4(uid)), false, false);
        m_stat5Label.setLabel(UIString(m_scoreboardLayout->getLabel5(uid)), false, false);
        m_killedByLabel.setLabel(UIString(m_scoreboardLayout->getLabel6(uid)), false, false);
        m_gamertagLabel.setLabel(UIString(m_scoreboardLayout->getLabel7(uid)), false, false);
        m_playerStatsValid = 1;
    }
    else
    {
        m_playerTitleLabel.init(UIString(0x4e925ce));
        m_stat1Label.setLabel(UIString(L"%"), false, false);
        m_stat2Label.setLabel(UIString(L"%"), false, false);
        m_stat3Label.setLabel(UIString(L"%"), false, false);
        m_stat4Label.setLabel(UIString(L"%"), false, false);
        m_stat5Label.setLabel(UIString(L"%"), false, false);
        m_killedByLabel.setLabel(UIString(0x7eadee66), false, false);
        m_gamertagLabel.setLabel(UIString(L"%"), false, false);
    }
}

void UIScene_GameModeScoreboardMenu::SetStatIconTextureName(int index, const std::wstring& name)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    document->SetStatIconTextureName(index, name);
}

void UIScene_GameModeScoreboardMenu::SetTitleIconTextureName(int index, const std::wstring& name)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    document->SetTitleIconTextureName(index, name);
}

void UIScene_GameModeScoreboardMenu::InitialiseScoreboard(INetworkPlayer* player)
{
    StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    m_scoreboardLayout->ClearPlayersInfo();
    if (m_scoreboardMode == 0)
        m_playerTitleLabel.init(UIString(0xfa81145));
    else
    {
        m_playerTitleLabel.init(UIString(0xef8b6004));
        m_killBar.init(UIString(L"%"), 0, 0, 100, 0);
        m_killBar.setLabel(UIString(L"%"), false, false);
        m_killBar.setProgress(100);
        UpdateStats(uid);
    }
    m_numColumns = static_cast<int>(m_scoreboardLayout->getNumberOfColumns());
    EScoreboardType scoreboardType = static_cast<EScoreboardType>(m_scoreboardMode);
    for (int column = 0; column < m_numColumns; ++column)
        SetTitleIconTextureName(column, std::wstring(IScoreboardLayout::getIcon(static_cast<int>(m_scoreboardLayout->getTitleIcon(column, scoreboardType)))));
    m_scoreboardInitialised = 1;
}

void UIScene_GameModeScoreboardMenu::SetHasPlayerBeenKilled(bool killed)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode()->asFJ_Document());
    document->SetHasPlayerBeenKilled(killed);
}

void UIScene_GameModeScoreboardMenu::tick()
{
    UIScene::tick();
    IUIScene_PlayerList::tick();
    if (m_playerStatsValid != 0)
        UpdateStats(m_playerUid);
    updateTooltips();
}

bool UIScene_GameModeScoreboardMenu::CanKickPlayer(INetworkPlayer* player)
{
    if (player == 0)
        return false;
    bool permitted = !player->IsHost();
    if (m_canKickPlayers != 0)
        return permitted;
    if (m_kickVoteEnabled != 0)
        return permitted;
    return false;
}

void UIScene_GameModeScoreboardMenu::updateTooltips()
{
    IUIScene_PlayerList::tick();
}

void UIScene_GameModeScoreboardMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        registerTexture(3);
}

unsigned int UIScene_GameModeScoreboardMenu::GetMainPanel()
{
    return (unsigned int)(uintptr_t)&m_mainPanel;
}

void UIScene_GameModeScoreboardMenu::customDraw(const char* name, fuiRect* rect)
{
    UIScene::customDraw(name, rect);
}

void UIScene_GameModeScoreboardMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    UIScene::handleInput(playerId, action, repeat, released, pressed, handled);
}

void UIScene_GameModeScoreboardMenu::handleDestroy()
{
    UIScene::handleDestroy();
    if (m_layer != 0)
        m_layer->showComponent(m_resolution, static_cast<EUIScene>(0x48), false);
}

void UIScene_GameModeScoreboardMenu::setScoreboardType(int type)
{
    static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode()->asFJ_Document())->SetScoreboardType(type);
}

void UIScene_GameModeScoreboardMenu::initNumColumns(int columns)
{
    static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode()->asFJ_Document())->InitNumColumns(columns);
}

void UIScene_GameModeScoreboardMenu::handleReload()
{
    UIScene::handleReload();
    if (m_scoreboardLayout != 0)
    {
        setScoreboardType(m_scoreboardMode);
        initNumColumns(static_cast<int>(m_scoreboardLayout->getNumberOfColumns()));
    }
}

void UIScene_GameModeScoreboardMenu::handlePress(int first, int second)
{
    if (first == 0 && second >= 0 && m_scoreboardLayout != 0 && second < m_scoreboardLayout->GetPlayerCount())
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
    if (m_scoreboardLayout != 0 && m_columnCount != static_cast<int>(m_scoreboardLayout->getNumberOfColumns()))
        handleReload();
    return 0;
}

void UIScene_GameModeScoreboardMenu::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    m_scoreboardLayout->BuildPlayerInfo(player, info);
}

PlayerInfo* UIScene_GameModeScoreboardMenu::CreatePlayerInfo()
{
    if (MG01ScoreboardLayout* layout = dynamic_cast<MG01ScoreboardLayout*>(m_scoreboardLayout))
        return layout->CreatePlayerInfo();
    if (MG02ScoreboardLayout* layout = dynamic_cast<MG02ScoreboardLayout*>(m_scoreboardLayout))
        return layout->CreatePlayerInfo();
    if (MG03ScoreboardLayout* layout = dynamic_cast<MG03ScoreboardLayout*>(m_scoreboardLayout))
        return layout->CreatePlayerInfo();
    return static_cast<MG04ScoreboardLayout*>(m_scoreboardLayout)->CreatePlayerInfo();
}

int UIScene_GameModeScoreboardMenu::GetPlayerCount()
{
    return m_scoreboardLayout->GetPlayerCount();
}

PlayerInfo* UIScene_GameModeScoreboardMenu::GetPlayer(int index)
{
    return m_scoreboardLayout->GetPlayer(index);
}

bool UIScene_GameModeScoreboardMenu::RemovePlayer(unsigned char smallId)
{
    return m_scoreboardLayout->RemovePlayer(FindPlayer(smallId));
}

void UIScene_GameModeScoreboardMenu::AddPlayer(unsigned char smallId)
{
    if (MG01ScoreboardLayout* layout = dynamic_cast<MG01ScoreboardLayout*>(m_scoreboardLayout))
        layout->AddPlayer(smallId);
    else if (MG02ScoreboardLayout* layout = dynamic_cast<MG02ScoreboardLayout*>(m_scoreboardLayout))
        layout->AddPlayer(smallId);
    else if (MG03ScoreboardLayout* layout = dynamic_cast<MG03ScoreboardLayout*>(m_scoreboardLayout))
        layout->AddPlayer(smallId);
    else if (m_scoreboardLayout != 0)
        static_cast<MG04ScoreboardLayout*>(m_scoreboardLayout)->AddPlayer(smallId);
}

UIControl_ButtonList* UIScene_GameModeScoreboardMenu::GetPlayerList()
{
    return 0;
}

void UIScene_GameModeScoreboardMenu::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    IUIScene_PlayerList::UpdatePlayer(index, oldInfo, newInfo);
    if (m_scoreboardLayout != 0 && newInfo != 0)
    {
        unsigned int playerIndex = static_cast<unsigned int>(index);
        m_playerList.setPlayerIcon(index, m_scoreboardLayout->getColorStateOfPlayerAt(playerIndex));
        m_playerList.setVOIPIcon(index, m_scoreboardLayout->getVoiceStatusOfPlayerAt(playerIndex));
    }
}

int UIScene_GameModeScoreboardMenu::getSceneType()
{
    return 60;
}

int UIScene_GameModeScoreboardMenu::GetPad()
{
    return m_resolution;
}
