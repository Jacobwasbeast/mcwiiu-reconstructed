#include "platform/wiiu/CPlatformNetworkManagerNintendo.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "game/core/Socket.hpp"
#include <algorithm>

namespace
{
void clearPlayerFlags(std::vector<CPlatformNetworkManagerNintendo::PlayerFlags*>& flags)
{
    std::vector<CPlatformNetworkManagerNintendo::PlayerFlags*>::iterator it;
    for (it = flags.begin(); it != flags.end(); ++it)
        delete *it;
    flags.clear();
}
}

CPlatformNetworkManagerNintendo::PlayerFlags::PlayerFlags(INetworkPlayer* player, unsigned int flags)
    : m_player(player), m_flags(flags), m_reserved(0)
{
}

CPlatformNetworkManagerNintendo::PlayerFlags::~PlayerFlags()
{
}

CPlatformNetworkManagerNintendo::CPlatformNetworkManagerNintendo()
    : m_networkManager(0), m_hostPlayer(0), m_players(), m_sessionData(), m_playerFlags(), m_friendSessions(), m_localPlayers(),
      m_gameNetworkManager(0), m_localGame(false), m_privateGame(false), m_leavingGame(false), m_adhocMode(false), m_lanMode(false),
      m_exitAndJoin(false), m_addingPlayer(false), m_maxChunkPackets(0), m_playerChangedCallback(0), m_playerChangedCallbackData(0),
      m_sessionsUpdatedCallback(0), m_sessionsUpdatedCallbackData(0)
{
}

CPlatformNetworkManagerNintendo::~CPlatformNetworkManagerNintendo()
{
    clearPlayerFlags(m_playerFlags);
}

INetworkPlayer* CPlatformNetworkManagerNintendo::getNetworkPlayer(NQRNetworkPlayer* player)
{
    if (player == 0)
        return 0;
    return (INetworkPlayer*)player->GetCustomDataValue();
}

unsigned int CPlatformNetworkManagerNintendo::addNetworkPlayer(NQRNetworkPlayer* player)
{
    if (player == 0)
        return 0;
    m_players.push_back(player);
    return 1;
}

void CPlatformNetworkManagerNintendo::HandleDataReceived(NQRNetworkPlayer* from, NQRNetworkPlayer* to, unsigned char* data, unsigned int size)
{
    if (from == 0 || to == 0 || data == 0)
        return;
    INetworkPlayer* player = to->IsHost() ? getNetworkPlayer(from) : getNetworkPlayer(to);
    if (player != 0 && player->GetSocket() != 0)
        player->GetSocket()->pushDataToQueue(data, size, !to->IsHost());
}

void CPlatformNetworkManagerNintendo::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (player != 0)
    {
        m_players.push_back(player);
        INetworkPlayer* networkPlayer = getNetworkPlayer(player);
        if (networkPlayer != 0)
            m_localPlayers.push_back(networkPlayer);
    }
}

void CPlatformNetworkManagerNintendo::HandlePlayerLeaving(NQRNetworkPlayer* player)
{
    if (player == 0)
        return;
    std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin();
    while (it != m_players.end())
    {
        if (*it == player)
        {
            m_players.erase(it);
            break;
        }
        ++it;
    }
    removeNetworkPlayer(player);
}

bool CPlatformNetworkManagerNintendo::isSystemPrimaryPlayer(NQRNetworkPlayer* player)
{
    return player != 0 && player == m_hostPlayer;
}

void CPlatformNetworkManagerNintendo::HandleStateChange(NQRNetworkManager::eNQRNetworkManagerState, NQRNetworkManager::eNQRNetworkManagerState newState, bool)
{
    m_leavingGame = newState == NQRNetworkManager::eNQRNetworkManagerState_Leaving;
}

void CPlatformNetworkManagerNintendo::HandleResyncPlayerRequest(NQRNetworkPlayer** players)
{
    if (players != 0)
        *players = m_hostPlayer;
}

void CPlatformNetworkManagerNintendo::HandleAddLocalPlayerFailed(int)
{
    m_addingPlayer = false;
}

