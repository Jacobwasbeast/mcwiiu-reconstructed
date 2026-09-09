#ifndef GAME_NETWORK_CGAMENETWORKMANAGER_HPP
#define GAME_NETWORK_CGAMENETWORKMANAGER_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>
#include "game/minigame/EMiniGameId.hpp"
#include "game/network/CPlatformNetworkManager.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/generation/PlayerUID.hpp"
#include "game/core/CriticalSection.hpp"

class ClientPacketListener;
class ClientboundPreLoginPacket;
class C4JEventImpl;
class FriendSessionInfo;
class GameSessionData;
class Minecraft;

class CGameNetworkManager
{
public:
    CGameNetworkManager();
    ~CGameNetworkManager();

    static CGameNetworkManager sInstance;

    INetworkPlayer* GetLocalPlayerByUserIndex(int userIndex);
    void ServerStoppedCreate(bool create);
    void ServerReadyCreate(bool create);
    void Initialise();
    void Terminate();
    static int GetLocalPlayerMask(int userIndex);
    bool IsInSession();
    bool JoinGameFromInviteInfo(int userIndex, int userIndexMask, const FriendPresence* presence);
    bool SetLocalGame(bool localGame);
    static int JoinFromInvite_SignInReturned(void* data, bool result, int error);
    bool HandleInviteWhenInMenus(int userIndex, const FriendPresence* presence);
    void ProcessInvite(unsigned int userIndex, unsigned int flags, FriendPresence* presence);
    void GameInviteReceived(int userIndex, FriendPresence* presence);
    bool TickInvitePending();
    void DoWork();
    bool IsHost();
    static void* ServerThreadProc(void* data);
    void ServerReadyWait();
    void ServerReadyDestroy();
    bool IsReadyToPlayOrIdle();
    int CorrectErrorIDS(int errorIds);
    int GetJoiningReadyPercentage();
    bool IsLeavingGame();
    int GetPlayerCount();
    bool IsLocalGame();
    bool StartNetworkGame(Minecraft* minecraft, void* data);
    bool _RunNetworkGame(void* data);
    int GetOnlinePlayerCount();
    bool AddLocalPlayerByUserIndex(int userIndex);
    bool RemoveLocalPlayerByUserIndex(int userIndex);
    INetworkPlayer* GetPlayerByIndex(int index);
    INetworkPlayer* GetPlayerByXuid(PlayerUID uid);
    INetworkPlayer* GetPlayerBySmallId(unsigned char smallId);
    std::wstring GetDisplayNameByGamertag(std::wstring gamertag);
    INetworkPlayer* GetHostPlayer();
    void RegisterPlayerChangedCallback(int userIndex, void (*callback)(void*, INetworkPlayer*, bool), void* data);
    void UnRegisterPlayerChangedCallback(int userIndex, void (*callback)(void*, INetworkPlayer*, bool), void* data);
    void HandleSignInChange();
    bool ShouldMessageForFullSession();
    bool IsInGameplay();
    void SetPrivateGame(bool privateGame);
    bool IsPrivateGame();
    void HostGame(int userIndexMask, bool onlineGame, bool privateGame, unsigned char gameMode, EMiniGameId miniGameId, unsigned int worldId, unsigned int* attributes);
    bool IsInStatsEnabledSession();
    bool IsMatchmakingEnabled();
    bool IsInPublicJoinableGame();
    void SetIsInPublicJoinableGame(bool joinable);
    bool IsInActiveCoopOrCompetitive(int userIndex);
    bool SessionHasSpace(unsigned int extraPlayers);
    bool SessionHasMii(int userIndex);
    int GetSessionList(int userIndex, int maxSessions, bool friendsOnly);
    bool GetGameSessionInfo(int userIndex, unsigned int sessionId, FriendSessionInfo* info);
    void SetSessionsUpdatedCallback(void (*callback)(void*), void* data);
    void GetFullFriendSessionInfo(FriendSessionInfo* info, void (*callback)(bool, void*), void* data);
    void ForceFriendsSessionRefresh();
    void ClearFriendsSessions();
    int JoinGame(FriendSessionInfo* info, int userIndex, bool matchmaking);
    bool FindAndJoinGame(int userIndex, EMiniGameId miniGameId, unsigned int attributeCount, unsigned int* attributes, unsigned int* ids);
    static void CancelJoinGame(void* data);
    bool LeaveGame(bool error);
    void UpdateAndSetGameSessionData(INetworkPlayer* player);
    void SendInviteGUI(int userIndex);
    void ResetLeavingGame();
    bool AlreadyInSession(const FriendPresence* presence);
    bool IsNetworkThreadRunning();
    static void* RunNetworkGameThreadProc(void* data);
    static void* ExitAndJoinFromInviteThreadProc(void* data);
    int TryToConnectOnline(int (*callback)(void*, bool, int), void* data, bool showProgress);
    static bool _LeaveGame();
    static void* ChangeSessionTypeThreadProc(void* data);
    void SystemFlagSet(INetworkPlayer* player, int flag);
    bool SystemFlagGet(INetworkPlayer* player, int flag);
    void SystemFlagClearAll(INetworkPlayer* player);
    void ServerReady();
    void ServerStopped();
    void ServerStoppedWait();
    void ServerStoppedDestroy();
    bool ServerStoppedValid();
    void RemoveCreatedConnection(ClientPacketListener* connection);
    bool usingLanMode();
    bool setLanMode(bool lanMode);
    bool isLanConnected();
    bool isLanModeChangeInProgress();
    std::wstring GatherStats();
    void renderQueueMeter();
    std::wstring GatherRTTStats();
    void SetSessionGameModeId(int gameModeId);
    void StateChange_AnyToHosting();
    void StateChange_AnyToJoining(bool error);
    void StateChange_JoiningToIdle(CPlatformNetworkManager::eJoinFailedReason reason);
    void StateChange_AnyToStarting();
    void StateChange_AnyToEnding(bool error, bool endSession);
    Socket* CreateSocket(INetworkPlayer* player, bool local);
    void CloseConnection(INetworkPlayer* player);
    void PlayerJoining(INetworkPlayer* player);
    void PlayerLeaving(INetworkPlayer* player);
    void HostChanged();
    void WriteStats(INetworkPlayer* player);
    void AddLocalPlayerFailed(int userIndex, bool error);
    int GetPrimaryPad();
    int GetLockedProfile();
    void HandleDisconnect(bool error);
    bool IsSignedInLive(int userIndex);
    bool AllowedToPlayMultiplayer(int userIndex);
    void FakeLocalPlayerJoined();
    bool IsHandlingBootInvite();
    void WiiU_ClearIsHandlingBootInvite();
    void WiiU_ShowErrorViewer();
    void ShowConnectOnlineNintendoProgress(int userIndex, int (*callback)(void*, bool, int, int), void* data);
    void WiiU_ReleaseForeground();
    void WiiU_AcquiredForeground();
    void WiiU_ReleaseCallback();
    void WiiU_SetBootFromFriendList();
    bool IsUGCRestricted(int userIndex);
    bool ModeAllowsUGC(EMiniGameId miniGameId);
    bool IsUGCRestrictedMode(int userIndex, EMiniGameId miniGameId);
    bool IsFriendsOnlyUGC(int userIndex);
    void CheckFriendsCanPlay(int userIndex, int& friendCount, boost::shared_ptr<ClientboundPreLoginPacket> packet, int titleVersion, int& canPlayCount, int& cannotPlayCount, int& blockedCount, int& unknownCount);
    unsigned int GetMaxPlayers(EMiniGameId miniGameId, unsigned int worldId);
    unsigned int GetMaxPlayers(GameSessionData* data);
    unsigned int GetMaxPlayers();
    void AttemptPSNSignIn(int (*callback)(void*, bool, int), void* data, bool showProgress, int userIndex);
    void RecvInviteGUI();
    void ClearInviteInfo();
    void MoreThanOneActivePlayer();
    bool ServerReadyValid();
    void StateChange_AnyToIdle();
    std::wstring GetOnlineName(int userIndex);

private:
    C4JEventImpl* m_serverStoppedEvent;
    C4JEventImpl* m_serverReadyEvent;
    CPlatformNetworkManager* m_platform;
    int m_pendingInviteUserIndex;
    unsigned int m_pendingInviteUserMask;
    FriendPresence* m_pendingInvitePresence;
    std::vector<ClientPacketListener*> m_connections;
    OSFastMutex m_connectionsMutex;
    bool m_initialised;
    bool m_networkThreadRunning;
    bool m_disconnectError;
    bool m_unknownState;
    bool m_publicJoinable;
    void* m_inviteInfo;
};

#endif
