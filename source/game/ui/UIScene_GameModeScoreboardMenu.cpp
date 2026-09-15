#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"

#include <stdio.h>
#include <wchar.h>

#include "game/core/Minecraft.hpp"
#include "game/core/ClientMasterGameMode.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/L10N.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/MG01ScoreboardLayout.hpp"
#include "game/core/MG02ScoreboardLayout.hpp"
#include "game/core/MG03ScoreboardLayout.hpp"
#include "game/core/MG04ScoreboardLayout.hpp"
#include "game/core/StatsUID.hpp"
#include "game/core/ColourTable.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/minigame/GameMedal.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/ui/FJ_GameModeScoreboard.hpp"
#include "game/ui/FJ_FuiNode.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/UILayer.hpp"
#include "game/ui/UIString.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Item.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "vendor/_XPARTY_USER_INFO.hpp"

struct _XPARTY_USER_LIST
{
    unsigned int count;
    _XPARTY_USER_INFO users[4];
};

extern unsigned int XPartyGetUserList(_XPARTY_USER_LIST* users);
std::wstring _toString(int value);

UIScene_GameModeScoreboardMenu::UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer)
    : IUIScene_PlayerList(), UIScene(type, layer), m_playerUid(0), m_localPlayerCanModerate(0),
      m_canKickPlayers(0), m_canVoteToKick(0), m_scoreboardLayout(0), m_initialisePending(0),
      m_registeredTextures(), m_columnCount(0),
      m_awardPanel(), m_mainPanel(), m_killBar(), m_playerTitleLabel(), m_stat1Label(),
      m_stat2Label(), m_stat3Label(), m_stat4Label(), m_stat5Label(), m_killedByLabel(),
      m_gamertagLabel(), m_playerList()
{
    int* options = static_cast<int*>(data);
    initialiseMovie();
    m_slotDrawCount = options[1];
    delete options;
    if (!CGameNetworkManager::sInstance.IsInPublicJoinableGame())
        m_canKickPlayers = 1;
    else
        m_canKickPlayers = m_slotDrawCount == 1;
    m_canVoteToKick = 0;
    if (m_slotDrawCount == 1 && !GetScoreboard()->HasStats())
        m_slotDrawCount = 0;
    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        m_scoreboardLayout = new MG02ScoreboardLayout();
    else if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        m_scoreboardLayout = new MG03ScoreboardLayout(static_cast<EScoreboardType>(m_slotDrawCount));
    else if (Minecraft::InMiniGame(eMiniGameId_Build, true))
        m_scoreboardLayout = new MG04ScoreboardLayout();
    else
        m_scoreboardLayout = new MG01ScoreboardLayout(static_cast<EScoreboardType>(m_slotDrawCount));
    CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(
        m_playerId, IUIScene_PlayerList::OnPlayerChanged, this);
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
    m_controls.push_back(&m_awardPanel);
    fuiRenderNode* awardPanel = m_awardPanel.getFuiRenderNode();
    m_stat1Label.setupControl(this, awardPanel, std::string("Stat1Label"));
    m_controls.push_back(&m_stat1Label);
    m_stat2Label.setupControl(this, awardPanel, std::string("Stat2Label"));
    m_controls.push_back(&m_stat2Label);
    m_stat3Label.setupControl(this, awardPanel, std::string("Stat3Label"));
    m_controls.push_back(&m_stat3Label);
    m_stat4Label.setupControl(this, awardPanel, std::string("Stat4Label"));
    m_controls.push_back(&m_stat4Label);
    m_stat5Label.setupControl(this, awardPanel, std::string("Stat5Label"));
    m_controls.push_back(&m_stat5Label);
    m_killBar.setupControl(this, awardPanel, std::string("KillBar"));
    m_controls.push_back(&m_killBar);
    m_killedByLabel.setupControl(this, awardPanel, std::string("KilledByLabel"));
    m_controls.push_back(&m_killedByLabel);
    m_gamertagLabel.setupControl(this, awardPanel, std::string("GamertagLabel"));
    m_controls.push_back(&m_gamertagLabel);
    m_mainPanel.setupControl(this, root, std::string("PanelScoreboard"));
    m_controls.push_back(&m_mainPanel);
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_playerTitleLabel.setupControl(this, panel, std::string("PlayerTitle"));
    m_controls.push_back(&m_playerTitleLabel);
    m_playerList.setupControl(this, panel, std::string("Scoreboard"));
    m_controls.push_back(&m_playerList);
    return 1;
}

