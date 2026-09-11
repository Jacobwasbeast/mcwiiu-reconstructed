#include "platform/wiiu/NQRNetworkManager_Lan_WiiU.hpp"

#include <algorithm>
#include <cstring>
#include <cwchar>

#include "game/core/System.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanSupport.hpp"
#include "nn/pia/session/Session.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"

void AssertPiaError(nn::pia::Result& result);

static NQRNetworkManager_Lan_WiiU* s_networkManager;
static int (*s_connectCallback)(void*, bool, int);
static void* s_connectCallbackData;
static bool s_cancelConnect;
static bool s_appSuspended;
static bool s_handlingBootInvite;
static bool s_initialiseAfterOnline;
static bool s_sessionDisconnectPending;

unsigned int GetTickCount();
extern "C" int socket_lib_init();
extern "C" int socket_lib_finish();
extern "C" void OSFastMutex_Unlock(OSFastMutex* mutex);
extern "C" void checkPiaCoreToUse();

static bool s_socketLibraryInitialised;
static bool s_localCommunicationInitialised;

int Net_InitialiseSocket()
{
    int result = socket_lib_init();
    s_socketLibraryInitialised = result != 0;
    return result != 0 ? 3 : 0;
}

int Net_FinaliseSocket()
{
    if (s_socketLibraryInitialised)
    {
        int result = socket_lib_finish();
        if (result != 0)
            return 3;
        s_socketLibraryInitialised = false;
    }
    return 0;
}

void LeaveCriticalSection(OSFastMutex* mutex)
{
    OSFastMutex_Unlock(mutex);
}

void LeaveCriticalSection(void* mutex)
{
    OSFastMutex_Unlock(static_cast<OSFastMutex*>(mutex));
}

void InitializeCriticalSection(OSFastMutex* mutex)
{
    OSFastMutex_Init(mutex, "map/set<T> too long");
}

void InitializeCriticalSectionAndSpinCount(OSFastMutex* mutex, unsigned long)
{
    InitializeCriticalSection(mutex);
}

void EnterCriticalSection(OSFastMutex* mutex)
{
    OSFastMutex_Lock(mutex);
}

int TryEnterCriticalSection(OSFastMutex* mutex)
{
    return OSFastMutex_TryLock(mutex) != 0;
}

void DeleteCriticalSection(OSFastMutex*)
{
}

void Local_FinalizeLocalCommunication()
{
    if (s_localCommunicationInitialised)
        s_localCommunicationInitialised = false;
}

NQRNetworkManager_Lan_WiiU::SendQueueItem::SendQueueItem()
    : stationId(0), data(NULL), size(0)
{
}

NQRNetworkManager_Lan_WiiU::SendQueueItem::SendQueueItem(unsigned long long station, unsigned char sendChannel, const void* sendData, unsigned int sendSize)
    : stationId(station), data(new unsigned char[sendSize + 1]), size(sendSize + 1)
{
    data[0] = sendChannel;
    if (sendData != NULL)
        std::memcpy(data + 1, sendData, sendSize);
}

NQRNetworkManager_Lan_WiiU::SendQueueItem::SendQueueItem(const SendQueueItem& other)
    : stationId(other.stationId), data(new unsigned char[other.size]), size(other.size)
{
    std::memcpy(data, other.data, size);
}

NQRNetworkManager_Lan_WiiU::SendQueueItem& NQRNetworkManager_Lan_WiiU::SendQueueItem::operator=(const SendQueueItem& other)
{
    if (this != &other)
    {
        unsigned char* replacement = new unsigned char[other.size];
        std::memcpy(replacement, other.data, other.size);
        delete[] data;
        stationId = other.stationId;
        data = replacement;
        size = other.size;
    }
    return *this;
}

NQRNetworkManager_Lan_WiiU::SendQueueItem::~SendQueueItem()
{
    delete[] data;
}

NQRNetworkManager_Lan_WiiU::FriendSearchResult::FriendSearchResult()
    : ownerPrincipalId(0), ownerLocalFriendIndex(0), sessionId(0), joinable(false), extData(NULL)
{
    std::memset(ownerName, 0, sizeof(ownerName));
    std::memset(&presence, 0, sizeof(presence));
    std::memset(status, 0, sizeof(status));
}

NQRNetworkManager_Lan_WiiU::FriendSearchResult::FriendSearchResult(const FriendSearchResult& other)
    : ownerPrincipalId(other.ownerPrincipalId), ownerLocalFriendIndex(other.ownerLocalFriendIndex),
      presence(other.presence), sessionId(other.sessionId), joinable(other.joinable), extData(other.extData)
{
    std::memcpy(ownerName, other.ownerName, sizeof(ownerName));
    std::memcpy(status, other.status, sizeof(status));
}

NQRNetworkManager_Lan_WiiU::FriendSearchResult& NQRNetworkManager_Lan_WiiU::FriendSearchResult::operator=(const FriendSearchResult& other)
{
    ownerPrincipalId = other.ownerPrincipalId;
    ownerLocalFriendIndex = other.ownerLocalFriendIndex;
    std::memcpy(ownerName, other.ownerName, sizeof(ownerName));
    presence = other.presence;
    sessionId = other.sessionId;
    joinable = other.joinable;
    std::memcpy(status, other.status, sizeof(status));
    extData = other.extData;
    return *this;
}

NQRNetworkManager_Lan_WiiU::ConnectedStation::ConnectedStation()
    : stationId(0), state(0), playerMask(0)
{
}

NQRNetworkManager_Lan_WiiU::ConnectedStation::ConnectedStation(unsigned long long station)
    : stationId(station), state(0), playerMask(0)
{
}

NQRNetworkManager_Lan_WiiU::ClientJoinData::ClientJoinData()
    : playerMask(0), playerUID0(0), playerUID1(0), playerUID2(0), playerUID3(0)
{
    std::memset(onlineIds, 0, sizeof(onlineIds));
    std::memset(displayNames, 0, sizeof(displayNames));
}

