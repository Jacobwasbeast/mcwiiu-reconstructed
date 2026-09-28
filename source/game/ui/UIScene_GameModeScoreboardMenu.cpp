#include "game/ui/UIScene_GameModeScoreboardMenu.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/core/GameSettings.hpp"
#include "game/core/PIXNamedEvents.hpp"

std::wstring _toString(int value);

UIScene_GameModeScoreboardMenu::UIScene_GameModeScoreboardMenu(int type, void* data, UILayer* layer)
    : IUIScene_PlayerList(), UIScene(type, layer), m_statsPending(false), m_awardPanel(), m_mainPanel(), m_killBar(),
      m_playerTitleLabel(), m_stat1Label(), m_stat2Label(), m_stat3Label(), m_stat4Label(), m_stat5Label(),
      m_killedByLabel(), m_gamertagLabel(), m_playerList()
{
    int* options = static_cast<int*>(data);
    initialiseMovie();
    m_scoreboardType = static_cast<EScoreboardType>(options[1]);
    delete options;
    if (!CGameNetworkManager::sInstance.IsInPublicJoinableGame())
        m_canKickPlayers = 1;
    else
        m_canKickPlayers = m_scoreboardType == EScoreboardType_Award;
    m_canVoteToKick = 0;
    if (m_scoreboardType == EScoreboardType_Award && !GetScoreboard()->HasStats())
        m_scoreboardType = EScoreboardType_Scoreboard;
    if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
        m_scoreboardLayout = new MG02ScoreboardLayout();
    else if (Minecraft::InMiniGame(eMiniGameId_Glide, true))
        m_scoreboardLayout = new MG03ScoreboardLayout(m_scoreboardType);
    else if (Minecraft::InMiniGame(eMiniGameId_Build, true))
        m_scoreboardLayout = new MG04ScoreboardLayout();
    else
        m_scoreboardLayout = new MG01ScoreboardLayout(m_scoreboardType);
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
    return GetPlayer()->GetLevel()->GetClientScoreboard();
}

void UIScene_GameModeScoreboardMenu::registerTexture(int index)
{
    if (index >= 119 || m_registeredTextures[index] != 0)
        return;
    wchar_t path[64];
    swprintf(path, 64, L"Graphics\\GameModeGraphics\\%ls.png", IScoreboardLayout::getIcon(index));
    if (g_pApp->hasArchiveFile(std::wstring(path), false))
    {
        arrayWithLength<unsigned char> data = g_pApp->getArchiveFile(std::wstring(path), false);
        if (data.length != 0 && data.data != 0)
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
        m_stat1Label.setLabel(UIString(medal == 0 ? IDS_MG_NO_MEDAL : medal->GetName()), false, false);
        m_stat2Label.setLabel(UIString(m_scoreboardLayout->getLabel2(uid)), false, false);
        m_stat3Label.setLabel(UIString(m_scoreboardLayout->getLabel3(uid)), false, false);
        m_stat4Label.setLabel(UIString(m_scoreboardLayout->getLabel4(uid)), false, false);
        m_stat5Label.setLabel(UIString(m_scoreboardLayout->getLabel5(uid)), false, false);
        m_killedByLabel.setLabel(UIString(m_scoreboardLayout->getLabel6(uid)), false, false);
        m_gamertagLabel.setLabel(UIString(m_scoreboardLayout->getLabel7(uid)), false, false);
        m_statsPending = true;
        m_selectedUid = uid;
    }
    else
    {
        m_playerTitleLabel.init(UIString(IDS_MG_PLAYERS_TITLE));
        m_stat1Label.setLabel(UIString(L"%"), false, false);
        m_stat2Label.setLabel(UIString(L"%"), false, false);
        m_stat3Label.setLabel(UIString(L"%"), false, false);
        m_stat4Label.setLabel(UIString(L"%"), false, false);
        m_stat5Label.setLabel(UIString(L"%"), false, false);
        m_killedByLabel.setLabel(UIString(IDS_MG_KILLED_BY), false, false);
        m_gamertagLabel.setLabel(UIString(L"%"), false, false);
    }
}

void UIScene_GameModeScoreboardMenu::SetStatIconTextureName(int index, const std::wstring& name)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode());
    document->SetStatIconTextureName(index, name);
}

void UIScene_GameModeScoreboardMenu::SetTitleIconTextureName(int index, const std::wstring& name)
{
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode());
    document->SetTitleIconTextureName(index, name);
}