boost::shared_ptr<MultiplayerLocalPlayer> UIScene_GameModeScoreboardMenu::GetPlayer()
{
    return Minecraft::GetInstance()->mLocalPlayers[m_playerId];
}

ClientScoreboard* UIScene_GameModeScoreboardMenu::GetScoreboard()
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = GetPlayer();
    if (player == 0 || player->GetLevel() == 0)
        return 0;
    return static_cast<MultiPlayerLevel*>(player->GetLevel())->GetClientScoreboard();
}

void UIScene_GameModeScoreboardMenu::registerTexture(int index)
{
    if (index >= 119 || m_registeredTextures[index] != 0)
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
            m_registeredTextures[index] = 1;
        }
    }
}

void UIScene_GameModeScoreboardMenu::UpdateStats(const StatsUID& uid)
{
    StatsUID empty = StatsUID::EMPTY_SUID();
    if (uid != empty)
    {
        GameMedal* medal = GetScoreboard()->GetMedal(uid);
        m_stat1Label.setLabel(UIString(medal == 0 ? 0xcf021a6 : medal->GetName()), false, false);
        m_stat2Label.setLabel(UIString(m_scoreboardLayout->getLabel2(uid)), false, false);
        m_stat3Label.setLabel(UIString(m_scoreboardLayout->getLabel3(uid)), false, false);
        m_stat4Label.setLabel(UIString(m_scoreboardLayout->getLabel4(uid)), false, false);
        m_stat5Label.setLabel(UIString(m_scoreboardLayout->getLabel5(uid)), false, false);
        m_killedByLabel.setLabel(UIString(m_scoreboardLayout->getLabel6(uid)), false, false);
        m_gamertagLabel.setLabel(UIString(m_scoreboardLayout->getLabel7(uid)), false, false);
        m_slotDrawPending = 1;
        m_playerUid = uid.GetID();
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
    m_scoreboardLayout->Init(m_playerId);
    if (m_slotDrawCount == 0)
        m_playerTitleLabel.init(UIString(0xfa81145a));
    else
    {
        m_playerTitleLabel.init(UIString(0xef8b6004));
        registerTexture(3);
        registerTexture(0x74);
        registerTexture(0x75);
        registerTexture(0x76);
        registerTexture(0x70);
        registerTexture(0x71);
        registerTexture(0x72);
        m_killBar.init(UIString(L"%"), 0, 0, 100, 0);
        m_killBar.setLabel(UIString(L"%"), false, false);
        m_killBar.setProgress(100);
        UpdateStats(uid);
    }
    EScoreboardType scoreboardType = static_cast<EScoreboardType>(m_slotDrawCount);
    SetStatIconTextureName(1, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon2(scoreboardType))));
    SetStatIconTextureName(2, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon3(scoreboardType))));
    SetStatIconTextureName(3, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon4(scoreboardType))));
    SetStatIconTextureName(4, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon5(scoreboardType))));
    for (int column = 0; column < m_columnCount; ++column)
        SetTitleIconTextureName(column, std::wstring(IScoreboardLayout::getIcon(static_cast<int>(m_scoreboardLayout->getTitleIcon(column, scoreboardType)))));

    int selectedPlayer = -1;
    for (unsigned int index = 0; index < static_cast<unsigned int>(m_scoreboardLayout->GetPlayerCount()); ++index)
    {
        INetworkPlayer* networkPlayer = CGameNetworkManager::sInstance.GetPlayerBySmallId(
            m_scoreboardLayout->getSmallIDOfPlayerAt(index));
        if (networkPlayer == 0)
            continue;
        StatsUID playerUid = StatsUID::NETWORKPLAYER_SUID(networkPlayer);
        m_playerList.addItem(m_scoreboardLayout->getNameOfPlayerAt(index),
                             m_scoreboardLayout->getColorStateOfPlayerAt(index),
                             m_scoreboardLayout->getVoiceStatusOfPlayerAt(index), true);
        int icon = 0x6e;
        if (m_slotDrawCount == 0)
            icon = m_scoreboardLayout->GetEndBarIcon(m_scoreboardLayout->GetPlayer(index));
        else
        {
            GameMedal* medal = GetScoreboard()->GetMedal(playerUid);
            if (medal != 0 && medal->GetId() >= 0)
            {
                icon = medal->GetId() + 0xc;
                registerTexture(icon);
            }
        }
        m_playerList.SetSlotTextureName(index, m_columnCount - 1,
                                        std::wstring(IScoreboardLayout::getIcon(icon)), true);
        std::wstring slots[7];
        for (int slot = 0; slot < 7; ++slot)
            slots[slot] = m_scoreboardLayout->GetRowLabel(slot, playerUid, scoreboardType);
        m_playerList.setSlotsText(index, slots[0], slots[1], slots[2], slots[3], slots[4],
                                  slots[5], slots[6], true);
        if (playerUid == uid)
            selectedPlayer = static_cast<int>(index);
    }
    m_playerList.setCurrentSelection(selectedPlayer, false, true);
    m_localPlayerCanModerate = player != 0 && player->IsHost();
    updateTooltips();
    if (controlHasFocus(0))
        m_playerList.setCurrentSelection(static_cast<int>(getControlChildFocus()), false, true);
    m_playerList.UpdateControl();
    gConsoleUIController.TouchBoxRebuild(this);
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
    if (m_initialisePending)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
        if (player != 0)
        {
            m_initialisePending = 0;
            InitialiseScoreboard(player);
        }
    }
    if (m_slotDrawPending)
    {
        StatsUID uid(m_playerUid);
        ClientScoreboard* scoreboard = GetScoreboard();
        GameMedal* medal;
        if (!m_scoreboardLayout->showLifeSpanProgress())
        {
            int progress = 0;
            std::wstring details = m_scoreboardLayout->GetCustomProgressDetails(uid, progress);
            wchar_t format[64];
            swprintf(format, 64, L"<font color=\"#%08x\"><p align=cen",
                     g_pApp->GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)));
            m_killBar.setLabel(UIString(std::wstring(format) + details + L"</p>"), false, false);
            m_killBar.setProgress(progress);
            medal = scoreboard->GetMedal(uid);
        }
        else
        {
            const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
            unsigned int rounds = miniGame->GetRoundCount();
            int totalRoundLength = 0;
            int totalLifeSpan = 0;
            unsigned int roundsSurvived = 0;
            for (unsigned int round = 0; round < rounds; ++round)
            {
                totalRoundLength += scoreboard->GetRoundLength(round);
                totalLifeSpan += scoreboard->GetLifeSpan(uid, round);
                if (scoreboard->GetSurvived(uid, round))
                    ++roundsSurvived;
            }
            int progress = 100;
            if (totalRoundLength != 0)
                progress = static_cast<int>(static_cast<float>(totalLifeSpan) /
                                            static_cast<float>(totalRoundLength) * 100.0f);
            std::wstring details = L10N::GetString(0x6f5446df);
            details += _toString(progress);
            details += L"%";
            wchar_t format[64];
            swprintf(format, 64, L"<font color=\"#%08x\"><p align=cen",
                     g_pApp->GetHTMLColour(static_cast<eMinecraftColour>(0x1b5)));
            m_killBar.setLabel(UIString(std::wstring(format) + details + L"</p>"), false, false);
            m_killBar.setProgress(progress);
            if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1)
            {
                std::wstring killer = scoreboard->GetKiller(uid, -1, -1);
                SetHasPlayerBeenKilled(!scoreboard->GetSurvived(uid, -1) && !killer.empty());
            }
            else
            {
                SetHasPlayerBeenKilled(roundsSurvived != rounds);
            }
            medal = scoreboard->GetMedal(uid);
        }
        int icon = medal == 0 || medal->GetId() < 0 ? 0x6e : medal->GetId() + 0xc;
        SetStatIconTextureName(0, std::wstring(IScoreboardLayout::getIcon(icon)));
        m_slotDrawPending = 0;
    }
}