void CPlatformNetworkManagerNintendo::HandleDisconnect(bool error)
{
    m_leavingGame = error;
}

void CPlatformNetworkManagerNintendo::HandleInviteReceived(int, nn::fp::FriendPresence*)
{
}

void CPlatformNetworkManagerNintendo::UpdateMatchMakingData()
{
    m_sessionData.m_numPlayers = (unsigned char)m_players.size();
}

void CPlatformNetworkManagerNintendo::Initialise(CGameNetworkManager* manager, int maxChunkPackets)
{
    m_gameNetworkManager = manager;
    m_maxChunkPackets = maxChunkPackets;
    m_leavingGame = false;
}

void CPlatformNetworkManagerNintendo::Terminate()
{
    m_players.clear();
    m_localPlayers.clear();
    clearPlayerFlags(m_playerFlags);
    m_hostPlayer = 0;
    m_networkManager = 0;
}

int CPlatformNetworkManagerNintendo::GetJoiningReadyPercentage()
{
    return m_players.empty() ? 0 : 100;
}

int CPlatformNetworkManagerNintendo::CorrectErrorIDS(int errorIds)
{
    return errorIds;
}

void CPlatformNetworkManagerNintendo::TickSearch()
{
    if (m_networkManager != 0 && m_networkManager->FriendRoomManagerIsBusy())
        m_networkManager->FriendRoomManagerSearch();
}

void CPlatformNetworkManagerNintendo::DoWork()
{
    if (m_networkManager != 0)
        m_networkManager->Tick();
}

int CPlatformNetworkManagerNintendo::GetPlayerCount() { return (int)m_players.size(); }
int CPlatformNetworkManagerNintendo::GetOnlinePlayerCount() { return (int)m_players.size(); }
int CPlatformNetworkManagerNintendo::GetLocalPlayerMask(int userIndex) { return userIndex >= 0 && userIndex < 32 && GetLocalPlayerByUserIndex(userIndex) != 0 ? 1 << userIndex : 0; }
bool CPlatformNetworkManagerNintendo::AddLocalPlayerByUserIndex(int userIndex)
{
    if (m_networkManager == 0)
        return false;
    m_addingPlayer = true;
    return m_networkManager->AddLocalPlayerByUserIndex(userIndex);
}

bool CPlatformNetworkManagerNintendo::RemoveLocalPlayerByUserIndex(int userIndex)
{
    INetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
    return player != 0 && RemoveLocalPlayer(player);
}

bool CPlatformNetworkManagerNintendo::AddLocalPlayerByPlayerUID(PlayerUID uid)
{
    return GetPlayerByXuid(uid) != 0;
}

INetworkPlayer* CPlatformNetworkManagerNintendo::GetLocalPlayerByUserIndex(int userIndex)
{
    std::vector<INetworkPlayer*>::iterator it;
    for (it = m_localPlayers.begin(); it != m_localPlayers.end(); ++it)
        if (*it != 0 && (*it)->GetUserIndex() == userIndex)
            return *it;
    return 0;
}

INetworkPlayer* CPlatformNetworkManagerNintendo::GetPlayerByIndex(int index)
{
    return index >= 0 && index < (int)m_players.size() ? getNetworkPlayer(m_players[index]) : 0;
}

INetworkPlayer* CPlatformNetworkManagerNintendo::GetPlayerByXuid(PlayerUID uid)
{
    int i;
    for (i = 0; i < (int)m_players.size(); ++i)
        if (GetPlayerByIndex(i) != 0 && GetPlayerByIndex(i)->GetUID() == uid)
            return GetPlayerByIndex(i);
    return 0;
}

INetworkPlayer* CPlatformNetworkManagerNintendo::GetPlayerBySmallId(unsigned char smallId)
{
    int i;
    for (i = 0; i < (int)m_players.size(); ++i)
        if (GetPlayerByIndex(i) != 0 && GetPlayerByIndex(i)->GetSmallId() == smallId)
            return GetPlayerByIndex(i);
    return 0;
}

