#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_HPP

#include <deque>
#include <string>

#include "game/core/CriticalSection.hpp"
#include "game/world/generation/PlayerUID.hpp"

class CMatchMakeSearchParam;
class NQRNetworkPlayer;

namespace nn
{
    namespace fp
    {
        struct FriendPresence;
    }
}

class NQRNetworkManager
{
public:
    enum eNQRNetworkManagerState
    {
        eNQRNetworkManagerState_Uninitialised,
        eNQRNetworkManagerState_Initialising,
        eNQRNetworkManagerState_Idle,
        eNQRNetworkManagerState_Hosting,
        eNQRNetworkManagerState_Joining,
        eNQRNetworkManagerState_Starting,
        eNQRNetworkManagerState_Playing,
        eNQRNetworkManagerState_Leaving,
        eNQRNetworkManagerState_Ending
    };

    enum eNQRNetworkManagerInternalState
    {
        eNQRNetworkManagerInternalState_Uninitialised,
        eNQRNetworkManagerInternalState_Initialising,
        eNQRNetworkManagerInternalState_Idle,
        eNQRNetworkManagerInternalState_IdleOnline,
        eNQRNetworkManagerInternalState_HostCreatingSession,
        eNQRNetworkManagerInternalState_HostSessionCreated,
        eNQRNetworkManagerInternalState_HostCreatingRoom,
        eNQRNetworkManagerInternalState_HostRegisteringSession,
        eNQRNetworkManagerInternalState_HostRoomReady,
        eNQRNetworkManagerInternalState_JoinCreatingSession,
        eNQRNetworkManagerInternalState_JoinRequested,
        eNQRNetworkManagerInternalState_JoinJoiningSession,
        eNQRNetworkManagerInternalState_JoinSessionJoined,
        eNQRNetworkManagerInternalState_JoinRoomReady,
        eNQRNetworkManagerInternalState_JoinSearchingMatchmakeSession,
        eNQRNetworkManagerInternalState_JoinFoundMatchmakeSession,
        eNQRNetworkManagerInternalState_JoinJoiningMatchmakeSession,
        eNQRNetworkManagerInternalState_JoinMatchmakeSessionJoined,
        eNQRNetworkManagerInternalState_EndingSession,
        eNQRNetworkManagerInternalState_StartingGame,
        eNQRNetworkManagerInternalState_Playing,
        eNQRNetworkManagerInternalState_LeavingRoom,
        eNQRNetworkManagerInternalState_LeavingSession,
        eNQRNetworkManagerInternalState_Ending
    };

    enum eNQRNetworkManagerFriendSearchState
    {
        eNQRNetworkManagerFriendSearchState_Idle,
        eNQRNetworkManagerFriendSearchState_Searching,
        eNQRNetworkManagerFriendSearchState_Complete
    };

    enum eNQRForceError
    {
        eNQRForceError_None,
        eNQRForceError_Connect,
        eNQRForceError_CreateSession,
        eNQRForceError_JoinSession,
        eNQRForceError_StartGame,
        eNQRForceError_InGame,
        eNQRForceError_LeaveSession
    };

    struct StateChangeInfo
    {
        eNQRNetworkManagerState eOldState;
        eNQRNetworkManagerState eNewState;
        bool bError;
    };

    struct PresenceSyncInfo
    {
        unsigned int m_uiWorldId;
        unsigned short m_usTitleVersion;
        unsigned char m_ucGameMode;
        unsigned char m_ucPlayerCount;
        unsigned char m_ucMaxPlayers;
        bool m_bJoinable;
        unsigned int m_uiHostAttributes;
    };

    struct SessionSearchResult
    {
        unsigned int m_uiSessionId;
        unsigned int m_uiOwnerPrincipalId;
        unsigned int m_uiOwnerLocalFriendIndex;
        char m_szOwnerName[17];
        void* m_pExtData;
    };

    class PlayerSyncData : public PlayerUID
    {
    public:
        PlayerSyncData();

        unsigned long long m_uiStationId;
        char m_szNetworkName[22];
        wchar_t m_szMiiName[11];
        unsigned int m_uiSmallId : 8;
        unsigned int m_uiLocalPlayerIndex : 4;
        unsigned int m_uiSyncFlags : 20;
    };

    virtual ~NQRNetworkManager();