bool UIScene_GameModeScoreboardMenu::CanKickPlayer(INetworkPlayer* player)
{
    if (player == 0)
        return false;
    boost::shared_ptr<MultiplayerLocalPlayer> localPlayer = GetPlayer();
    bool permitted = m_localPlayerCanModerate != 0 || localPlayer->isModerator();
    INetworkPlayer* host = CGameNetworkManager::sInstance.GetHostPlayer();
    bool protectedPlayer = player->IsLocal() || player->IsSameSystem(host);
    if (m_canKickPlayers != 0)
        return permitted && !protectedPlayer;
    if (m_canVoteToKick != 0)
        return !protectedPlayer && Minecraft::GetInstance()->GetClientMasterGameMode()->GetKickVote(
                                       m_playerId, player->GetSmallId()) == 0;
    return false;
}

void UIScene_GameModeScoreboardMenu::updateTooltips()
{
    int partyTooltip = -1;
    _XPARTY_USER_LIST users;
    users.count = 0;
    if (XPartyGetUserList(&users) != 0 && users.count > 1 &&
        !CGameNetworkManager::sInstance.IsLocalGame())
        partyTooltip = static_cast<int>(0xe7e99795);

    int selected = static_cast<int>(m_playerList.getCurrentSelection());
    if (selected < 0 || selected >= m_scoreboardLayout->GetPlayerCount() ||
        !m_scoreboardLayout->doesExist(static_cast<unsigned int>(selected)))
    {
        gConsoleUIController.SetTooltips(m_playerId, -1, 0x4f6bcf5e, partyTooltip,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, false, false);
        return;
    }

    unsigned int index = static_cast<unsigned int>(selected);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
        m_scoreboardLayout->getSmallIDOfPlayerAt(index));
    int kickTooltip = CanKickPlayer(player) ? static_cast<int>(0x3e96698f) : -1;
    int playerTooltip = static_cast<int>(0xaec93217);
    if (player != 0 && player->IsLocal() && player->GetUserIndex() == m_playerId)
        playerTooltip = static_cast<int>(0x85953770);
    gConsoleUIController.SetTooltips(m_playerId, kickTooltip, 0x4f6bcf5e, partyTooltip,
                                     playerTooltip, -1, -1, -1, -1, -1, -1, -1, -1,
                                     -1, -1, -1, -1, false, false);
}

