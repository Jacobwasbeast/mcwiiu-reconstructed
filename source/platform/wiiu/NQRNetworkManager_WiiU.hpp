#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_WIIU_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_WIIU_HPP

#include <deque>
#include <map>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"

class C4JEventImpl;
class C4JThreadImpl;

namespace nn
{
    namespace fp
    {
        struct FriendPresence;
    }
}

class NQRNetworkManager_WiiU : public NQRNetworkManager
{
public:
    enum ENEXAsyncProcess
    {
        eNEXAsyncProcess_None,
        eNEXAsyncProcess_Login,
        eNEXAsyncProcess_Logout,
        eNEXAsyncProcess_Finalize,
        eNEXAsyncProcess_Shutdown,
        eNEXAsyncProcess_Search,
        eNEXAsyncProcess_Update,
        eNEXAsyncProcess_CreateSession,
        eNEXAsyncProcess_JoinSession,
        eNEXAsyncProcess_LeaveSession,
        eNEXAsyncProcess_BrowseSession,
        eNEXAsyncProcess_UpdateSession,
        eNEXAsyncProcess_JoinRandomSession
    };

    enum EJoinFailedReason
    {
        eJoinFailedReason_None,
        eJoinFailedReason_Full,
        eJoinFailedReason_Version,
        eJoinFailedReason_Mii,
        eJoinFailedReason_Network
    };

    struct SendQueueItem
    {
        unsigned long long stationId;
        unsigned char channel;
        unsigned char* data;
        unsigned int size;

        SendQueueItem();
        SendQueueItem(unsigned long long station, unsigned char sendChannel, const void* sendData, unsigned int sendSize);
        SendQueueItem(const SendQueueItem& other);
        SendQueueItem& operator=(const SendQueueItem& other);
        ~SendQueueItem();
    };

    struct FriendSearchResult
    {
        unsigned int ownerPrincipalId;
        unsigned int ownerLocalFriendIndex;
        char ownerName[17];
        PresenceSyncInfo presence;
        unsigned int sessionId;
        bool joinable;
        unsigned char status[3];
        void* extData;

        FriendSearchResult();
    };

    struct ConnectedStation
    {
        unsigned long long stationId;
        int state;
        unsigned int playerMask;

        ConnectedStation();
        ConnectedStation(unsigned long long station);
    };

    class ClientJoinData
    {
    public:
        ClientJoinData();

        unsigned int playerMask;
        PlayerUID playerUID0;
        PlayerUID playerUID1;
        PlayerUID playerUID2;
        PlayerUID playerUID3;
        char onlineIds[4][21];
        wchar_t displayNames[4][11];
    };

    NQRNetworkManager_WiiU(INQRNetworkManagerListener* listener);
    virtual ~NQRNetworkManager_WiiU();