void UIScene_GameModeScoreboardMenu::InitialiseScoreboard(INetworkPlayer* player)
{
    StatsUID uid = StatsUID::NETWORKPLAYER_SUID(player);
    m_scoreboardLayout->Init(m_playerId);
    if (m_scoreboardType == EScoreboardType_Scoreboard)
        m_playerTitleLabel.init(UIString(IDS_PLAYERS_INVITE));
    else
    {
        m_playerTitleLabel.init(UIString(IDS_PLAYERS));
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
    SetStatIconTextureName(1, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon2(m_scoreboardType))));
    SetStatIconTextureName(2, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon3(m_scoreboardType))));
    SetStatIconTextureName(3, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon4(m_scoreboardType))));
    SetStatIconTextureName(4, std::wstring(IScoreboardLayout::getIcon(m_scoreboardLayout->getStatIcon5(m_scoreboardType))));
    for (int column = 0; column < m_columnCount; ++column)
        SetTitleIconTextureName(column, std::wstring(IScoreboardLayout::getIcon(static_cast<int>(m_scoreboardLayout->getTitleIcon(column, m_scoreboardType)))));

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
        if (m_scoreboardType == EScoreboardType_Scoreboard)
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
            slots[slot] = m_scoreboardLayout->GetRowLabel(slot, playerUid, m_scoreboardType);
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
    FJ_GameModeScoreboard* document = static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode());
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
    if (m_statsPending)
    {
        ClientScoreboard* scoreboard = GetScoreboard();
        GameMedal* medal;
        if (!m_scoreboardLayout->showLifeSpanProgress())
        {
            int progress = 0;
            std::wstring details = m_scoreboardLayout->GetCustomProgressDetails(m_selectedUid, progress);
            wchar_t format[64];
            swprintf(format, 64, L"<font color=\"#%08x\"><p align=center>",
                     g_pApp->GetHTMLColour(eMinecraftColour_HTMLColor_White));
            m_killBar.setLabel(UIString(std::wstring(format) + details + L"</p>"), false, false);
            m_killBar.setProgress(progress);
            medal = scoreboard->GetMedal(m_selectedUid);
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
                totalLifeSpan += scoreboard->GetLifeSpan(m_selectedUid, round);
                if (scoreboard->GetSurvived(m_selectedUid, round))
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
            swprintf(format, 64, L"<font color=\"#%08x\"><p align=center>",
                     g_pApp->GetHTMLColour(eMinecraftColour_HTMLColor_White));
            m_killBar.setLabel(UIString(std::wstring(format) + details + L"</p>"), false, false);
            m_killBar.setProgress(progress);
            if (!miniGame->HasMultipleRounds() && miniGame->GetLivesPerRound() == 1)
            {
                std::wstring killer = scoreboard->GetKiller(m_selectedUid, -1, -1);
                SetHasPlayerBeenKilled(!scoreboard->GetSurvived(m_selectedUid, -1) && !killer.empty());
            }
            else
            {
                SetHasPlayerBeenKilled(roundsSurvived != rounds);
            }
            medal = scoreboard->GetMedal(m_selectedUid);
        }
        int icon = medal == 0 || medal->GetId() < 0 ? 0x6e : medal->GetId() + 0xc;
        SetStatIconTextureName(0, std::wstring(IScoreboardLayout::getIcon(icon)));
        m_statsPending = false;
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
        partyTooltip = static_cast<int>(IDS_TOOLTIPS_INVITE_PARTY);

    int selected = static_cast<int>(m_playerList.getCurrentSelection());
    if (selected < 0 || selected >= m_scoreboardLayout->GetPlayerCount() ||
        !m_scoreboardLayout->doesExist(static_cast<unsigned int>(selected)))
    {
        gConsoleUIController.SetTooltips(m_playerId, -1, IDS_TOOLTIPS_BACK, partyTooltip,
                                         -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
                                         -1, -1, -1, false, false);
        return;
    }

    unsigned int index = static_cast<unsigned int>(selected);
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
        m_scoreboardLayout->getSmallIDOfPlayerAt(index));
    int kickTooltip = CanKickPlayer(player) ? static_cast<int>(IDS_TOOLTIPS_KICK) : -1;
    int playerTooltip = IDS_TOOLTIPS_VIEW_GAMERCARD;
    if (player != 0 && player->IsLocal() && player->GetUserIndex() == m_playerId)
        playerTooltip = static_cast<int>(IDS_TOOLTIPS_VIEW_GAMERPROFILE);
    gConsoleUIController.SetTooltips(m_playerId, kickTooltip, IDS_TOOLTIPS_BACK, partyTooltip,
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
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mLocalPlayers[m_playerId] == NULL || minecraft->mGameModes[m_playerId] == NULL)
        return;
    wchar_t slotName[64];
    mbstowcs(slotName, name, 64);
    boost::shared_ptr<ItemInstance> item =
        m_scoreboardLayout->getItemForSlot(slotName, m_playerList.getCurrentSelection(), m_scoreboardType);
    if (!item->isEmpty())
        customDrawSlotControlFui(rect, m_playerId, item, 1.0f, item->getItem()->isFoil(item), false, false);
}