    virtual void Tick() = 0;
    virtual void Initialise() = 0;
    virtual void Terminate() = 0;
    virtual eNQRNetworkManagerState GetState() = 0;
    virtual bool IsHost() = 0;
    virtual bool IsReadyToPlayOrIdle() = 0;
    virtual bool IsInSession() = 0;
    virtual bool IsMatchmakingEnabled() = 0;
    virtual void CreateAndJoinRoom(int iUserIndexMask, int iMaxPlayers, void* pExtData, int iExtDataSize, int* piAttributes, int iAttributeCount, bool bMatchmaking, unsigned char ucGameMode) = 0;
    virtual void UpdateExternalRoomData() = 0;
    virtual bool FriendRoomManagerIsBusy() = 0;
    virtual bool FriendRoomManagerSearch() = 0;
    virtual bool FriendRoomManagerSearch2() = 0;
    virtual int FriendRoomManagerGetCount() = 0;
    virtual void FriendRoomManagerGetRoomInfo(int iIndex, SessionSearchResult* pResult) = 0;
    virtual bool JoinRoom(SessionSearchResult* pResult, int iUserIndexMask) = 0;
    virtual bool JoinRoom(unsigned int uiSessionId, int iUserIndexMask, const PresenceSyncInfo* pPresenceInfo) = 0;
    virtual bool JoinRoom(int iUserIndexMask, const nn::fp::FriendPresence* pFriendPresence) = 0;
    virtual bool FriendPresenceUpdatedRoomList() = 0;
    virtual SessionSearchResult* GetFriendRoomSearchResults(int* piCount) = 0;
    virtual void GetExtDataForRoom(unsigned int uiSessionId, void* pExtData, void (*pCallback)(bool, void*), void* pCallbackData) = 0;
    virtual bool FindAndJoinRoom(int iUserIndexMask, unsigned int uiSearchFlags, CMatchMakeSearchParam* pSearchParam, unsigned char ucGameMode) = 0;
    virtual void StartGame() = 0;
    virtual void LeaveRoom(bool bError) = 0;
    virtual void EndGame() = 0;
    virtual bool SessionHasSpace(int iExtraPlayers) = 0;
    virtual bool SessionHasMii(int iUserIndex) = 0;
    virtual bool AddLocalPlayerByUserIndex(int iUserIndex) = 0;
    virtual bool RemoveLocalPlayerByUserIndex(int iUserIndex) = 0;
    virtual void SendInviteGUI() = 0;
    virtual int GetPlayerCount() = 0;
    virtual int GetOnlinePlayerCount() = 0;
    virtual NQRNetworkPlayer* GetPlayerByIndex(int iIndex) = 0;
    virtual NQRNetworkPlayer* GetPlayerBySmallId(int iSmallId) = 0;
    virtual NQRNetworkPlayer* GetPlayerByXuid(PlayerUID uid) = 0;
    virtual std::wstring GetDisplayNameByGamertag(std::wstring gamertag) = 0;
    virtual NQRNetworkPlayer* GetLocalPlayerByUserIndex(int iUserIndex) = 0;
    virtual NQRNetworkPlayer* GetHostPlayer() = 0;
    virtual void SetPresenceDataStartHostingGame() = 0;
    virtual int GetJoiningReadyPercentage() = 0;
    virtual void LocalDataSend(NQRNetworkPlayer* pFrom, NQRNetworkPlayer* pTo, const void* pData, unsigned int uiSize) = 0;
    virtual void RemoteDataSend(NQRNetworkPlayer* pFrom, NQRNetworkPlayer* pTo, const void* pData, unsigned int uiSize) = 0;
    virtual int GetSessionIndex(NQRNetworkPlayer* pPlayer) = 0;
    virtual void TryToConnectOnline(int (*pCallback)(void*, bool, int), void* pCallbackData, bool bShowProgress) = 0;
    virtual bool isAdhocConnected() = 0;
    virtual bool connectToAdhoc() = 0;
    virtual bool isLanConnected() = 0;

    int GetOutstandingAckCount(NQRNetworkPlayer* pPlayer);
    int GetSendQueueSizeBytes();
    int GetSendQueueSizeMessages();

    static void SafeToRespondToGameBootInvite(int iUserIndex);

protected:
    std::deque<StateChangeInfo> m_stateChanges;
    OSFastMutex m_stateChangeLock;
    OSFastMutex m_playerLock;
    OSFastMutex m_localPlayerRequestLock;
    std::deque<int> m_localPlayerRequests;
};

extern int g_iGameBootInviteUserIndex;
extern bool g_bSafeToRespondToGameBootInvite;

#endif