void UIScene_GameModeScoreboardMenu::handleGainFocus(bool hadFocus)
{
    if (hadFocus)
        CGameNetworkManager::sInstance.RegisterPlayerChangedCallback(
            m_playerId, IUIScene_PlayerList::OnPlayerChanged, this);
}

UIControl* UIScene_GameModeScoreboardMenu::GetMainPanel()
{
    return &m_mainPanel;
}

void UIScene_GameModeScoreboardMenu::customDraw(const char* name, fuiRect* rect)
{
    boost::shared_ptr<MultiplayerLocalPlayer> player = GetPlayer();
    if (player == 0 || player->GetLevel() == 0)
        return;
    wchar_t slotName[64];
    mbstowcs(slotName, name, 64);
    unsigned int selected = m_playerList.getCurrentSelection();
    boost::shared_ptr<ItemInstance> item = m_scoreboardLayout->getItemForSlot(
        slotName, selected, static_cast<EScoreboardType>(m_slotDrawCount));
    if (item != 0 && !item->isEmpty())
    {
        not_null_ptr<ItemInstance> drawItem(item);
        customDrawSlotControlFui(rect, m_playerId, drawItem, 1.0f,
                                 item->getItem()->isFoil(drawItem), false, false);
    }
}

void UIScene_GameModeScoreboardMenu::handleInput(int playerId, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, released, pressed);
    if (action == 2 && pressed && !repeat)
    {
        CGameNetworkManager::sInstance.IsLocalGame();
        return;
    }
    if (action == 3 && pressed && m_playerList.hasFocus() && m_playerList.getItemCount() > 0)
    {
        int selected = static_cast<int>(m_playerList.getCurrentSelection());
        if (selected >= 0 && selected < m_scoreboardLayout->GetPlayerCount())
        {
            unsigned int index = static_cast<unsigned int>(selected);
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
                m_scoreboardLayout->getSmallIDOfPlayerAt(index));
            if (player != 0)
                player->GetUID() != PlayerUID(0);
        }
        return;
    }
    if ((action >= 4 && action < 6) || action == 0x14)
        sendInputToMovie(action, repeat, released, pressed);
    else if (action == 0x15 && pressed && !repeat)
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_BACK);
        navigateBack();
    }
}