std::wstring CPlatformNetworkManagerNintendo::GetDisplayNameByGamertag(std::wstring gamertag) { return gamertag; }
bool CPlatformNetworkManagerNintendo::ShouldMessageForFullSession() { return true; }
INetworkPlayer* CPlatformNetworkManagerNintendo::GetHostPlayer() { return getNetworkPlayer(m_hostPlayer); }
bool CPlatformNetworkManagerNintendo::IsHost() { return m_hostPlayer != 0; }
bool CPlatformNetworkManagerNintendo::JoinGameFromInviteInfo(int, int, const nn::fp::FriendPresence*) { return false; }
bool CPlatformNetworkManagerNintendo::LeaveGame(bool error) { return _LeaveGame(error, true); }
bool CPlatformNetworkManagerNintendo::IsInSession() { return !m_players.empty(); }
bool CPlatformNetworkManagerNintendo::IsInGameplay() { return m_sessionData.m_bGameStarted; }
bool CPlatformNetworkManagerNintendo::IsReadyToPlayOrIdle() { return !m_leavingGame; }
bool CPlatformNetworkManagerNintendo::IsInStatsEnabledSession() { return true; }
bool CPlatformNetworkManagerNintendo::IsMatchmakingEnabled() { return m_networkManager != 0 && m_networkManager->IsMatchmakingEnabled(); }
bool CPlatformNetworkManagerNintendo::IsHandlingBootInvite() { return false; }
bool CPlatformNetworkManagerNintendo::SessionHasSpace(unsigned int extraPlayers) { return m_players.size() + extraPlayers <= GameSessionData::MAX_SESSION_PLAYERS; }
bool CPlatformNetworkManagerNintendo::SessionHasMii(int userIndex) { return GetLocalPlayerByUserIndex(userIndex) != 0; }
void CPlatformNetworkManagerNintendo::SendInviteGUI(int) { if (m_networkManager != 0) m_networkManager->SendInviteGUI(); }
bool CPlatformNetworkManagerNintendo::IsAddingPlayer() { return m_addingPlayer; }
void CPlatformNetworkManagerNintendo::SystemFlagReset() { clearPlayerFlags(m_playerFlags); }
void CPlatformNetworkManagerNintendo::HostGame(int mask, bool, bool privateGame, unsigned char mode, EMiniGameId miniGameId, unsigned int worldId, unsigned int* attributes)
{
    m_privateGame = privateGame;
    m_localGame = false;
    _HostGame(mask, mode, miniGameId, worldId, attributes);
}
int CPlatformNetworkManagerNintendo::JoinGame(FriendSessionInfo*, int, int, bool) { return 0; }
bool CPlatformNetworkManagerNintendo::FindAndJoinGame(int, int, EMiniGameId, unsigned int, unsigned int*, unsigned int*) { return false; }
bool CPlatformNetworkManagerNintendo::SetLocalGame(bool localGame) { m_localGame = localGame; return true; }
bool CPlatformNetworkManagerNintendo::IsLocalGame() { return m_localGame; }
void CPlatformNetworkManagerNintendo::SetPrivateGame(bool privateGame) { m_privateGame = privateGame; }
bool CPlatformNetworkManagerNintendo::IsPrivateGame() { return m_privateGame; }
bool CPlatformNetworkManagerNintendo::IsLeavingGame() { return m_leavingGame; }
void CPlatformNetworkManagerNintendo::ResetLeavingGame() { m_leavingGame = false; }
void CPlatformNetworkManagerNintendo::RegisterPlayerChangedCallback(int, void (*callback)(void*, INetworkPlayer*, bool), void* data) { m_playerChangedCallback = callback; m_playerChangedCallbackData = data; }
void CPlatformNetworkManagerNintendo::UnRegisterPlayerChangedCallback(int, void (*callback)(void*, INetworkPlayer*, bool), void* data) { if (m_playerChangedCallback == callback && m_playerChangedCallbackData == data) { m_playerChangedCallback = 0; m_playerChangedCallbackData = 0; } }
void CPlatformNetworkManagerNintendo::HandleSignInChange() { }
bool CPlatformNetworkManagerNintendo::_RunNetworkGame() { DoWork(); return true; }
bool CPlatformNetworkManagerNintendo::_LeaveGame(bool error, bool) { m_leavingGame = true; if (m_networkManager != 0) m_networkManager->LeaveRoom(error); return true; }
void CPlatformNetworkManagerNintendo::_HostGame(int mask, unsigned char mode, EMiniGameId, unsigned int worldId, unsigned int* attributes)
{
    if (m_networkManager != 0) m_networkManager->CreateAndJoinRoom(mask, GameSessionData::MAX_SESSION_PLAYERS, &m_sessionData, sizeof(m_sessionData), (int*)attributes, 0, false, mode);
    m_sessionData.m_numPlayers = 0;
    m_sessionData.m_gameHostOptions = worldId;
}
bool CPlatformNetworkManagerNintendo::_StartGame() { m_sessionData.m_bGameStarted = true; if (m_networkManager != 0) m_networkManager->StartGame(); return true; }
void CPlatformNetworkManagerNintendo::UpdateAndSetGameSessionData(INetworkPlayer* player) { if (player != 0) m_sessionData.m_numPlayers = (unsigned char)m_players.size(); }
bool CPlatformNetworkManagerNintendo::RemoveLocalPlayer(INetworkPlayer* player)
{
    std::vector<INetworkPlayer*>::iterator it = std::find(m_localPlayers.begin(), m_localPlayers.end(), player);
    if (it == m_localPlayers.end()) return false;
    m_localPlayers.erase(it); SystemFlagRemovePlayer(player); return true;
}
void CPlatformNetworkManagerNintendo::SystemFlagAddPlayer(INetworkPlayer* player) { if (player != 0) m_playerFlags.push_back(new PlayerFlags(player, 0)); }
void CPlatformNetworkManagerNintendo::SystemFlagSet(INetworkPlayer* player, int flag) { std::vector<PlayerFlags*>::iterator it; for (it = m_playerFlags.begin(); it != m_playerFlags.end(); ++it) if ((*it)->m_player == player) (*it)->m_flags |= 1u << flag; }
bool CPlatformNetworkManagerNintendo::SystemFlagGet(INetworkPlayer* player, int flag) { std::vector<PlayerFlags*>::iterator it; for (it = m_playerFlags.begin(); it != m_playerFlags.end(); ++it) if ((*it)->m_player == player) return ((*it)->m_flags & (1u << flag)) != 0; return false; }
void CPlatformNetworkManagerNintendo::SystemFlagClearAll(INetworkPlayer* player) { std::vector<PlayerFlags*>::iterator it; for (it = m_playerFlags.begin(); it != m_playerFlags.end(); ++it) if ((*it)->m_player == player) (*it)->m_flags = 0; }
std::wstring CPlatformNetworkManagerNintendo::GatherStats() { return std::wstring(); }
std::wstring CPlatformNetworkManagerNintendo::GatherRTTStats() { return std::wstring(); }
void CPlatformNetworkManagerNintendo::SetSessionTexturePackParentId(int id) { m_sessionData.m_texturePackParentId = id; }
void CPlatformNetworkManagerNintendo::SetSessionSubTexturePackId(int id) { m_sessionData.m_subTexturePackId = (unsigned char)id; }
void CPlatformNetworkManagerNintendo::SetSessionGameModeId(int id) { m_sessionData.m_gameModeId = id; }
void CPlatformNetworkManagerNintendo::Notify(int, unsigned long) { }
int CPlatformNetworkManagerNintendo::GetSessionList(int, int, bool) { return 0; }
bool CPlatformNetworkManagerNintendo::GetGameSessionInfo(int, unsigned int, FriendSessionInfo*) { return false; }
void CPlatformNetworkManagerNintendo::SetSessionsUpdatedCallback(void (*callback)(void*), void* data) { m_sessionsUpdatedCallback = callback; m_sessionsUpdatedCallbackData = data; }
void CPlatformNetworkManagerNintendo::GetFullFriendSessionInfo(FriendSessionInfo*, void (*)(bool, void*), void*) { }
void CPlatformNetworkManagerNintendo::ForceFriendsSessionRefresh() { }
void CPlatformNetworkManagerNintendo::ClearFriendsSessions() { m_friendSessions.clear(); }
bool CPlatformNetworkManagerNintendo::AlreadyInSession(const nn::fp::FriendPresence*) { return IsInSession(); }
bool CPlatformNetworkManagerNintendo::GetAgeRestricted() { return false; }
bool CPlatformNetworkManagerNintendo::usingAdhocMode() { return m_adhocMode; }
bool CPlatformNetworkManagerNintendo::setAdhocMode(bool mode) { m_adhocMode = mode; return true; }
void CPlatformNetworkManagerNintendo::startAdhocMatching() { }
bool CPlatformNetworkManagerNintendo::isAdhocConnected() { return m_adhocMode; }
bool CPlatformNetworkManagerNintendo::connectToAdhoc() { m_adhocMode = true; return true; }
bool CPlatformNetworkManagerNintendo::isAdhocModeChangeInProgress() { return false; }
void CPlatformNetworkManagerNintendo::AttemptAdhocSignIn(int (*)(void*, bool, int), void*, bool) { }
bool CPlatformNetworkManagerNintendo::setLanMode(bool mode) { m_lanMode = mode; return true; }
bool CPlatformNetworkManagerNintendo::usingLanMode() { return m_lanMode; }
bool CPlatformNetworkManagerNintendo::isLanConnected() { return m_lanMode; }
bool CPlatformNetworkManagerNintendo::isLanModeChangeInProgress() { return false; }
bool CPlatformNetworkManagerNintendo::checkValidInviteData(const nn::fp::FriendPresence* presence) { return presence != 0; }
void CPlatformNetworkManagerNintendo::SetExitAndJoinFromInviteInProgress(bool inProgress) { m_exitAndJoin = inProgress; }
void CPlatformNetworkManagerNintendo::CancelDisconnectFromExitAndJoin() { m_exitAndJoin = false; }
int CPlatformNetworkManagerNintendo::GetOutstandingAckCount(INetworkPlayer* player) { return player == 0 ? 0 : player->GetOutstandingAckCount(); }
void CPlatformNetworkManagerNintendo::TryToConnectOnline(int (*)(void*, bool, int), void*, bool) { }
void CPlatformNetworkManagerNintendo::AttemptPSNSignIn(int (*)(void*, bool, int), void*, bool, int) { }
void CPlatformNetworkManagerNintendo::RecvInviteGUI() { }
bool CPlatformNetworkManagerNintendo::UpdateInviteData(void*) { return false; }
void CPlatformNetworkManagerNintendo::CheckFriendsCanPlay(int, int& friendCount, boost::shared_ptr<ClientboundPreLoginPacket>, int, int& canPlayCount, int& cannotPlayCount, int& blockedCount, int& unknownCount)
{
    friendCount = 0; canPlayCount = 0; cannotPlayCount = 0; blockedCount = 0; unknownCount = 0;
}

void CPlatformNetworkManagerNintendo::RemovePlayerOnSocketClosedThreadProc(void* data)
{
    NQRNetworkPlayer* player = (NQRNetworkPlayer*)data;
    if (player != 0)
        player->ConnectionComplete();
}

void CPlatformNetworkManagerNintendo::removeNetworkPlayer(NQRNetworkPlayer* player)
{
    INetworkPlayer* networkPlayer = getNetworkPlayer(player);
    std::vector<INetworkPlayer*>::iterator it = std::find(m_localPlayers.begin(), m_localPlayers.end(), networkPlayer);
    if (it != m_localPlayers.end()) m_localPlayers.erase(it);
}

void CPlatformNetworkManagerNintendo::SystemFlagRemovePlayer(INetworkPlayer* player)
{
    std::vector<PlayerFlags*>::iterator it = m_playerFlags.begin();
    while (it != m_playerFlags.end())
    {
        if ((*it)->m_player == player) { delete *it; it = m_playerFlags.erase(it); }
        else ++it;
    }
}