NQRNetworkManager_Lan_WiiU::NQRNetworkManager_Lan_WiiU(INQRNetworkManagerListener* listener)
    : m_friendPresenceUpdated(false), m_friendSearchState(eNQRNetworkManagerFriendSearchState_Idle),
      m_connectThread(NULL), m_tickEvent(NULL), m_tickThread(NULL),
      m_listener(listener), m_internalState(eNQRNetworkManagerInternalState_Uninitialised),
      m_state(eNQRNetworkManagerState_Uninitialised), m_initialised(false), m_host(false),
      m_receiving(false), m_searchCriteria(NULL), m_lastNetworkError(0), m_applicationConnected(false), m_online(false),
      m_sessionId(0), m_matchmaking(false), m_hostStationIdCached(false), m_cachedHostStationId(0),
      m_cachedJointSessionHostStationId(0), m_roomPlayerMask(0), m_roomSessionData(0), m_friendSearchBusy(false),
      m_friendPresenceCount(0), m_nextSmallId(0),
      m_nexContext(NULL), m_nexAsyncProcess(eNEXAsyncProcess_None), m_pendingSessionId(0),
      m_pendingUserMask(0), m_maxPlayers(8), m_gameMode(0), m_externalData(NULL),
      m_externalDataSize(0), m_running(false), m_leaveRequested(false), m_createRequested(false),
      m_joinRequested(false), m_browseRequested(false), m_randomJoinRequested(false),
      m_updateRequested(false), m_lastRateControlledEvent(0), m_saveTransferState(0),
      m_saveTransferData(NULL), m_saveTransferSize(0), m_saveTransferOffset(0),
      m_saveTransferPacketPending(false), m_saveTransferSessionId(0)
{
    std::memset(&m_presenceInfo, 0, sizeof(m_presenceInfo));
    InitializeCriticalSection(&m_playerMutex);
    InitializeCriticalSection(&m_sendQueueMutex);
    InitializeCriticalSection(&m_friendSearchMutex);
    InitializeCriticalSection(&m_connectedStationMutex);
    s_networkManager = this;
}

NQRNetworkManager_Lan_WiiU::~NQRNetworkManager_Lan_WiiU()
{
    Terminate();
    ClearSendQueue();
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        delete *player;
    m_players.clear();
    delete[] m_saveTransferData;
    DeleteCriticalSection(&m_connectedStationMutex);
    DeleteCriticalSection(&m_friendSearchMutex);
    DeleteCriticalSection(&m_sendQueueMutex);
    DeleteCriticalSection(&m_playerMutex);
    if (s_networkManager == this)
        s_networkManager = NULL;
}

void NQRNetworkManager_Lan_WiiU::SetNexAsyncProcess(ENEXAsyncProcess process)
{
    m_nexAsyncProcess = process;
}

void NQRNetworkManager_Lan_WiiU::SetFriendSearchState(eNQRNetworkManagerFriendSearchState state)
{
    m_friendSearchState = state;
}









void NQRNetworkManager_Lan_WiiU::Tick()
{
    if (m_tickEvent != NULL)
        m_tickEvent->Set();
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(localIndex) && (*player)->GetSessionIndex() == static_cast<int>(stationId))
            return *player;
    return NULL;
}

void NQRNetworkManager_Lan_WiiU::RemoveConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    for (std::vector<ConnectedStation>::iterator station = m_connectedStations.begin(); station != m_connectedStations.end(); ++station)
    {
        if (station->stationId == stationId)
        {
            m_connectedStations.erase(station);
            break;
        }
    }
    LeaveCriticalSection(&m_connectedStationMutex);
}

void NQRNetworkManager_Lan_WiiU::RemoveConnectedPlayer(NQRNetworkPlayer* player)
{
    if (player == NULL)
        return;
    EnterCriticalSection(&m_playerMutex);
    std::vector<NQRNetworkPlayer*>::iterator position = std::find(m_players.begin(), m_players.end(), player);
    if (position != m_players.end())
        m_players.erase(position);
    LeaveCriticalSection(&m_playerMutex);
    if (m_listener != NULL)
        m_listener->HandlePlayerLeaving(player);
}

void NQRNetworkManager_Lan_WiiU::NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId)
{
    NetworkPlayerSmallIdAllocated(player, smallId);
}

void NQRNetworkManager_Lan_WiiU::AddConnectedPlayer(NQRNetworkPlayer* player)
{
    if (player == NULL)
        return;
    EnterCriticalSection(&m_playerMutex);
    if (std::find(m_players.begin(), m_players.end(), player) == m_players.end())
        m_players.push_back(player);
    LeaveCriticalSection(&m_playerMutex);
    HandlePlayerJoined(player);
}

void NQRNetworkManager_Lan_WiiU::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (m_listener != NULL && player != NULL && player->IsReady())
        m_listener->HandlePlayerJoined(player);
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::FindOrCreateNonNetworkPlayer(int type, int localIndex, PlayerSyncData* syncData)
{
    if (syncData == NULL)
        return NULL;
    NQRNetworkPlayer* player = GetPlayerFromStationIdAndLocalIdx(syncData->m_uiStationId, localIndex);
    if (player == NULL)
    {
        player = new NQRNetworkPlayer(this, static_cast<NQRNetworkPlayer::eNQRNetworkPlayerType>(type), false,
            syncData->m_uiStationId, localIndex, syncData, syncData->m_szNetworkName, syncData->m_szMiiName);
        AddConnectedPlayer(player);
    }
    return player;
}

void NQRNetworkManager_Lan_WiiU::NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId)
{
    if (player != NULL)
    {
        player->SmallIdAllocated(smallId);
        HandlePlayerJoined(player);
    }
}

void NQRNetworkManager_Lan_WiiU::MapRoomSlotPlayers(int removedIndex)
{
    unsigned int mask = 0;
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
    {
        int index = (*player)->GetSessionIndex();
        if (index >= 0 && index < 32 && index != removedIndex)
            mask |= 1u << index;
    }
    m_roomPlayerMask = mask;
    unsigned int count = static_cast<unsigned int>(m_players.size());
    m_roomSessionData = (m_roomSessionData & 0xfff0ffffu) | ((count == 0 ? 0 : count - 1) & 0xfu) << 16;
    UpdateRoomSyncUIDsFromPlayers();
}

void NQRNetworkManager_Lan_WiiU::UpdateRoomSyncUIDsFromPlayers()
{
    for (int index = 0; index < 8; ++index)
        m_roomPlayers[index] = PlayerSyncData();
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
    {
        int index = (*player)->GetSessionIndex();
        if (index >= 0 && index < 8)
        {
            m_roomPlayers[index].m_uiStationId = static_cast<unsigned long long>(index);
            m_roomPlayers[index].m_uiSmallId = (*player)->GetSmallId();
            m_roomPlayers[index].m_uiLocalPlayerIndex = (*player)->GetLocalPlayerIndex();
            m_roomPlayers[index].m_uiSyncFlags = 1;
        }
    }
}