void UIScene_GameModeScoreboardMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(
        m_playerId, IUIScene_PlayerList::OnPlayerChanged, this);
    m_layer->showComponent(m_playerId, static_cast<EUIScene>(0x48), false);
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
    for (int index = 0; index < 119; ++index)
        m_registeredTextures[index] = 0;
    m_scoreboardLayout->ClearPlayersInfo();
    m_playerList.init(0);
    setScoreboardType(m_slotDrawCount);
    const int textures[] = { 0, 1, 0x6e, 4, 6, 7, 5, 0x73, 0x6f };
    for (unsigned int index = 0; index < sizeof(textures) / sizeof(textures[0]); ++index)
        registerTexture(textures[index]);
    m_columnCount = static_cast<int>(m_scoreboardLayout->getNumberOfColumns());
    initNumColumns(m_columnCount);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetLocalPlayerByUserIndex(m_playerId);
    m_initialisePending = player == 0;
    if (player != 0)
        InitialiseScoreboard(player);
}

void UIScene_GameModeScoreboardMenu::handlePress(int first, int second)
{
    if (first != 0 || (!m_canKickPlayers && !m_canVoteToKick) || second < 0 ||
        second >= m_scoreboardLayout->GetPlayerCount())
        return;
    unsigned int index = static_cast<unsigned int>(second);
    if (!m_scoreboardLayout->doesExist(index))
        return;
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
        m_scoreboardLayout->getSmallIDOfPlayerAt(index));
    if (!CanKickPlayer(player))
        return;
    unsigned char* smallId = new unsigned char(m_scoreboardLayout->getSmallIDOfPlayerAt(index));
    int parameters[2] = { static_cast<int>(0xb9a1bf4d), static_cast<int>(0x4fcbc7e7) };
    int message = Minecraft::InMiniGame(eMiniGameId_Any, true)
                      ? static_cast<int>(0x2cf43e54)
                      : static_cast<int>(0x0b5f4b25);
    gConsoleUIController.RequestAlertMessage(
        0x628f8b32, message, parameters, 2, m_playerId,
        (int (*)(void*, int, C4JStorage::EMessageResult))IUIScene_PlayerList::KickPlayerReturned,
        smallId, 0, true);
}

