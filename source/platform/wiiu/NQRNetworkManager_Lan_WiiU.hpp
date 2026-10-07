#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_LAN_WIIU_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_LAN_WIIU_HPP

#include <deque>
#include <cstring>
#include <map>
#include <string>
#include <vector>

#include "game/core/CriticalSection.hpp"
#include "game/network/INQRNetworkManagerListener.hpp"
#include "nn/pia/lan/LanSessionInfo.hpp"
#include "platform/wiiu/C4JStorage.hpp"
#include "platform/wiiu/NQRNetworkManager.hpp"

class C4JEventImpl;
class C4JThread;
class C4JThreadImpl;


namespace nn
{
    namespace fp
    {
        struct FriendPresence;
    }
    namespace pia
    {
        namespace session
        {
            class SessionSearchCriteria;
        }
    }
}

class NQRNetworkManager_Lan_WiiU : public NQRNetworkManager
{
public:
    enum ESaveTransferState
    {
        eSaveTransfer_Idle,
        eSaveTransfer_CreatingSession,
        eSaveTransfer_CreateSessionFailed,
        eSaveTransfer_WaitingForReceiver,
        eSaveTransfer_Transferring,
        eSaveTransfer_SendComplete,
        eSaveTransfer_Searching,
        eSaveTransfer_NoSessionFound,
        eSaveTransfer_Unused,
        eSaveTransfer_SessionFound,
        eSaveTransfer_Joining,
        eSaveTransfer_JoinFailed,
        eSaveTransfer_Joined,
        eSaveTransfer_ReceiveComplete,
        eSaveTransfer_ConnectionLost
    };