void NQRNetworkManager_Lan_WiiU::RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size)
{
    EnterCriticalSection(&m_sendQueueMutex);
    m_sendQueue.push_back(SendQueueItem(stationId, channel, data, size));
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_Lan_WiiU::SyncRoomData()
{
    UpdateRoomSyncUIDsFromPlayers();
    m_updateRequested = true;
}

void NQRNetworkManager_Lan_WiiU::RemoveRemotePlayersAndSync(unsigned long long stationId, int localIndex)
{
    std::vector<NQRNetworkPlayer*> removed;
    EnterCriticalSection(&m_playerMutex);
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end();)
    {
        if (!(*player)->IsLocal() && (localIndex < 0 || (*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(localIndex)))
        {
            removed.push_back(*player);
            player = m_players.erase(player);
        }
        else
            ++player;
    }
    LeaveCriticalSection(&m_playerMutex);
    for (std::vector<NQRNetworkPlayer*>::iterator player = removed.begin(); player != removed.end(); ++player)
    {
        if (m_listener != NULL)
            m_listener->HandlePlayerLeaving(*player);
        delete *player;
    }
    RemoveConnectedStation(stationId);
    SyncRoomData();
}

void NQRNetworkManager_Lan_WiiU::ClearSendQueue()
{
    EnterCriticalSection(&m_sendQueueMutex);
    m_sendQueue.clear();
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_Lan_WiiU::SetState(eNQRNetworkManagerInternalState state)
{
    eNQRNetworkManagerState oldState = m_state;
    m_internalState = state;
    if (state == eNQRNetworkManagerInternalState_Uninitialised)
        m_state = eNQRNetworkManagerState_Uninitialised;
    else if (state == eNQRNetworkManagerInternalState_Initialising)
        m_state = eNQRNetworkManagerState_Initialising;
    else if (state == eNQRNetworkManagerInternalState_Idle || state == eNQRNetworkManagerInternalState_IdleOnline)
        m_state = eNQRNetworkManagerState_Idle;
    else if (state >= eNQRNetworkManagerInternalState_HostCreatingSession && state <= eNQRNetworkManagerInternalState_HostRoomReady)
        m_state = eNQRNetworkManagerState_Hosting;
    else if (state >= eNQRNetworkManagerInternalState_JoinCreatingSession && state <= eNQRNetworkManagerInternalState_JoinMatchmakeSessionJoined)
        m_state = eNQRNetworkManagerState_Joining;
    else if (state == eNQRNetworkManagerInternalState_StartingGame)
        m_state = eNQRNetworkManagerState_Starting;
    else if (state == eNQRNetworkManagerInternalState_Playing)
        m_state = eNQRNetworkManagerState_Playing;
    else if (state == eNQRNetworkManagerInternalState_LeavingRoom || state == eNQRNetworkManagerInternalState_LeavingSession)
        m_state = eNQRNetworkManagerState_Leaving;
    else
        m_state = eNQRNetworkManagerState_Ending;
    if (m_listener != NULL && oldState != m_state)
        m_listener->HandleStateChange(oldState, m_state, m_lastNetworkError != 0);
}

void NQRNetworkManager_Lan_WiiU::TickSendQueue()
{
    EnterCriticalSection(&m_sendQueueMutex);
    if (!m_sendQueue.empty() && m_applicationConnected)
        m_sendQueue.pop_front();
    LeaveCriticalSection(&m_sendQueueMutex);
}

int NQRNetworkManager_Lan_WiiU::InitialiseNetwork(NetworkType type)
{
    if (type != eNetworkType_Adhoc && type != eNetworkType_Lan)
        return 0;
    return Net_InitialiseSocket();
}

void NQRNetworkManager_Lan_WiiU::Pia_InitializeCommon(unsigned char*, unsigned int, unsigned long long stationId, bool host, bool online)
{
    m_host = host;
    m_online = online;
    m_applicationConnected = true;
    AddConnectedStation(stationId);
}

void NQRNetworkManager_Lan_WiiU::Lan_InitializePiaLan()
{
    nn::pia::Result result = nn::pia::lan::Initialize();
    AssertPiaError(result);
    result = nn::pia::lan::BeginSetup();
    AssertPiaError(result);
    result = nn::pia::lan::LanFacade::CreateInstance();
    AssertPiaError(result);
    result = nn::pia::lan::EndSetup();
    AssertPiaError(result);
}

void NQRNetworkManager_Lan_WiiU::Pia_InitializeSession(const void*)
{
    m_applicationConnected = true;
}

NQRNetworkManager_Lan_WiiU::ConnectedStation* NQRNetworkManager_Lan_WiiU::GetConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    ConnectedStation* result = NULL;
    for (std::vector<ConnectedStation>::iterator station = m_connectedStations.begin(); station != m_connectedStations.end(); ++station)
        if (station->stationId == stationId)
        {
            result = &*station;
            break;
        }
    LeaveCriticalSection(&m_connectedStationMutex);
    return result;
}

void NQRNetworkManager_Lan_WiiU::AddConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    bool found = false;
    for (std::vector<ConnectedStation>::iterator station = m_connectedStations.begin(); station != m_connectedStations.end(); ++station)
        if (station->stationId == stationId)
            found = true;
    if (!found)
        m_connectedStations.push_back(ConnectedStation(stationId));
    LeaveCriticalSection(&m_connectedStationMutex);
}

void NQRNetworkManager_Lan_WiiU::Pia_SignalLeaveSession()
{
    m_leaveRequested = true;
}

void NQRNetworkManager_Lan_WiiU::Pia_SessionCallback(int eventType, unsigned long long stationId, void*)
{
    if (eventType == 0)
    {
        bool hostKnown = s_networkManager->m_host;
        unsigned long long hostStationId = nn::pia::session::g_piaSession->m_hostStationId;
        unsigned long long jointSessionHostStationId = nn::pia::session::g_piaSession->m_jointSessionHostStationId;
        s_networkManager->m_cachedHostStationId = hostStationId;
        s_networkManager->m_cachedJointSessionHostStationId = jointSessionHostStationId;
        s_networkManager->m_hostStationIdCached = true;

        if (!hostKnown)
        {
            s_networkManager->AddConnectedStation(stationId);
            return;
        }
        if (stationId != hostStationId)
            s_networkManager->AddConnectedStation(stationId);
    }
    else if (eventType == 1)
    {
        if (!s_networkManager->m_host)
        {
            if (stationId == s_networkManager->m_cachedJointSessionHostStationId &&
                s_networkManager->m_internalState != eNQRNetworkManagerInternalState_LeavingRoom &&
                !s_sessionDisconnectPending)
            {
                s_networkManager->SetState(eNQRNetworkManagerInternalState_LeavingRoom);
                s_networkManager->Pia_SignalLeaveSession();
            }
        }
        else if (stationId != s_networkManager->m_cachedHostStationId)
        {
            s_networkManager->RemoveRemotePlayersAndSync(stationId, 0xf);
            s_networkManager->RemoveConnectedStation(stationId);
        }
    }
}