void UIScene_GameModeScoreboardMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
        m_playerList.updateChildFocus(childFocus);
    updateTooltips();
    if (m_slotDrawCount == 1)
    {
        int selected = static_cast<int>(m_playerList.getCurrentSelection());
        unsigned int index = static_cast<unsigned int>(selected);
        if (selected >= 0 && selected < m_scoreboardLayout->GetPlayerCount() &&
            m_scoreboardLayout->doesExist(index))
        {
            INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
                m_scoreboardLayout->getSmallIDOfPlayerAt(index));
            if (player != 0)
                UpdateStats(StatsUID::NETWORKPLAYER_SUID(player));
        }
    }
}

uint64_t UIScene_GameModeScoreboardMenu::CheckReloadMovie()
{
    if (m_scoreboardLayout != 0 && m_columnCount != static_cast<int>(m_scoreboardLayout->getNumberOfColumns()))
        reloadMovie(false);
    return 0;
}

void UIScene_GameModeScoreboardMenu::BuildPlayerInfo(INetworkPlayer* player, PlayerInfo* info)
{
    m_scoreboardLayout->BuildPlayerInfo(player, info);
}

PlayerInfo* UIScene_GameModeScoreboardMenu::CreatePlayerInfo()
{
    return m_scoreboardLayout->CreatePlayerInfo();
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
    m_scoreboardLayout->AddPlayer(smallId);
}

UIControl_ButtonList* UIScene_GameModeScoreboardMenu::GetPlayerList()
{
    return &m_playerList;
}

void UIScene_GameModeScoreboardMenu::UpdatePlayer(int index, PlayerInfo* oldInfo, PlayerInfo* newInfo)
{
    IUIScene_PlayerList::UpdatePlayer(index, oldInfo, newInfo);
    bool changed = hasAPlayerChanged();
    if (changed)
    {
        INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(newInfo->smallId);
        if (m_slotDrawCount == 0)
        {
            int icon = m_scoreboardLayout->GetEndBarIcon(newInfo);
            m_playerList.SetSlotTextureName(index, m_columnCount - 1,
                                            std::wstring(IScoreboardLayout::getIcon(icon)), true);
            std::wstring slots[7];
            if (player != 0)
            {
                StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
                for (int slot = 0; slot < 7; ++slot)
                    slots[slot] = m_scoreboardLayout->GetRowLabel(
                        slot, uid, static_cast<EScoreboardType>(m_slotDrawCount));
            }
            m_playerList.setSlotsText(index, slots[0], slots[1], slots[2], slots[3],
                                      slots[4], slots[5], slots[6], true);
        }
        else if (player != 0)
        {
            StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
            GameMedal* medal = GetScoreboard()->GetMedal(uid);
            int icon = medal == 0 || medal->GetId() < 0 ? 0x6e : medal->GetId() + 0xc;
            if (medal != 0 && medal->GetId() >= 0)
                registerTexture(icon);
            m_playerList.SetSlotTextureName(index, m_columnCount - 1,
                                            std::wstring(IScoreboardLayout::getIcon(icon)), true);
            std::wstring slots[7];
            for (int slot = 0; slot < 7; ++slot)
                slots[slot] = m_scoreboardLayout->GetRowLabel(
                    slot, uid, static_cast<EScoreboardType>(m_slotDrawCount));
            m_playerList.setSlotsText(index, slots[0], slots[1], slots[2], slots[3],
                                      slots[4], slots[5], slots[6], true);
        }
    }
    if (newInfo->colour != oldInfo->colour || changed)
    {
        oldInfo->colour = newInfo->colour;
        m_playerList.setPlayerIcon(index, newInfo->colour);
    }
    if (newInfo->voiceStatus != oldInfo->voiceStatus || changed)
    {
        oldInfo->voiceStatus = newInfo->voiceStatus;
        m_playerList.setVOIPIcon(index, newInfo->voiceStatus);
    }
    m_scoreboardLayout->UpdatePlayer(index, oldInfo, newInfo, changed);
}

int UIScene_GameModeScoreboardMenu::getSceneType()
{
    return 60;
}

int UIScene_GameModeScoreboardMenu::GetPad()
{
    return m_playerId;
}