    enum NetworkType
    {
        eNetworkType_None,
        eNetworkType_Adhoc,
        eNetworkType_Online,
        eNetworkType_Local,
        eNetworkType_Lan
    };

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
        unsigned char* data;
        unsigned int size;
    };

    struct FriendSearchResult
    {
        unsigned int unknown0;
        unsigned int ownerPrincipalId;
        unsigned int unknown8;
        unsigned int ownerLocalFriendIndex;
        char ownerName[36];
        unsigned int sessionId;
        bool joinable;
        void* extData;
        void* unknown40;
        nn::pia::lan::LanSessionInfo sessionInfo;
    };

    struct ConnectedStation
    {
        unsigned long long stationId;
        int state;

        ConnectedStation(unsigned long long station) : stationId(station), state(0) {}
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

    NQRNetworkManager_Lan_WiiU(INQRNetworkManagerListener* listener);
    virtual ~NQRNetworkManager_Lan_WiiU();

    virtual void Tick();
    virtual void Initialise();
    virtual void Terminate();
    virtual eNQRNetworkManagerState GetState();
    virtual bool IsHost();
    virtual bool IsReadyToPlayOrIdle();
    virtual bool IsInSession();
    virtual bool IsMatchmakingEnabled();
    virtual void CreateAndJoinRoom(int userIndexMask, int maxPlayers, void* extData, int extDataSize, int* attributes, int attributeCount, bool matchmaking, unsigned char gameMode);
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
    void SetFriendSearchState(eNQRNetworkManagerFriendSearchState state);
    static int InvitationSignInReturned(void* context, bool signedIn, int error, int userIndex);
    static int InvitationMustConnectReturned(void* context, int pad, C4JStorage::EMessageResult result);
    NQRNetworkPlayer* GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex);
    void RemoveConnectedStation(unsigned long long stationId);
    void RemoveConnectedPlayer(NQRNetworkPlayer* player);
    void NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId);
    void AddConnectedPlayer(NQRNetworkPlayer* player);
    void HandlePlayerJoined(NQRNetworkPlayer* player);
    NQRNetworkPlayer* FindOrCreateNonNetworkPlayer(int type, int localIndex, PlayerSyncData* syncData);
    void NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId);
    void MapRoomSlotPlayers(int removedIndex);
    void UpdateRoomSyncUIDsFromPlayers();
    void RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size);
    void SyncRoomData();
    void RemoveRemotePlayersAndSync(unsigned long long stationId, int localIndex);
    void ClearSendQueue();
    void SetState(eNQRNetworkManagerInternalState state);
    void TickSendQueue();
    int InitialiseNetwork(NetworkType type);
    void Pia_InitializeCommon(unsigned char* buffer, unsigned int size, unsigned long long stationId, bool host, bool online);
    void Lan_InitializePiaLan();
    void Pia_InitializeSession(const void* setting);
    ConnectedStation* GetConnectedStation(unsigned long long stationId);
    void AddConnectedStation(unsigned long long stationId);
    void Pia_SignalLeaveSession();
    static void Pia_SessionCallback(int eventType, unsigned long long stationId, void* context);
    void InitialiseAfterOnline();
    bool IsInitialisedAfterOnline();
    void OnlineCheck();
    void FriendSearchTick();
    void Session_CleanupSession();
    bool Inet_Logout();
    void Lan_CleanupPiaLan();
    void Inet_FinalizeNetwork();
    void Session_FinalizePiaSession();
    void Lan_FinalizePiaLan();
    void Inet_FinalizePiaInet();
    void Pia_FinalizeCommon();
    void Pia_Finalize();
    void Pia_Shutdown();
    void SetLastNetworkError(int result);
    void Net_GetLocalAddress(unsigned int* address, unsigned int* subnetMask);
    bool Lan_StartupPiaLan();
    void UpdateApplicationConnected();
    bool Pia_CreateSession();
    bool canRunRateControlledEvent();
    bool Pia_JoinSession();
    bool Pia_JoinMatchmakeSession();
    bool Pia_LeaveSession();
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
    void UpdateRichPresenceCustomData(PresenceSyncInfo* presenceInfo);
    static void MatchmakingExtDataUpdatedCallback(bool success, void* context);
    void RemoveAllPlayersOnClient();
    void ResetToIdle();
    void TickNexAsyncProcess();
    void SendJoinData();
    void SendSaveTransferJoinData();
    void NetworkPlayerConnectionComplete(NQRNetworkPlayer* player);
    void PollUnconnectedStations();
    bool AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData, EJoinFailedReason* reason);
    void SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason);
    void SendSaveTransferAckPacket();
    void PollReceiveData();
    void SendSaveTransferDataPacket(void* data, int size);
    void TickSaveTransferData();
    static void ShowErrorViewer();
    void RemoveLocalClientPlayers(int userIndex);
    void ErrorHandlingTick();
    void TickFromThread();
    void RunTickThread();
    static int TickThreadProc(void* context);
    void MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo);
    static void GetFriendsThreadProc(void* context);
    bool Session_BrowseSession(nn::pia::session::SessionSearchCriteria* criteria);
    unsigned int GetFriendsFromSessionBrowse();
    void ClearRichPresenceCustomData();
    void UpdateRemotePlay();
    void SendLeaveData(unsigned int playerMask);
    static bool IsHandlingBootInvite();
    NQRNetworkPlayer* GetPlayerIfReady(NQRNetworkPlayer* player);
    static void SetAppSuspended(bool suspended);
    void SignalInitialiseAfterOnline();
    unsigned int GetSaveTransferConnectionFromSessionBrowse();
    static void GetSaveTransferConnectionThreadProc(void* context);
    void Pia_ShutdownForOnline();
    void CreateSessionForSaveTransfer(int dataSize, void* data, int metadataSize, void* metadata, long long seed, unsigned int hostOptions, unsigned int texturePack, wchar_t* name);
    unsigned int FindSessionForSaveTransfer();
    void JoinSaveTransferConnection();

    void UpdatePlayersFromRoomSyncUIDs();
    int GetOldMask(int mask);
    int GetAddedMask(int oldMask, int newMask);
    int GetRemovedMask(int oldMask, int newMask);
    void ForceErrorPoint(eNQRForceError error);
    static void RefreshChatAndContentRestrictionsReturned_HandleInvite(void* context);
    void ResetNEXAsyncContext();
    void Pia_SignalJoinMatchmakeSession();
    void Pia_SignalJoinRandomSession();
    static void Pia_CheckGetLocalAndHostStationId();
    const char* GetNexASyncProcessString(ENEXAsyncProcess process);
    int Net_InitialiseNetworkInterface(NetworkType type);
    void Net_InitialiseFriends();
    static void ResumeFromSleep();
    bool IsNetworkConnected();