void NQRNetworkManager_Lan_WiiU::InitialiseAfterOnline()
{
    Lan_InitializePiaLan();
    m_initialised = InitialiseNetwork(eNetworkType_Lan) != 0 && Lan_StartupPiaLan() && Pia_StartupSession();
    SetState(m_initialised ? eNQRNetworkManagerInternalState_IdleOnline : eNQRNetworkManagerInternalState_Initialising);
}

bool NQRNetworkManager_Lan_WiiU::IsInitialisedAfterOnline()
{
    return m_initialised;
}

void NQRNetworkManager_Lan_WiiU::OnlineCheck()
{
    if (m_applicationConnected && !m_initialised)
        InitialiseAfterOnline();
    else if (!m_applicationConnected && m_initialised)
        m_online = false;
}

void NQRNetworkManager_Lan_WiiU::FriendSearchTick()
{
    if (m_friendSearchState == eNQRNetworkManagerFriendSearchState_Searching)
        GetFriendsFromSessionBrowse();
}


void NQRNetworkManager_Lan_WiiU::Session_CleanupSession()
{
    m_connectedStations.clear();
    m_sessionId = 0;
    m_receiving = false;
}

bool NQRNetworkManager_Lan_WiiU::Inet_Logout()
{
    SetNexAsyncProcess(eNEXAsyncProcess_Logout);
    return true;
}

void NQRNetworkManager_Lan_WiiU::Lan_CleanupPiaLan()
{
    m_online = false;
}

void NQRNetworkManager_Lan_WiiU::Session_FinalizePiaSession()
{
    Session_CleanupSession();
}

void NQRNetworkManager_Lan_WiiU::Lan_FinalizePiaLan()
{
    m_applicationConnected = false;
}

void NQRNetworkManager_Lan_WiiU::Pia_FinalizeCommon()
{
    m_initialised = false;
}

void NQRNetworkManager_Lan_WiiU::Pia_Finalize()
{
    Session_FinalizePiaSession();
    Lan_FinalizePiaLan();
    Lan_CleanupPiaLan();
    Pia_FinalizeCommon();
}

void NQRNetworkManager_Lan_WiiU::Pia_Shutdown()
{
    ClearSendQueue();
    RemoveAllPlayersOnClient();
    Pia_Finalize();
}

void NQRNetworkManager_Lan_WiiU::SetLastNetworkError(int result)
{
    m_lastNetworkError = result;
}

void NQRNetworkManager_Lan_WiiU::Net_GetLocalAddress(unsigned int* address, unsigned int* subnetMask)
{
    if (address != NULL)
        *address = 0;
    if (subnetMask != NULL)
        *subnetMask = 0;
}

bool NQRNetworkManager_Lan_WiiU::Lan_StartupPiaLan()
{
    if (nn::pia::lan::LanFacade::GetInstance() == NULL)
        return false;

    unsigned int address;
    unsigned int subnetMask;
    Net_GetLocalAddress(&address, &subnetMask);

    nn::pia::lan::LanFacade::LanNetworkSetting setting;
    setting.m_field00 = address;
    setting.m_field04 = subnetMask;
    setting.m_field08 = 0;

    nn::pia::Result result = nn::pia::lan::LanFacade::GetInstance()->Bind(setting);
    return result == 0;
}

void NQRNetworkManager_Lan_WiiU::UpdateApplicationConnected()
{
    if (s_appSuspended)
        return;
    if (!m_applicationConnected && m_state != eNQRNetworkManagerState_Idle)
    {
        m_lastNetworkError = 1;
        Pia_Shutdown();
        ResetToIdle();
    }
}

bool NQRNetworkManager_Lan_WiiU::Pia_CreateSession()
{
    m_createRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_CreateSession);
    return true;
}

bool NQRNetworkManager_Lan_WiiU::canRunRateControlledEvent()
{
    unsigned int now = GetTickCount();
    if (now - m_lastRateControlledEvent < 1000)
        return false;
    m_lastRateControlledEvent = now;
    return true;
}

bool NQRNetworkManager_Lan_WiiU::Pia_JoinSession()
{
    m_joinRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_JoinSession);
    return true;
}


bool NQRNetworkManager_Lan_WiiU::Pia_LeaveSession()
{
    m_leaveRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_LeaveSession);
    return true;
}



void NQRNetworkManager_Lan_WiiU::UpdateExternalRoomDataFromThread()
{
    m_updateRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_UpdateSession);
}


bool NQRNetworkManager_Lan_WiiU::Pia_StartupSession()
{
    m_receiving = true;
    return true;
}

void NQRNetworkManager_Lan_WiiU::Pia_SignalCreateSession()
{
    m_createRequested = true;
}

void NQRNetworkManager_Lan_WiiU::Pia_SignalJoinSession()
{
    m_joinRequested = true;
}

void NQRNetworkManager_Lan_WiiU::SetLocalPlayersAndSync()
{
    MapRoomSlotPlayers(-1);
    SyncRoomData();
}

void NQRNetworkManager_Lan_WiiU::ClearIsHandlingBootInvite()
{
    s_handlingBootInvite = false;
}

void NQRNetworkManager_Lan_WiiU::SetNQRPresenceInfoFromExtData(PresenceSyncInfo* presenceInfo, void* extData)
{
    if (presenceInfo != NULL && extData != NULL)
        std::memcpy(presenceInfo, extData, sizeof(PresenceSyncInfo));
}

void NQRNetworkManager_Lan_WiiU::UpdateRichPresenceCustomData(PresenceSyncInfo* presenceInfo)
{
    delete[] static_cast<unsigned char*>(m_externalData);
    m_externalData = NULL;
    m_externalDataSize = 0;
    if (presenceInfo != NULL)
    {
        unsigned char* copy = new unsigned char[sizeof(PresenceSyncInfo)];
        std::memcpy(copy, presenceInfo, sizeof(PresenceSyncInfo));
        m_externalData = copy;
        m_externalDataSize = sizeof(PresenceSyncInfo);
    }
}


void NQRNetworkManager_Lan_WiiU::RemoveAllPlayersOnClient()
{
    EnterCriticalSection(&m_playerMutex);
    std::vector<NQRNetworkPlayer*> removed = m_players;
    m_players.clear();
    LeaveCriticalSection(&m_playerMutex);
    for (std::vector<NQRNetworkPlayer*>::iterator player = removed.begin(); player != removed.end(); ++player)
    {
        if (m_listener != NULL)
            m_listener->HandlePlayerLeaving(*player);
        delete *player;
    }
}

