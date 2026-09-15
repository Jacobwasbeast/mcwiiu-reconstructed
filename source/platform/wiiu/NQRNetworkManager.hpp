#ifndef PLATFORM_WIIU_NQRNETWORKMANAGER_HPP
#define PLATFORM_WIIU_NQRNETWORKMANAGER_HPP

#include <cstddef>
#include <cstdint>
#include <new>
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
    template <typename T, unsigned int BlockBytes, std::size_t TargetSize>
    class TargetDeque
    {
    public:
        TargetDeque() : m_map(0), m_mapSlots(0), m_offset(0), m_count(0) {}
        ~TargetDeque() {}

        TargetDeque(const TargetDeque&) = delete;
        TargetDeque& operator=(const TargetDeque&) = delete;

        bool empty() const { return m_count == 0; }

        void push_back(const T& value)
        {
            const unsigned int capacity = BlockCapacity();
            EnsureCapacity(m_count + 1);
            const unsigned int index = m_offset + m_count;
            const unsigned int block = (index / capacity) % m_mapSlots;
            const unsigned int element = index % capacity;
            T* blockData = static_cast<T**>(reinterpret_cast<void*>(
                static_cast<std::uintptr_t>(m_map)))[block];
            if (blockData == 0)
            {
                blockData = reinterpret_cast<T*>(::operator new(BlockCapacity() * sizeof(T)));
                static_cast<T**>(reinterpret_cast<void*>(static_cast<std::uintptr_t>(m_map)))[block] = blockData;
            }
            new (blockData + element) T(value);
            ++m_count;
        }

        void pop_front()
        {
            if (m_count != 0)
            {
                const unsigned int capacity = BlockCapacity();
                const unsigned int block = (m_offset / capacity) % m_mapSlots;
                const unsigned int element = m_offset % capacity;
                T* blockData = static_cast<T**>(reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(m_map)))[block];
                (blockData + element)->~T();
                --m_count;
                m_offset = m_count == 0 ? 0 : (m_offset + 1) % (m_mapSlots * capacity);
            }
        }

        void pop_back()
        {
            if (m_count != 0)
            {
                const unsigned int capacity = BlockCapacity();
                const unsigned int index = m_offset + m_count - 1;
                const unsigned int block = (index / capacity) % m_mapSlots;
                const unsigned int element = index % capacity;
                T** blocks = reinterpret_cast<T**>(static_cast<std::uintptr_t>(m_map));
                (blocks[block] + element)->~T();
                --m_count;
                if (m_count == 0)
                    m_offset = 0;
            }
        }

        void clear() { _Tidy(); }

        void _Tidy()
        {
            if (m_map != 0)
            {
                T** blocks = static_cast<T**>(reinterpret_cast<void*>(
                    static_cast<std::uintptr_t>(m_map)));
                while (m_count != 0)
                    pop_back();
                for (unsigned int i = 0; i < m_mapSlots; ++i)
                    if (blocks[i] != 0)
                        ::operator delete(blocks[i]);
                ::operator delete(blocks);
            }
            m_map = 0;
            m_mapSlots = 0;
            m_offset = 0;
            m_count = 0;
        }

    private:
        static unsigned int BlockCapacity()
        {
            return BlockBytes / sizeof(T);
        }

        void EnsureCapacity(unsigned int required)
        {
            const unsigned int capacity = BlockCapacity();
            if (m_map == 0)
            {
                m_mapSlots = 8;
                T** replacement = reinterpret_cast<T**>(::operator new(sizeof(T*) * m_mapSlots));
                for (unsigned int i = 0; i < m_mapSlots; ++i)
                    replacement[i] = 0;
                m_map = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(replacement));
                return;
            }
            const unsigned int requiredBlocks =
                (m_offset % capacity + required + capacity - 1) / capacity;
            if (m_mapSlots >= requiredBlocks)
                return;
            const unsigned int oldSlots = m_mapSlots;
            const unsigned int slots = oldSlots * 2;
            T** previous = reinterpret_cast<T**>(static_cast<std::uintptr_t>(m_map));
            T** replacement = reinterpret_cast<T**>(::operator new(sizeof(T*) * slots));
            for (unsigned int i = 0; i < slots; ++i)
                replacement[i] = 0;
            const unsigned int firstBlock = m_offset / capacity;
            const unsigned int usedBlocks = (m_count + (m_offset % capacity) + capacity - 1) / capacity;
            for (unsigned int i = 0; i < usedBlocks; ++i)
                replacement[i] = previous[(firstBlock + i) % oldSlots];
            ::operator delete(previous);
            m_map = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(replacement));
            m_mapSlots = slots;
            m_offset %= capacity;
        }

        std::uint32_t m_allocator;
        std::uint32_t m_map;
        std::uint32_t m_mapSlots;
        std::uint32_t m_offset;
        std::uint32_t m_count;
        unsigned char m_targetStorage[TargetSize - 0x14];
    };

    struct VTableEntry
    {
        std::uint32_t function;
        std::int32_t adjustment;
    };

    struct VTable
    {
        std::uint32_t header[3];
        VTableEntry slots[48];
    };

    NQRNetworkManager();
    ~NQRNetworkManager();

    template <typename Return, typename... Arguments>
    Return Dispatch(unsigned int slot, Arguments... arguments)
    {
        const VTable* table = reinterpret_cast<const VTable*>(
            static_cast<std::uintptr_t>(m_ghsVTable));
        typedef Return (*Function)(NQRNetworkManager*, Arguments...);
        Function function = reinterpret_cast<Function>(
            static_cast<std::uintptr_t>(table->slots[slot].function));
        return function(this, arguments...);
    }

    enum VTableSlot
    {
        eVTableDestructor = 0,
        eVTableTick = 1,
        eVTableInitialise,
        eVTableTerminate,
        eVTableGetState,
        eVTableIsHost,
        eVTableIsReadyToPlayOrIdle,
        eVTableIsInSession,
        eVTableIsMatchmakingEnabled,
        eVTableCreateAndJoinRoom,
        eVTableUpdateExternalRoomData,
        eVTableFriendRoomManagerIsBusy,
        eVTableFriendRoomManagerSearch,
        eVTableFriendRoomManagerSearch2,
        eVTableFriendRoomManagerGetCount,
        eVTableFriendRoomManagerGetRoomInfo,
        eVTableJoinRoomResult,
        eVTableJoinRoomSession,
        eVTableJoinRoomFriend,
        eVTableFriendPresenceUpdatedRoomList,
        eVTableGetFriendRoomSearchResults,
        eVTableGetExtDataForRoom,
        eVTableFindAndJoinRoom,
        eVTableStartGame,
        eVTableLeaveRoom,
        eVTableEndGame,
        eVTableSessionHasSpace,
        eVTableSessionHasMii,
        eVTableAddLocalPlayerByUserIndex,
        eVTableRemoveLocalPlayerByUserIndex,
        eVTableSendInviteGUI,
        eVTableGetPlayerCount,
        eVTableGetOnlinePlayerCount,
        eVTableGetPlayerByIndex,
        eVTableGetPlayerBySmallId,
        eVTableGetPlayerByXuid,
        eVTableGetDisplayNameByGamertag,
        eVTableGetLocalPlayerByUserIndex,
        eVTableGetHostPlayer,
        eVTableSetPresenceDataStartHostingGame,
        eVTableGetJoiningReadyPercentage,
        eVTableLocalDataSend,
        eVTableRemoteDataSend,
        eVTableGetSessionIndex,
        eVTableTryToConnectOnline,
        eVTableIsAdhocConnected,
        eVTableConnectToAdhoc,
        eVTableIsLanConnected,
        eVTableCount = 48
    };

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

    void Tick(); void Initialise(); void Terminate();
    eNQRNetworkManagerState GetState(); bool IsHost();
    bool IsReadyToPlayOrIdle(); bool IsInSession(); bool IsMatchmakingEnabled();
    void CreateAndJoinRoom(int, int, void*, int, int*, int, bool, unsigned char);
    void UpdateExternalRoomData(); bool FriendRoomManagerIsBusy();
    bool FriendRoomManagerSearch(); bool FriendRoomManagerSearch2();
    int FriendRoomManagerGetCount();
    void FriendRoomManagerGetRoomInfo(int, SessionSearchResult*);
    bool JoinRoom(SessionSearchResult*, int);
    bool JoinRoom(unsigned int, int, const PresenceSyncInfo*);
    bool JoinRoom(int, const nn::fp::FriendPresence*);
    bool FriendPresenceUpdatedRoomList();
    SessionSearchResult* GetFriendRoomSearchResults(int*);
    void GetExtDataForRoom(unsigned int, void*, void (*)(bool, void*), void*);
    bool FindAndJoinRoom(int, unsigned int, CMatchMakeSearchParam*, unsigned char);
    void StartGame(); void LeaveRoom(bool); void EndGame();
    bool SessionHasSpace(int); bool SessionHasMii(int);
    bool AddLocalPlayerByUserIndex(int); bool RemoveLocalPlayerByUserIndex(int);
    void SendInviteGUI(); int GetPlayerCount(); int GetOnlinePlayerCount();
    NQRNetworkPlayer* GetPlayerByIndex(int); NQRNetworkPlayer* GetPlayerBySmallId(int);
    NQRNetworkPlayer* GetPlayerByXuid(PlayerUID);
    std::wstring GetDisplayNameByGamertag(std::wstring);
    NQRNetworkPlayer* GetLocalPlayerByUserIndex(int); NQRNetworkPlayer* GetHostPlayer();
    void SetPresenceDataStartHostingGame(); int GetJoiningReadyPercentage();
    void LocalDataSend(NQRNetworkPlayer*, NQRNetworkPlayer*, const void*, unsigned int);
    void RemoteDataSend(NQRNetworkPlayer*, NQRNetworkPlayer*, const void*, unsigned int);
    int GetSessionIndex(NQRNetworkPlayer*);
    void TryToConnectOnline(int (*)(void*, bool, int), void*, bool);
    bool isAdhocConnected(); bool connectToAdhoc(); bool isLanConnected();

    int GetOutstandingAckCount(NQRNetworkPlayer* pPlayer);
    int GetSendQueueSizeBytes();
    int GetSendQueueSizeMessages();

    static void SafeToRespondToGameBootInvite(int iUserIndex);

protected:
    TargetDeque<StateChangeInfo, 0x0c, 0x14> m_stateChanges;
    OSFastMutex m_stateChangeLock;
    OSFastMutex m_playerLock;
    OSFastMutex m_localPlayerRequestLock;
    TargetDeque<int, 0x10, 0x14> m_localPlayerRequests;
    std::uint32_t m_ghsVTable;

};

static_assert(sizeof(NQRNetworkManager::TargetDeque<NQRNetworkManager::StateChangeInfo, 0x0c, 0x14>) == 0x14,
              "NQR StateChangeInfo deque ABI");
static_assert(sizeof(NQRNetworkManager::TargetDeque<int, 0x10, 0x14>) == 0x14,
              "NQR local request deque ABI");
static_assert(sizeof(NQRNetworkManager) == 0xb0, "NQR base ABI size");

extern int g_iGameBootInviteUserIndex;
extern bool g_bSafeToRespondToGameBootInvite;

#endif