    virtual void Tick();
    virtual void Initialise();
    virtual void Terminate();
    virtual eNQRNetworkManagerState GetState();
    virtual bool IsHost();
    virtual bool IsReadyToPlayOrIdle();
    virtual bool IsInSession();
    virtual bool IsMatchmakingEnabled();
    virtual void CreateAndJoinRoom(int userIndex, int localUserMask, void* extData, int extDataSize, int* attributes, int attributeCount, bool localGame, unsigned char gameMode);
    virtual void UpdateExternalRoomData();
    virtual bool FriendRoomManagerIsBusy();
    virtual bool FriendRoomManagerSearch();
    virtual bool FriendRoomManagerSearch2();
    virtual int FriendRoomManagerGetCount();
    virtual void FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result);
    virtual bool JoinRoom(SessionSearchResult* result, int userIndexMask);
    virtual bool JoinRoom(unsigned int sessionId, int userIndexMask, const PresenceSyncInfo* presenceInfo);
    virtual bool JoinRoom(int userIndexMask, const nn::fp::FriendPresence* friendPresence);
    virtual bool FriendPresenceUpdatedRoomList();
    virtual SessionSearchResult* GetFriendRoomSearchResults(int* count);
    virtual void GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*), void* callbackData);
    virtual bool FindAndJoinRoom(int userIndexMask, unsigned int searchFlags, CMatchMakeSearchParam* searchParam, unsigned char gameMode);
    virtual void StartGame();
    virtual void LeaveRoom(bool error);
    virtual void EndGame();
    virtual bool SessionHasSpace(int extraPlayers);
    virtual bool SessionHasMii(int userIndex);
    virtual bool AddLocalPlayerByUserIndex(int userIndex);
    virtual bool RemoveLocalPlayerByUserIndex(int userIndex);
    virtual void SendInviteGUI();
    virtual int GetPlayerCount();
    virtual int GetOnlinePlayerCount();
    virtual NQRNetworkPlayer* GetPlayerByIndex(int index);
    virtual NQRNetworkPlayer* GetPlayerBySmallId(int smallId);
    virtual NQRNetworkPlayer* GetPlayerByXuid(PlayerUID uid);
    virtual std::wstring GetDisplayNameByGamertag(std::wstring gamertag);
    virtual NQRNetworkPlayer* GetLocalPlayerByUserIndex(int userIndex);
    virtual NQRNetworkPlayer* GetHostPlayer();
    virtual void SetPresenceDataStartHostingGame();
    virtual int GetJoiningReadyPercentage();
    virtual void LocalDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size);
    virtual void RemoteDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size);
    virtual int GetSessionIndex(NQRNetworkPlayer* player);
    virtual void TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData, bool showProgress);
    virtual bool isAdhocConnected();
    virtual bool connectToAdhoc();
    virtual bool isLanConnected();

    void SetNexAsyncProcess(ENEXAsyncProcess process);
    static int ConnectOnlineNintendo_Returned(void* context, bool connected, int error);
    static int ConnectOnlineNintendoThreadProc(void* context);
    void CancelConnectCallbacks();
    static void CancelConnectOnlineNintendo(void* context);
    static void ShowConnectOnlineNintendoProgress(int userIndex, int (*callback)(void*, bool, int, int), void* callbackData);
    static int InvitationSignInReturned(void* context, bool signedIn, int error, int userIndex);
    static int InvitationMustConnectReturned(void* context, int button, C4JStorage::EMessageResult result);
    void UpdateInvitationChecks();
    NQRNetworkPlayer* GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex);
    void RemoveConnectedStation(unsigned long long stationId);
    void RemoveConnectedPlayer(NQRNetworkPlayer* player);
    void NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId);
    void AddConnectedPlayer(NQRNetworkPlayer* player);
    void HandlePlayerJoined(NQRNetworkPlayer* player);
    NQRNetworkPlayer* FindOrCreateNonNetworkPlayer(int slotIndex, int type, PlayerSyncData* syncData);
    void NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId);
    void MapRoomSlotPlayers(int previousPlayerCount);
    void UpdateRoomSyncUIDsFromPlayers();
    void RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size);
    void SyncRoomData();
    void RemoveRemotePlayersAndSync(unsigned long long stationId, int localIndex);
    void ClearSendQueue();
    void SetState(eNQRNetworkManagerInternalState state);
    void TickSendQueue();
    void Pia_InitializeCommon(unsigned char* buffer, unsigned int size, unsigned long long stationId, bool host, bool online);
    void Pia_InitializeInet();
    void Pia_InitializeSession(const void* setting);
    ConnectedStation* GetConnectedStation(unsigned long long stationId);
    void AddConnectedStation(unsigned long long stationId);
    void Pia_SignalLeaveSession();
    static void Pia_SessionCallback(int eventType, unsigned long long stationId, void* context);
    bool NexFacadeLogin(unsigned int principalId);
    void InitialiseAfterOnline();
    void OnlineCheck();
    void FriendSearchTick();
    static void FriendPresenceLoginCallback(unsigned int result, void* context);
    void Session_CleanupSession();
    bool Inet_Logout();
    void Inet_FinalizeNetwork();
    void Session_FinalizePiaSession();
    void Inet_FinalizePiaInet();
    void Pia_FinalizeCommon();
    void Pia_Finalize();
    void Pia_Shutdown();
    void SetLastNetworkError(int result);
    void UpdateApplicationConnected();
    bool Pia_CreateSession();
    bool canRunRateControlledEvent();
    bool Pia_JoinSession();
    bool Pia_JoinMatchmakeSession();
    bool Pia_LeaveSession();
    void Pia_SignalBrowseSession();
    bool Pia_BrowseSession();
    bool Pia_JoinRandomSession();
    void UpdateExternalRoomDataFromThread();
    void UpdateMatchmakingAttributes();
    bool Pia_StartupSession();
    void Pia_SignalCreateSession();
    void Pia_SignalJoinSession();
    void SetLocalPlayersAndSync();
    static void ClearIsHandlingBootInvite();
    void SetNQRPresenceInfoFromExtData(PresenceSyncInfo* presenceInfo, void* extData);
    void UpdateRichPresenceCustomData(void* extData, unsigned int extDataSize);
    static void MatchmakingExtDataUpdatedCallback(bool success, void* context);
    void RemoveAllPlayersOnClient();
    void ResetToIdle();
    void TickNexAsyncProcess();
    void SendJoinData();
    void NetworkPlayerConnectionComplete(NQRNetworkPlayer* player);
    void PollUnconnectedStations();
    bool AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData, EJoinFailedReason* reason);
    void SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason);
    void PollReceiveData();
    static void ShowErrorViewer();
    void RemoveLocalClientPlayers(int userIndex);
    void ErrorHandlingTick();
    void TryToConnectOnline();
    void TickFromThread();
    void RunTickThread();
    static int TickThreadProc(void* context);
    void MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo);
    void UpdateFriendFromPresence(unsigned int friendIndex);
    static void UpdateFriendFromPresence_callback(unsigned int friendIndex, void* context);
    unsigned int GetFriendsFromPresence();
    static void GetFriendsThreadProc(void* context);
    void ClearRichPresenceCustomData();
    void UpdateRemotePlay();
    void SendLeaveData(unsigned int playerMask);
    static bool IsHandlingBootInvite();
    NQRNetworkPlayer* GetPlayerIfReady(NQRNetworkPlayer* player);
    static void SetAppSuspended(bool suspended);
    void SetApplicationDisconnected();
    void Pia_ShutdownForLan();

    void TickNexFacadeLogin();
    void UpdatePlayersFromRoomSyncUIDs();
    int GetOldMask(int mask);
    int GetAddedMask(int oldMask, int newMask);
    int GetRemovedMask(int oldMask, int newMask);
    void ForceErrorPoint(eNQRForceError error);
    static void RefreshChatAndContentRestrictionsReturned_HandleInvite(void* context);
    void ResetNEXAsyncContext();
    void NexFacadeLogout();
    void Pia_FinalizeInet();
    void Pia_SignalJoinMatchmakeSession();
    void Pia_SignalJoinRandomSession();
    static void Pia_CheckGetLocalAndHostStationId();
    void CheckApplicationStillConnected();
    const char* GetNexASyncProcessString(ENEXAsyncProcess process);