void NQRNetworkManager_Lan_WiiU::ResetToIdle()
{
    m_host = false;
    m_receiving = false;
    m_matchmaking = false;
    m_leaveRequested = false;
    m_createRequested = false;
    m_joinRequested = false;
    m_browseRequested = false;
    m_randomJoinRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_None);
    SetState(m_online ? eNQRNetworkManagerInternalState_IdleOnline : eNQRNetworkManagerInternalState_Idle);
}

void NQRNetworkManager_Lan_WiiU::TickNexAsyncProcess()
{
    checkPiaCoreToUse();
    if (m_nexAsyncProcess != eNEXAsyncProcess_None)
    {
        SetNexAsyncProcess(eNEXAsyncProcess_None);
        return;
    }
    if (m_createRequested)
        Pia_CreateSession();
    else if (m_joinRequested)
    {
        if (canRunRateControlledEvent())
            Pia_JoinSession();
    }
    else if (m_leaveRequested)
        Pia_LeaveSession();
    else if (m_browseRequested)
    {
        if (canRunRateControlledEvent())
        {
            m_browseRequested = false;
            return;
        }
    }
    else if (m_randomJoinRequested)
    {
        if (canRunRateControlledEvent())
        {
            m_randomJoinRequested = false;
            return;
        }
    }
    else if (m_updateRequested)
        UpdateExternalRoomDataFromThread();
}

void NQRNetworkManager_Lan_WiiU::SendJoinData()
{
    ClientJoinData joinData;
    joinData.playerMask = m_roomPlayerMask;
    PlayerUID* playerUIDs[4] = { &joinData.playerUID0, &joinData.playerUID1, &joinData.playerUID2, &joinData.playerUID3 };
    for (int index = 0; index < 4; ++index)
    {
        *playerUIDs[index] = m_roomPlayers[index];
        std::strncpy(joinData.onlineIds[index], m_roomPlayers[index].m_szNetworkName, sizeof(joinData.onlineIds[index]));
        std::wcsncpy(joinData.displayNames[index], m_roomPlayers[index].m_szMiiName, sizeof(joinData.displayNames[index]) / sizeof(wchar_t));
    }
    RemoteDataSend(0, 1, &joinData, sizeof(joinData));
}

void NQRNetworkManager_Lan_WiiU::SendSaveTransferJoinData()
{
    wchar_t displayName[18];
    std::memset(displayName, 0, sizeof(displayName));
    NQRNetworkPlayer* player = GetLocalPlayerByUserIndex(0);
    if (player != NULL)
        std::wcsncpy(displayName, player->GetMiiName(), 17);
    RemoteDataSend(0, 200, displayName, sizeof(displayName));
}

void NQRNetworkManager_Lan_WiiU::NetworkPlayerConnectionComplete(NQRNetworkPlayer* player)
{
    if (player != NULL)
    {
        player->ConnectionComplete();
        if (player->IsReady())
            HandlePlayerJoined(player);
    }
}

void NQRNetworkManager_Lan_WiiU::PollUnconnectedStations()
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_unconnectedPlayers.begin(); player != m_unconnectedPlayers.end();)
    {
        if ((*player)->IsConnectionComplete())
        {
            NetworkPlayerConnectionComplete(*player);
            player = m_unconnectedPlayers.erase(player);
        }
        else
            ++player;
    }
}

bool NQRNetworkManager_Lan_WiiU::AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData, EJoinFailedReason* reason)
{
    if (!SessionHasSpace(1))
    {
        if (reason != NULL)
            *reason = eJoinFailedReason_Full;
        return false;
    }
    AddConnectedStation(stationId);
    PlayerUID* playerUIDs[4] = { &joinData.playerUID0, &joinData.playerUID1, &joinData.playerUID2, &joinData.playerUID3 };
    for (int index = 0; index < 4; ++index)
        if ((joinData.playerMask & (1u << index)) != 0)
        {
            PlayerSyncData syncData;
            static_cast<PlayerUID&>(syncData) = *playerUIDs[index];
            std::strncpy(syncData.m_szNetworkName, joinData.onlineIds[index], sizeof(syncData.m_szNetworkName));
            std::wcsncpy(syncData.m_szMiiName, joinData.displayNames[index], sizeof(syncData.m_szMiiName) / sizeof(wchar_t));
            FindOrCreateNonNetworkPlayer(NQRNetworkPlayer::eNQRNetworkPlayerType_Remote, index, &syncData);
        }
    SyncRoomData();
    if (reason != NULL)
        *reason = eJoinFailedReason_None;
    return true;
}

void NQRNetworkManager_Lan_WiiU::SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason)
{
    unsigned int packet[2];
    packet[0] = playerMask;
    packet[1] = static_cast<unsigned int>(reason);
    RemoteDataSend(stationId, 2, packet, sizeof(packet));
}

void NQRNetworkManager_Lan_WiiU::SendSaveTransferAckPacket()
{
    RemoteDataSend(0, 202, NULL, 0);
}

void NQRNetworkManager_Lan_WiiU::PollReceiveData()
{
    if (!m_receiving)
        return;
    PollUnconnectedStations();
}

void NQRNetworkManager_Lan_WiiU::SendSaveTransferDataPacket(void* data, int size)
{
    RemoteDataSend(0, 201, data, static_cast<unsigned int>(size));
}

void NQRNetworkManager_Lan_WiiU::TickSaveTransferData()
{
    if (m_saveTransferState == 4 && !m_saveTransferPacketPending)
    {
        unsigned int remaining = m_saveTransferSize - m_saveTransferOffset;
        unsigned int packetSize = remaining > 0x4000 ? 0x4000 : remaining;
        SendSaveTransferDataPacket(m_saveTransferData + m_saveTransferOffset, static_cast<int>(packetSize));
        m_saveTransferPacketPending = true;
        m_saveTransferOffset += packetSize;
    }
    if ((m_saveTransferState == 4 || m_saveTransferState == 12) && m_connectedStations.empty())
        m_saveTransferState = 14;
}

void NQRNetworkManager_Lan_WiiU::ShowErrorViewer()
{
    if (s_networkManager != NULL)
        s_networkManager->ErrorHandlingTick();
}