private:
    typedef std::map<std::wstring, std::wstring> PlayerNameMap;

    struct SaveTransferDescriptor
    {
        unsigned short kind;
        unsigned short version;
        unsigned int dataSize;
        unsigned int metadataSize;
        long long seed;
        unsigned int hostOptions;
        unsigned int texturePack;
        wchar_t name[64];

        SaveTransferDescriptor() : kind(0), version(0), dataSize(0), metadataSize(0), seed(0), hostOptions(0), texturePack(0)
        {
            std::memset(name, 0, sizeof(name));
        }
    };

    bool m_friendPresenceUpdated;
    C4JThread* m_tickThread;
    C4JEventImpl* m_tickEvent;
    C4JThreadImpl* m_connectThread;
    PlayerNameMap m_miiNames;
    int m_unknownCC;
    bool m_applicationConnected;
    bool m_online;
    INQRNetworkManagerListener* m_listener;
    eNQRNetworkManagerInternalState m_internalState;
    eNQRNetworkManagerState m_state;
    bool m_initialised;
    bool m_host;
    bool m_hostStationIdCached;
    unsigned long long m_cachedJointSessionHostStationId;
    unsigned long long m_cachedHostStationId;
    int m_pendingUserMask;
    int m_maxPlayers;
    unsigned char m_unknown100[0xdd4 - 0x100];
    bool m_saveTransferSession;
    unsigned char m_unknownDD5[0xe84 - 0xdd5];
    bool m_receiving;
    unsigned char m_unknownE85[8];
    bool m_matchmaking;
    OSFastMutex m_playerMutex;
    PlayerSyncData m_roomPlayers[8];
    unsigned char m_unknown1140[0x116c - 0x1140];
    std::vector<NQRNetworkPlayer*> m_unconnectedPlayers;
    std::deque<SendQueueItem> m_sendQueue;
    OSFastMutex m_sendQueueMutex;
    unsigned char m_unknown11BC[0x1250 - 0x11bc];
    eNQRNetworkManagerFriendSearchState m_friendSearchState;
    unsigned char m_unknown1254[4];
    bool m_friendSearchBusy;
    unsigned char m_unknown1259[0x1268 - 0x1259];
    OSFastMutex m_friendSearchMutex;
    unsigned char m_unknown1294[0x12a4 - 0x1294];
    std::vector<FriendSearchResult> m_friendSearchResults;
    unsigned char m_unknown12B4[0x12e0 - 0x12b4];
    std::vector<NQRNetworkPlayer*> m_players;
    OSFastMutex m_connectedStationMutex;
    std::vector<ConnectedStation> m_connectedStations;
    unsigned char m_unknown132C[0x1360 - 0x132c];
    void* m_nexContext;
    ENEXAsyncProcess m_nexAsyncProcess;
    unsigned char m_unknown1368[0x1381 - 0x1368];
    bool m_updateRequested;
    unsigned char m_unknown1382[0x1390 - 0x1382];
    bool m_createRequested;
    bool m_joinRequested;
    unsigned char m_unknown1392[1];
    bool m_leaveRequested;
    bool m_browseRequested;
    bool m_randomJoinRequested;
    unsigned char m_unknown1396[0x13a8 - 0x1396];

public:
    unsigned int m_saveTransferSize;
    unsigned char* m_saveTransferData;
    unsigned int m_saveTransferOffset;

private:
    unsigned char m_unknown13B4[0x13fc - 0x13b4];

public:
    ESaveTransferState m_saveTransferState;

private:
    unsigned char m_unknown1400[0x143c - 0x1400];

public:
    unsigned int m_saveTransferSessionId;

private:
    unsigned char m_unknown1440[0x1bc8 - 0x1440];
    unsigned long long m_lastRateControlledEvent;
    unsigned char m_unknown1BD0[0x1bd8 - 0x1bd0];
    void* m_searchCriteria;
    int m_sessionId;
    unsigned int m_roomPlayerMask;
    unsigned int m_friendPresenceCount;
    unsigned int m_nextSmallId;
    unsigned int m_pendingSessionId;
    unsigned char m_gameMode;
    PresenceSyncInfo m_presenceInfo;
    void* m_externalData;
    unsigned int m_externalDataSize;
    bool m_running;

    bool m_saveTransferPacketPending;
    SaveTransferDescriptor m_saveTransferDescriptor;
    unsigned int m_saveTransferGeneration;
    unsigned int m_saveTransferFlags;
    unsigned int m_saveTransferPacketSize;
    unsigned char m_saveTransferMode;

};

extern NQRNetworkManager_Lan_WiiU* gLanNetworkManager;

#endif