private:
    typedef std::map<std::wstring, std::wstring> PlayerNameMap;

    bool m_friendPresenceUpdated;
    C4JThreadImpl* m_connectThread;
    C4JEventImpl* m_tickEvent;
    C4JThreadImpl* m_tickThread;
    PlayerNameMap m_miiNames;
    INQRNetworkManagerListener* m_listener;
    eNQRNetworkManagerInternalState m_internalState;
    eNQRNetworkManagerState m_state;
    bool m_initialised;
    bool m_host;
    bool m_receiving;
    void* m_searchCriteria;
    int m_lastNetworkError;
    bool m_applicationConnected;
    bool m_online;
    int m_sessionId;
    unsigned long long m_localStationId;
    unsigned long long m_hostStationId;
    bool m_matchmaking;
    bool m_localGame;
    int* m_roomAttributes;
    int m_roomAttributeCount;
    OSFastMutex m_playerMutex;
    PlayerSyncData m_roomPlayers[8];
    NQRNetworkPlayer* m_roomSlotPlayers[8];
    OSFastMutex m_connectedPlayerMutex;
    OSFastMutex m_playerConnectionMutex;
    std::vector<NQRNetworkPlayer*> m_unconnectedPlayers;
    std::deque<SendQueueItem> m_sendQueue;
    OSFastMutex m_sendQueueMutex;
    unsigned int m_roomPlayerMask;
    int m_roomPlayerCount;
    int m_localPlayerCount;
    int m_localPlayersJoined;
    std::vector<FriendSearchResult> m_friendSearchResults;
    OSFastMutex m_friendSearchMutex;
    bool m_friendSearchBusy;
    unsigned int m_friendPresenceCount;
    std::vector<NQRNetworkPlayer*> m_players;
    OSFastMutex m_connectedStationMutex;
    std::vector<ConnectedStation> m_connectedStations;
    unsigned char m_nextSmallId;
    void* m_nexContext;
    ENEXAsyncProcess m_nexAsyncProcess;
    unsigned int m_pendingSessionId;
    int m_pendingUserMask;
    int m_maxPlayers;
    unsigned char m_gameMode;
    PresenceSyncInfo m_presenceInfo;
    void* m_externalData;
    unsigned int m_externalDataSize;
    bool m_running;
    bool m_leaveRequested;
    bool m_createRequested;
    bool m_joinRequested;
    bool m_browseRequested;
    bool m_randomJoinRequested;
    bool m_updateRequested;
    unsigned int m_lastRateControlledEvent;
};

#endif