void NQRNetworkManager_Lan_WiiU::RemoveLocalClientPlayers(int userIndex)
{
    EnterCriticalSection(&m_playerMutex);
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end();)
    {
        if ((*player)->IsLocal() && (*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(userIndex))
        {
            NQRNetworkPlayer* removed = *player;
            player = m_players.erase(player);
            if (m_listener != NULL)
                m_listener->HandlePlayerLeaving(removed);
            delete removed;
        }
        else
            ++player;
    }
    LeaveCriticalSection(&m_playerMutex);
}

void NQRNetworkManager_Lan_WiiU::ErrorHandlingTick()
{
    if (m_lastNetworkError != 0 && m_listener != NULL)
        m_listener->HandleDisconnect(true);
}


void NQRNetworkManager_Lan_WiiU::TickFromThread()
{
    OnlineCheck();
    UpdateApplicationConnected();
    TickNexAsyncProcess();
    PollReceiveData();
    TickSendQueue();
    FriendSearchTick();
    ErrorHandlingTick();
}

void NQRNetworkManager_Lan_WiiU::RunTickThread()
{
    while (m_running)
    {
        if (m_tickEvent != NULL)
            m_tickEvent->WaitForSignal(100);
        TickFromThread();
    }
}

int NQRNetworkManager_Lan_WiiU::TickThreadProc(void* context)
{
    NQRNetworkManager_Lan_WiiU* manager = static_cast<NQRNetworkManager_Lan_WiiU*>(context);
    if (manager != NULL)
        manager->RunTickThread();
    return 0;
}

void NQRNetworkManager_Lan_WiiU::MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo)
{
    if (extData == NULL)
        return;
    *extData = NULL;
    if (presenceInfo != NULL)
    {
        unsigned char* data = new unsigned char[sizeof(PresenceSyncInfo)];
        std::memcpy(data, presenceInfo, sizeof(PresenceSyncInfo));
        *extData = data;
    }
}



void NQRNetworkManager_Lan_WiiU::Initialise()
{
    if (m_initialised)
        return;
    m_tickEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_tickThread = new C4JThreadImpl(TickThreadProc, this, "NQRNetworkManager", 0x10000);
    m_running = true;
    m_tickThread->Run();
    SetState(eNQRNetworkManagerInternalState_Initialising);
}

void NQRNetworkManager_Lan_WiiU::Terminate()
{
    if (m_running)
    {
        m_running = false;
        if (m_tickEvent != NULL)
            m_tickEvent->Set();
        if (m_tickThread != NULL)
            m_tickThread->WaitForCompletion(-1);
    }
    delete m_tickThread;
    m_tickThread = NULL;
    delete m_tickEvent;
    m_tickEvent = NULL;
    Pia_Shutdown();
    SetState(eNQRNetworkManagerInternalState_Uninitialised);
}

NQRNetworkManager::eNQRNetworkManagerState NQRNetworkManager_Lan_WiiU::GetState()
{
    return m_state;
}

bool NQRNetworkManager_Lan_WiiU::IsHost()
{
    return m_host;
}

bool NQRNetworkManager_Lan_WiiU::IsReadyToPlayOrIdle()
{
    return m_localPlayerRequests.empty() &&
        (m_internalState == eNQRNetworkManagerInternalState_HostRoomReady ||
         m_internalState == eNQRNetworkManagerInternalState_Playing ||
         m_internalState == eNQRNetworkManagerInternalState_Idle);
}

bool NQRNetworkManager_Lan_WiiU::IsInSession()
{
    return m_receiving;
}

bool NQRNetworkManager_Lan_WiiU::IsMatchmakingEnabled()
{
    return m_matchmaking;
}

void NQRNetworkManager_Lan_WiiU::CreateAndJoinRoom(int userIndexMask, int maxPlayers, void* extData, int extDataSize, int*, int, bool matchmaking, unsigned char gameMode)
{
    m_pendingUserMask = userIndexMask;
    m_maxPlayers = maxPlayers;
    m_matchmaking = matchmaking;
    m_gameMode = gameMode;
    if (extData != NULL && extDataSize >= static_cast<int>(sizeof(PresenceSyncInfo)))
        SetNQRPresenceInfoFromExtData(&m_presenceInfo, extData);
    UpdateRichPresenceCustomData(&m_presenceInfo);
    m_host = true;
    SetState(eNQRNetworkManagerInternalState_HostCreatingSession);
    if (m_online)
        Pia_SignalCreateSession();
    else
        SetLocalPlayersAndSync();
}

void NQRNetworkManager_Lan_WiiU::UpdateExternalRoomData()
{
    m_updateRequested = true;
}

bool NQRNetworkManager_Lan_WiiU::FriendRoomManagerIsBusy()
{
    return m_friendSearchBusy;
}

bool NQRNetworkManager_Lan_WiiU::FriendRoomManagerSearch()
{
    EnterCriticalSection(&m_friendSearchMutex);
    if (m_internalState != eNQRNetworkManagerInternalState_Idle || m_friendSearchBusy)
    {
        LeaveCriticalSection(&m_friendSearchMutex);
        return false;
    }
    m_friendSearchResults.clear();
    m_friendPresenceUpdated = false;
    m_friendPresenceCount = 0;
    m_friendSearchBusy = true;
    LeaveCriticalSection(&m_friendSearchMutex);
    GetFriendsFromSessionBrowse();
    return true;
}

bool NQRNetworkManager_Lan_WiiU::FriendRoomManagerSearch2()
{
    EnterCriticalSection(&m_friendSearchMutex);
    if (m_friendPresenceCount == 0)
    {
        m_friendSearchBusy = false;
        LeaveCriticalSection(&m_friendSearchMutex);
        return false;
    }
    m_friendSearchBusy = false;
    LeaveCriticalSection(&m_friendSearchMutex);
    return true;
}

int NQRNetworkManager_Lan_WiiU::FriendRoomManagerGetCount()
{
    EnterCriticalSection(&m_friendSearchMutex);
    int count = static_cast<int>(m_friendSearchResults.size());
    LeaveCriticalSection(&m_friendSearchMutex);
    return count;
}

void NQRNetworkManager_Lan_WiiU::FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result)
{
    if (result == NULL)
        return;
    EnterCriticalSection(&m_friendSearchMutex);
    if (index >= 0 && index < static_cast<int>(m_friendSearchResults.size()))
    {
        FriendSearchResult& source = m_friendSearchResults[index];
        result->m_uiSessionId = source.sessionId;
        result->m_uiOwnerPrincipalId = source.ownerPrincipalId;
        result->m_uiOwnerLocalFriendIndex = source.ownerLocalFriendIndex;
        std::memcpy(result->m_szOwnerName, source.ownerName, sizeof(result->m_szOwnerName));
        result->m_pExtData = source.extData;
    }
    LeaveCriticalSection(&m_friendSearchMutex);
}