void UIScene_GameModeScoreboardMenu::handleInput(int playerId, int action, bool repeat, bool pressed, bool released, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_playerId, action, repeat, pressed, released);
    switch (action)
    {
    case EControllerActions_MenuX:
        if (pressed && !repeat)
            CGameNetworkManager::sInstance.IsLocalGame();
        break;
    case EControllerActions_MenuY:
        if (pressed && m_playerList.hasFocus() && m_playerList.getItemCount() > 0)
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
        }
        break;
    case EControllerActions_MenuUp:
    case EControllerActions_MenuDown:
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
    }
}

void UIScene_GameModeScoreboardMenu::handleDestroy()
{
    CGameNetworkManager::sInstance.UnRegisterPlayerChangedCallback(
        m_playerId, IUIScene_PlayerList::OnPlayerChanged, this);
    m_layer->showComponent(m_playerId, EUIScene_MenuBackground, false);
}

void UIScene_GameModeScoreboardMenu::setScoreboardType(int type)
{
    static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode())->SetScoreboardType(type);
}

void UIScene_GameModeScoreboardMenu::initNumColumns(int columns)
{
    static_cast<FJ_GameModeScoreboard*>(m_movie->getRootNode()->getFuiNode())->InitNumColumns(columns);
}

void UIScene_GameModeScoreboardMenu::handleReload()
{
    for (int index = 0; index < 119; ++index)
        m_registeredTextures[index] = 0;
    m_scoreboardLayout->ClearPlayersInfo();
    m_playerList.init(0);
    setScoreboardType(m_scoreboardType);
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

void UIScene_GameModeScoreboardMenu::handlePress(int controlId, int childId)
{
    if (controlId != 0 || (!m_canKickPlayers && !m_canVoteToKick) || childId < 0 ||
        childId >= m_scoreboardLayout->GetPlayerCount())
        return;
    unsigned int index = static_cast<unsigned int>(childId);
    if (!m_scoreboardLayout->doesExist(index))
        return;
    INetworkPlayer* player = CGameNetworkManager::sInstance.GetPlayerBySmallId(
        m_scoreboardLayout->getSmallIDOfPlayerAt(index));
    if (!CanKickPlayer(player))
        return;
    unsigned char* smallId = new unsigned char(m_scoreboardLayout->getSmallIDOfPlayerAt(index));
    int parameters[2] = { static_cast<int>(IDS_CONFIRM_OK), static_cast<int>(IDS_CONFIRM_CANCEL) };
    int message = Minecraft::InMiniGame(eMiniGameId_Any, true)
                      ? static_cast<int>(0x2cf43e54)
                      : static_cast<int>(IDS_UNLOCK_KICK_PLAYER);
    gConsoleUIController.RequestAlertMessage(
        IDS_UNLOCK_KICK_PLAYER_TITLE, message, parameters, 2, m_playerId,
        (int (*)(void*, int, C4JStorage::EMessageResult))IUIScene_PlayerList::KickPlayerReturned,
        smallId, 0, true);
}

void UIScene_GameModeScoreboardMenu::handleFocusChange(int focus, int childFocus)
{
    if (focus == 0)
        m_playerList.updateChildFocus(childFocus);
    updateTooltips();
    if (m_scoreboardType == EScoreboardType_Award)
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
        if (m_scoreboardType == EScoreboardType_Scoreboard)
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
                        slot, uid, m_scoreboardType);
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
                    slot, uid, m_scoreboardType);
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
    return EUIScene_GameModeScoreboardMenu;
}

int UIScene_GameModeScoreboardMenu::GetPad()
{
    return m_playerId;
}