NQRNetworkManager::SessionSearchResult* NQRNetworkManager_Lan_WiiU::GetFriendRoomSearchResults(int* count)
{
    int resultCount = FriendRoomManagerGetCount();
    SessionSearchResult* results = resultCount == 0 ? NULL : new SessionSearchResult[resultCount];
    for (int index = 0; index < resultCount; ++index)
        FriendRoomManagerGetRoomInfo(index, &results[index]);
    if (count != NULL)
        *count = resultCount;
    m_friendPresenceUpdated = false;
    return results;
}

bool NQRNetworkManager_Lan_WiiU::JoinRoom(SessionSearchResult* result, int userIndexMask)
{
    if (result == NULL)
        return false;
    PresenceSyncInfo presence;
    SetNQRPresenceInfoFromExtData(&presence, result->m_pExtData);
    return JoinRoom(result->m_uiSessionId, userIndexMask, &presence);
}

bool NQRNetworkManager_Lan_WiiU::JoinRoom(unsigned int sessionId, int userIndexMask, const PresenceSyncInfo* presenceInfo)
{
    m_pendingSessionId = sessionId;
    m_pendingUserMask = userIndexMask;
    if (presenceInfo != NULL)
        m_presenceInfo = *presenceInfo;
    m_host = false;
    SetState(eNQRNetworkManagerInternalState_JoinRequested);
    Pia_SignalJoinSession();
    return true;
}

bool NQRNetworkManager_Lan_WiiU::JoinRoom(int userIndexMask, const nn::fp::FriendPresence*)
{
    return JoinRoom(0, userIndexMask, NULL);
}

bool NQRNetworkManager_Lan_WiiU::FriendPresenceUpdatedRoomList()
{
    return m_friendPresenceUpdated;
}

void NQRNetworkManager_Lan_WiiU::GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*), void* callbackData)
{
    bool found = false;
    EnterCriticalSection(&m_friendSearchMutex);
    for (std::vector<FriendSearchResult>::iterator result = m_friendSearchResults.begin(); result != m_friendSearchResults.end(); ++result)
        if (result->sessionId == sessionId)
        {
            if (extData != NULL && result->extData != NULL)
                std::memcpy(extData, result->extData, sizeof(PresenceSyncInfo));
            found = true;
            break;
        }
    LeaveCriticalSection(&m_friendSearchMutex);
    if (callback != NULL)
        callback(found, callbackData);
}

bool NQRNetworkManager_Lan_WiiU::FindAndJoinRoom(int userIndexMask, unsigned int searchFlags, CMatchMakeSearchParam*, unsigned char gameMode)
{
    m_pendingUserMask = userIndexMask;
    m_gameMode = gameMode;
    m_matchmaking = true;
    m_browseRequested = (searchFlags & 15) != 0;
    SetState(eNQRNetworkManagerInternalState_JoinSearchingMatchmakeSession);
    return true;
}



void NQRNetworkManager_Lan_WiiU::GetFriendsThreadProc(void* context)
{
    NQRNetworkManager_Lan_WiiU* manager = static_cast<NQRNetworkManager_Lan_WiiU*>(context);
    if (manager != NULL)
        manager->GetFriendsFromSessionBrowse();
}

bool NQRNetworkManager_Lan_WiiU::Session_BrowseSession(nn::pia::session::SessionSearchCriteria*)
{
    m_browseRequested = true;
    SetNexAsyncProcess(eNEXAsyncProcess_BrowseSession);
    return true;
}

unsigned int NQRNetworkManager_Lan_WiiU::GetFriendsFromSessionBrowse()
{
    EnterCriticalSection(&m_friendSearchMutex);
    m_friendPresenceCount = static_cast<unsigned int>(m_friendSearchResults.size());
    m_friendPresenceUpdated = true;
    LeaveCriticalSection(&m_friendSearchMutex);
    return m_friendPresenceCount;
}

void NQRNetworkManager_Lan_WiiU::StartGame()
{
    SetState(eNQRNetworkManagerInternalState_StartingGame);
    SetState(eNQRNetworkManagerInternalState_Playing);
}

void NQRNetworkManager_Lan_WiiU::ClearRichPresenceCustomData()
{
    delete[] static_cast<unsigned char*>(m_externalData);
    m_externalData = NULL;
    m_externalDataSize = 0;
}

void NQRNetworkManager_Lan_WiiU::LeaveRoom(bool error)
{
    if (error)
        m_lastNetworkError = 1;
    SetState(eNQRNetworkManagerInternalState_LeavingRoom);
    SendLeaveData(m_roomPlayerMask);
    Pia_SignalLeaveSession();
}

void NQRNetworkManager_Lan_WiiU::EndGame()
{
}

bool NQRNetworkManager_Lan_WiiU::SessionHasSpace(int extraPlayers)
{
    return GetPlayerCount() + extraPlayers <= m_maxPlayers;
}

bool NQRNetworkManager_Lan_WiiU::SessionHasMii(int userIndex)
{
    NQRNetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
    return player != NULL && player->GetMiiName()[0] != 0;
}

void NQRNetworkManager_Lan_WiiU::UpdateRemotePlay()
{
    m_presenceInfo.m_ucPlayerCount = static_cast<unsigned char>(GetPlayerCount());
    m_presenceInfo.m_ucMaxPlayers = static_cast<unsigned char>(m_maxPlayers);
    m_presenceInfo.m_bJoinable = SessionHasSpace(1);
}

bool NQRNetworkManager_Lan_WiiU::AddLocalPlayerByUserIndex(int userIndex)
{
    if (!SessionHasSpace(1) || GetLocalPlayerByUserIndex(userIndex) != NULL)
        return false;
    PlayerUID uid(0);
    NQRNetworkPlayer::eNQRNetworkPlayerType type = m_players.empty() ? NQRNetworkPlayer::eNQRNetworkPlayerType_Host : NQRNetworkPlayer::eNQRNetworkPlayerType_Local;
    NQRNetworkPlayer* player = new NQRNetworkPlayer(this, type, true, 0, userIndex, &uid, "", L"");
    AddConnectedPlayer(player);
    NetworkPlayerConnectionComplete(player);
    NetworkPlayerSmallIdAllocated(player, static_cast<unsigned char>(++m_nextSmallId));
    MapRoomSlotPlayers(-1);
    SyncRoomData();
    UpdateRemotePlay();
    return true;
}

void NQRNetworkManager_Lan_WiiU::SendLeaveData(unsigned int playerMask)
{
    RemoteDataSend(0, 3, &playerMask, sizeof(playerMask));
}

bool NQRNetworkManager_Lan_WiiU::RemoveLocalPlayerByUserIndex(int userIndex)
{
    NQRNetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
    if (player == NULL)
        return false;
    SendLeaveData(1u << userIndex);
    RemoveConnectedPlayer(player);
    delete player;
    MapRoomSlotPlayers(-1);
    SyncRoomData();
    UpdateRemotePlay();
    return true;
}


int NQRNetworkManager_Lan_WiiU::GetPlayerCount()
{
    return static_cast<int>((m_roomSessionData >> 16 & 0xf) + 1);
}

int NQRNetworkManager_Lan_WiiU::GetOnlinePlayerCount()
{
    int count = 0;
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if (!(*player)->IsLocal())
            ++count;
    return count;
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetPlayerIfReady(NQRNetworkPlayer* player)
{
    return player != NULL && player->IsReady() ? player : NULL;
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetPlayerByIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_players.size()))
        return NULL;
    return GetPlayerIfReady(m_players[index]);
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetPlayerBySmallId(int smallId)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetSmallId() == static_cast<unsigned char>(smallId))
            return GetPlayerIfReady(*player);
    return NULL;
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetPlayerByXuid(PlayerUID uid)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetUID() == uid)
            return GetPlayerIfReady(*player);
    return NULL;
}

std::wstring NQRNetworkManager_Lan_WiiU::GetDisplayNameByGamertag(std::wstring gamertag)
{
    PlayerNameMap::iterator name = m_miiNames.find(gamertag);
    return name == m_miiNames.end() ? gamertag : name->second;
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetLocalPlayerByUserIndex(int userIndex)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->IsLocal() && (*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(userIndex))
            return GetPlayerIfReady(*player);
    return NULL;
}

NQRNetworkPlayer* NQRNetworkManager_Lan_WiiU::GetHostPlayer()
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->IsHost())
            return GetPlayerIfReady(*player);
    return NULL;
}


void NQRNetworkManager_Lan_WiiU::LocalDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size)
{
    if (m_listener != NULL && from != NULL && to != NULL && data != NULL)
        m_listener->HandleDataReceived(from, to, static_cast<unsigned char*>(const_cast<void*>(data)), size);
}

void NQRNetworkManager_Lan_WiiU::RemoteDataSend(NQRNetworkPlayer*, NQRNetworkPlayer* to, const void* data, unsigned int size)
{
    if (to != NULL)
        RemoteDataSend(static_cast<unsigned long long>(to->GetSessionIndex()), 0, data, size);
}

int NQRNetworkManager_Lan_WiiU::GetSessionIndex(NQRNetworkPlayer* player)
{
    for (unsigned int index = 0; index < m_players.size(); ++index)
        if (m_players[index] == player)
            return static_cast<int>(index);
    return -1;
}

void NQRNetworkManager_Lan_WiiU::SetPresenceDataStartHostingGame()
{
    m_presenceInfo.m_ucGameMode = m_gameMode;
    m_presenceInfo.m_ucPlayerCount = static_cast<unsigned char>(GetPlayerCount());
    m_presenceInfo.m_ucMaxPlayers = static_cast<unsigned char>(m_maxPlayers);
    m_presenceInfo.m_bJoinable = SessionHasSpace(1);
    UpdateRichPresenceCustomData(&m_presenceInfo);
}

int NQRNetworkManager_Lan_WiiU::GetJoiningReadyPercentage()
{
    if (m_internalState == eNQRNetworkManagerInternalState_HostCreatingSession ||
        m_internalState == eNQRNetworkManagerInternalState_JoinCreatingSession)
        return 50;
    return 100;
}

void NQRNetworkManager_Lan_WiiU::SignalInitialiseAfterOnline()
{
    s_initialiseAfterOnline = true;
}

unsigned int NQRNetworkManager_Lan_WiiU::GetSaveTransferConnectionFromSessionBrowse()
{
    return m_saveTransferSessionId;
}

void NQRNetworkManager_Lan_WiiU::GetSaveTransferConnectionThreadProc(void* context)
{
    NQRNetworkManager_Lan_WiiU* manager = static_cast<NQRNetworkManager_Lan_WiiU*>(context);
    if (manager != NULL)
        manager->GetSaveTransferConnectionFromSessionBrowse();
}

void NQRNetworkManager_Lan_WiiU::Pia_ShutdownForOnline()
{
    m_leaveRequested = true;
    if (m_initialised)
    {
        m_browseRequested = true;
        m_randomJoinRequested = true;
    }
}

void NQRNetworkManager_Lan_WiiU::CreateSessionForSaveTransfer(int, void* data, int dataSize, void*, long long, unsigned int worldId, unsigned int titleVersion, wchar_t*)
{
    delete[] m_saveTransferData;
    m_saveTransferData = dataSize > 0 ? new unsigned char[dataSize] : NULL;
    if (m_saveTransferData != NULL && data != NULL)
        std::memcpy(m_saveTransferData, data, dataSize);
    m_saveTransferSize = static_cast<unsigned int>(dataSize);
    m_saveTransferOffset = 0;
    m_saveTransferPacketPending = false;
    m_presenceInfo.m_uiWorldId = worldId;
    m_presenceInfo.m_usTitleVersion = static_cast<unsigned short>(titleVersion);
    m_saveTransferState = 1;
    Pia_SignalCreateSession();
}

unsigned int NQRNetworkManager_Lan_WiiU::FindSessionForSaveTransfer()
{
    SetFriendSearchState(eNQRNetworkManagerFriendSearchState_Searching);
    m_saveTransferState = 9;
    return GetFriendsFromSessionBrowse();
}

void NQRNetworkManager_Lan_WiiU::JoinSaveTransferConnection()
{
    m_saveTransferState = 10;
    JoinRoom(m_saveTransferSessionId, 0, NULL);
    m_saveTransferOffset = 0;
}

void NQRNetworkManager_Lan_WiiU::TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData, bool showProgress)
{
}

bool NQRNetworkManager_Lan_WiiU::isAdhocConnected()
{
    return IsInitialisedAfterOnline();
}

bool NQRNetworkManager_Lan_WiiU::connectToAdhoc()
{
    if (!IsInitialisedAfterOnline() && s_initialiseAfterOnline)
        InitialiseAfterOnline();
    return IsInitialisedAfterOnline();
}

bool NQRNetworkManager_Lan_WiiU::isLanConnected()
{
    return IsInitialisedAfterOnline();
}

void NQRNetworkManager_Lan_WiiU::SendInviteGUI()
{
}
