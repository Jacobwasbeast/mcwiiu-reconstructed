#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"

#include <algorithm>
#include <cstddef>
#include <cstring>
#include <cwchar>

#include "game/core/System.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"

static NQRNetworkManager_WiiU* s_networkManager;
static int (*s_connectCallback)(void*, bool, int);
static void* s_connectCallbackData;
static bool s_cancelConnect;
static bool s_appSuspended;
static bool s_handlingBootInvite;

namespace nn
{
namespace nex
{
class ConnectivityManager
{
public:
    static ConnectivityManager* GetInstanceWithoutNetworkCheck();
    void OnStartNATSession(void* context);
};

class MatchmakeExtensionClient
{
public:
    MatchmakeExtensionClient();
    int Bind(void* credentials);
    void Unbind();
};

class BackEndServices
{
public:
    void RegisterNotificationEventHandler(void* handler);
    void UnregisterNotificationEventHandler(void* handler);
};

class NgsFacade
{
public:
    BackEndServices* GetBackEndServices();
    void* GetCredentials();
};

class EventLog
{
public:
    static EventLog* GetInstance();
    void SetLogLevel(int level);
};

class TraceLog
{
public:
    static TraceLog* GetInstance();
};

class GlobalVariables
{
public:
    static void AcquireInstance();
    static void ReleaseInstance();
};
}
}

namespace nn
{
namespace pia
{
namespace session
{
class Session;
}
}
}

extern void checkPiaCoreToUse();
extern void InitializeRuntimeMemory(unsigned int runtimeSize, unsigned int reservedSize);
extern void FinalizeRuntimeMemory();
extern nn::nex::NgsFacade* Nex_GetNgsFacadeInstanceNative();
extern void* Nex_GetDataStoreClientNative();
extern int Nex_SearchObjectNative();
extern void Pia_StartupSessionNative(void* result, void* token, const char* name);

static void* s_dataStoreClient;
static nn::nex::MatchmakeExtensionClient* s_matchmakeClient;
static bool s_matchmakeHandlerRegistered;
static nn::nex::NgsFacade* s_ngsFacade;
static bool s_nexInitialised;
static unsigned int s_errorHandlingState;
static unsigned int s_errorHandlingResult;
static unsigned char s_matchmakeNotificationHandler;

unsigned int GetTickCount();

void NQRNetworkManager_WiiU_Pia_StartupSession(void* result, void* token, const char* name)
{
    Pia_StartupSessionNative(result, token, name);
}

unsigned int Nex_FinalizeDataStore__Fv()
{
    if (s_dataStoreClient != NULL)
    {
        s_dataStoreClient = NULL;
    }
    return 0;
}

void Nex_InitializeErrorHandling__Fv()
{
    s_errorHandlingState = 0;
    s_errorHandlingResult = 0;
}

unsigned int Nex_FinalizeMatchmake__Fv()
{
    if (s_matchmakeClient != NULL)
    {
        if (s_matchmakeHandlerRegistered && s_ngsFacade != NULL)
        {
            s_ngsFacade->GetBackEndServices()->UnregisterNotificationEventHandler(&s_matchmakeNotificationHandler);
            s_matchmakeHandlerRegistered = false;
        }
        s_matchmakeClient->Unbind();
        s_matchmakeClient = NULL;
    }
    return 0;
}

void Nex_FinalizeErrorHandling__Fv()
{
    s_errorHandlingState = 0;
    s_errorHandlingResult = 0;
}

void Nex_FinalizeNex__Fv()
{
    checkPiaCoreToUse();
    if (s_nexInitialised)
    {
        s_ngsFacade = NULL;
        nn::nex::GlobalVariables::ReleaseInstance();
        Nex_FinalizeErrorHandling__Fv();
        FinalizeRuntimeMemory();
        s_nexInitialised = false;
    }
}

void* Nex_GetMatchmakeClient__Fv()
{
    return s_matchmakeClient;
}

void* Nex_GetNgsFacadeInstance__Fv()
{
    return s_ngsFacade;
}

unsigned int Nex_InitializeMatchmake__Fv()
{
    s_matchmakeClient = new nn::nex::MatchmakeExtensionClient();
    s_ngsFacade = Nex_GetNgsFacadeInstanceNative();
    if (s_matchmakeClient == NULL || s_ngsFacade == NULL ||
        s_matchmakeClient->Bind(s_ngsFacade->GetCredentials()) == 0)
        return 0;
    s_ngsFacade->GetBackEndServices()->RegisterNotificationEventHandler(&s_matchmakeNotificationHandler);
    s_matchmakeHandlerRegistered = true;
    return 1;
}

unsigned int Nex_InitializeNex__FUiT1Q4_2nn3nex4Core10ThreadMode(unsigned int runtimeSize, unsigned int reservedSize, unsigned int)
{
    checkPiaCoreToUse();
    InitializeRuntimeMemory(runtimeSize, reservedSize);
    nn::nex::GlobalVariables::AcquireInstance();
    Nex_InitializeErrorHandling__Fv();
    s_ngsFacade = Nex_GetNgsFacadeInstanceNative();
    s_nexInitialised = true;
    return 0;
}

unsigned int Nex_SearchObject__Fv()
{
    return static_cast<unsigned int>(Nex_SearchObjectNative());
}

void StartNATSessionCallback__FPQ3_2nn3nex11CallContextPCQ3_2nn3nex11UserContext(void* context, const void* userContext)
{
    nn::nex::ConnectivityManager* manager = nn::nex::ConnectivityManager::GetInstanceWithoutNetworkCheck();
    if (userContext != NULL && *static_cast<void* const*>(userContext) == manager)
        manager->OnStartNATSession(context);
}

NQRNetworkManager_WiiU::SendQueueItem::SendQueueItem()
    : stationId(0), channel(0), data(NULL), size(0)
{
}

NQRNetworkManager_WiiU::SendQueueItem::SendQueueItem(unsigned long long station, unsigned char sendChannel, const void* sendData, unsigned int sendSize)
    : stationId(station), channel(sendChannel), data(new unsigned char[sendSize]), size(sendSize)
{
    std::memcpy(data, sendData, size);
}

NQRNetworkManager_WiiU::SendQueueItem::SendQueueItem(const SendQueueItem& other)
    : stationId(other.stationId), channel(other.channel), data(new unsigned char[other.size]), size(other.size)
{
    std::memcpy(data, other.data, size);
}

NQRNetworkManager_WiiU::SendQueueItem& NQRNetworkManager_WiiU::SendQueueItem::operator=(const SendQueueItem& other)
{
    if (this != &other)
    {
        unsigned char* replacement = new unsigned char[other.size];
        std::memcpy(replacement, other.data, other.size);
        delete[] data;
        stationId = other.stationId;
        channel = other.channel;
        data = replacement;
        size = other.size;
    }
    return *this;
}

NQRNetworkManager_WiiU::SendQueueItem::~SendQueueItem()
{
    delete[] data;
}

NQRNetworkManager_WiiU::FriendSearchResult::FriendSearchResult()
    : ownerPrincipalId(0), ownerLocalFriendIndex(0), sessionId(0), joinable(false), extData(NULL)
{
    std::memset(ownerName, 0, sizeof(ownerName));
    std::memset(&presence, 0, sizeof(presence));
    std::memset(status, 0, sizeof(status));
}

NQRNetworkManager_WiiU::ConnectedStation::ConnectedStation()
    : stationId(0), state(0), playerMask(0)
{
}

NQRNetworkManager_WiiU::ConnectedStation::ConnectedStation(unsigned long long station)
    : stationId(station), state(0), playerMask(0)
{
}

NQRNetworkManager_WiiU::ClientJoinData::ClientJoinData()
    : playerMask(0), playerUID0(0), playerUID1(0), playerUID2(0), playerUID3(0)
{
    std::memset(onlineIds, 0, sizeof(onlineIds));
    std::memset(displayNames, 0, sizeof(displayNames));
}

NQRNetworkManager_WiiU::NQRNetworkManager_WiiU(INQRNetworkManagerListener* listener)
    : m_friendPresenceUpdated(false), m_connectThread(NULL), m_tickEvent(NULL), m_tickThread(NULL),
      m_listener(listener), m_internalState(eNQRNetworkManagerInternalState_Uninitialised),
      m_state(eNQRNetworkManagerState_Uninitialised), m_initialised(false), m_host(false),
      m_receiving(false), m_searchCriteria(NULL), m_lastNetworkError(0), m_applicationConnected(false), m_online(false),
      m_sessionId(0), m_matchmaking(false), m_roomPlayerMask(0), m_roomSessionData(0), m_friendSearchBusy(false),
      m_friendPresenceCount(0), m_nextSmallId(0),
      m_nexContext(NULL), m_nexAsyncProcess(eNEXAsyncProcess_None), m_pendingSessionId(0),
      m_pendingUserMask(0), m_maxPlayers(8), m_gameMode(0), m_externalData(NULL),
      m_externalDataSize(0), m_running(false), m_leaveRequested(false), m_createRequested(false),
      m_joinRequested(false), m_browseRequested(false), m_randomJoinRequested(false),
      m_updateRequested(false), m_lastRateControlledEvent(0)
{
    std::memset(&m_presenceInfo, 0, sizeof(m_presenceInfo));
    InitializeCriticalSection(&m_playerMutex);
    InitializeCriticalSection(&m_sendQueueMutex);
    InitializeCriticalSection(&m_friendSearchMutex);
    InitializeCriticalSection(&m_connectedStationMutex);
    s_networkManager = this;
}

NQRNetworkManager_WiiU::~NQRNetworkManager_WiiU()
{
    Terminate();
    ClearSendQueue();
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        delete *player;
    m_players.clear();
    DeleteCriticalSection(&m_connectedStationMutex);
    DeleteCriticalSection(&m_friendSearchMutex);
    DeleteCriticalSection(&m_sendQueueMutex);
    DeleteCriticalSection(&m_playerMutex);
    if (s_networkManager == this)
        s_networkManager = NULL;
}

void NQRNetworkManager_WiiU::SetNexAsyncProcess(ENEXAsyncProcess process)
{
    m_nexAsyncProcess = process;
}

int NQRNetworkManager_WiiU::ConnectOnlineNintendo_Returned(void* context, bool connected, int error)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
    {
        manager->m_applicationConnected = connected;
        if (connected)
            manager->InitialiseAfterOnline();
        else
            manager->m_lastNetworkError = error;
    }
    if (s_connectCallback != NULL)
        return s_connectCallback(s_connectCallbackData, connected, error);
    return 0;
}

int NQRNetworkManager_WiiU::ConnectOnlineNintendoThreadProc(void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL && !s_cancelConnect)
        manager->TryToConnectOnline();
    return 0;
}

void NQRNetworkManager_WiiU::CancelConnectCallbacks()
{
    s_connectCallback = NULL;
    s_connectCallbackData = NULL;
    s_cancelConnect = false;
}

void NQRNetworkManager_WiiU::CancelConnectOnlineNintendo(void*)
{
    s_cancelConnect = true;
    s_connectCallback = NULL;
    s_connectCallbackData = NULL;
}

void NQRNetworkManager_WiiU::ShowConnectOnlineNintendoProgress(int, int (*callback)(void*, bool, int), void* callbackData)
{
    s_connectCallback = callback;
    s_connectCallbackData = callbackData;
    s_cancelConnect = false;
    if (s_networkManager != NULL)
        s_networkManager->TryToConnectOnline();
}

int NQRNetworkManager_WiiU::InvitationSignInReturned(void* context, bool signedIn, int error, int)
{
    if (signedIn)
        return ConnectOnlineNintendo_Returned(context, true, 0);
    return ConnectOnlineNintendo_Returned(context, false, error);
}

int NQRNetworkManager_WiiU::InvitationMustConnectReturned(void* context, int button, C4JStorage::EMessageResult)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL && button == 0)
        manager->TryToConnectOnline();
    else
        s_handlingBootInvite = false;
    return 0;
}

void NQRNetworkManager_WiiU::UpdateInvitationChecks()
{
    if (g_bSafeToRespondToGameBootInvite && !s_handlingBootInvite)
    {
        s_handlingBootInvite = true;
        g_bSafeToRespondToGameBootInvite = false;
        TryToConnectOnline();
    }
}

void NQRNetworkManager_WiiU::Tick()
{
    UpdateInvitationChecks();
    if (m_tickEvent != NULL)
        m_tickEvent->Set();
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(localIndex) && (*player)->GetSessionIndex() == static_cast<int>(stationId))
            return *player;
    return NULL;
}

void NQRNetworkManager_WiiU::RemoveConnectedStation(unsigned long long stationId)
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

void NQRNetworkManager_WiiU::RemoveConnectedPlayer(NQRNetworkPlayer* player)
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

void NQRNetworkManager_WiiU::NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId)
{
    NetworkPlayerSmallIdAllocated(player, smallId);
}

void NQRNetworkManager_WiiU::AddConnectedPlayer(NQRNetworkPlayer* player)
{
    if (player == NULL)
        return;
    EnterCriticalSection(&m_playerMutex);
    if (std::find(m_players.begin(), m_players.end(), player) == m_players.end())
        m_players.push_back(player);
    LeaveCriticalSection(&m_playerMutex);
    HandlePlayerJoined(player);
}

void NQRNetworkManager_WiiU::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (m_listener != NULL && player != NULL && player->IsReady())
        m_listener->HandlePlayerJoined(player);
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::FindOrCreateNonNetworkPlayer(int type, int localIndex, PlayerSyncData* syncData)
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

void NQRNetworkManager_WiiU::NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId)
{
    if (player != NULL)
    {
        player->SmallIdAllocated(smallId);
        HandlePlayerJoined(player);
    }
}

void NQRNetworkManager_WiiU::MapRoomSlotPlayers(int removedIndex)
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

void NQRNetworkManager_WiiU::UpdateRoomSyncUIDsFromPlayers()
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

void NQRNetworkManager_WiiU::RemoteDataSend(unsigned long long stationId, unsigned char channel, const void* data, unsigned int size)
{
    if (data == NULL || size == 0)
        return;
    EnterCriticalSection(&m_sendQueueMutex);
    m_sendQueue.push_back(SendQueueItem(stationId, channel, data, size));
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_WiiU::SyncRoomData()
{
    UpdateRoomSyncUIDsFromPlayers();
    m_updateRequested = true;
}

void NQRNetworkManager_WiiU::RemoveRemotePlayersAndSync(unsigned long long stationId, int localIndex)
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

void NQRNetworkManager_WiiU::ClearSendQueue()
{
    EnterCriticalSection(&m_sendQueueMutex);
    m_sendQueue.clear();
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_WiiU::SetState(eNQRNetworkManagerInternalState state)
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

void NQRNetworkManager_WiiU::TickSendQueue()
{
    EnterCriticalSection(&m_sendQueueMutex);
    if (!m_sendQueue.empty() && m_applicationConnected)
        m_sendQueue.pop_front();
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_WiiU::Pia_InitializeCommon(unsigned char*, unsigned int, unsigned long long stationId, bool host, bool online)
{
    m_host = host;
    m_online = online;
    m_applicationConnected = true;
    AddConnectedStation(stationId);
}

void NQRNetworkManager_WiiU::Pia_InitializeInet()
{
    m_online = true;
}

void NQRNetworkManager_WiiU::Pia_InitializeSession(const void*)
{
    m_applicationConnected = true;
}

NQRNetworkManager_WiiU::ConnectedStation* NQRNetworkManager_WiiU::GetConnectedStation(unsigned long long stationId)
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

void NQRNetworkManager_WiiU::AddConnectedStation(unsigned long long stationId)
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

void NQRNetworkManager_WiiU::Pia_SignalLeaveSession()
{
    m_leaveRequested = true;
}

void NQRNetworkManager_WiiU::Pia_SessionCallback(int, unsigned long long stationId, void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
        manager->AddConnectedStation(stationId);
}

bool NQRNetworkManager_WiiU::NexFacadeLogin(unsigned int principalId)
{
    m_pendingSessionId = principalId;
    SetNexAsyncProcess(eNEXAsyncProcess_Login);
    return true;
}

void NQRNetworkManager_WiiU::InitialiseAfterOnline()
{
    Pia_InitializeInet();
    m_initialised = true;
    SetState(eNQRNetworkManagerInternalState_IdleOnline);
}

void NQRNetworkManager_WiiU::OnlineCheck()
{
    if (m_applicationConnected && !m_initialised)
        InitialiseAfterOnline();
    else if (!m_applicationConnected && m_initialised)
        SetApplicationDisconnected();
}

void NQRNetworkManager_WiiU::FriendSearchTick()
{
    if (m_friendPresenceUpdated)
        UpdateFriendFromPresence(0);
}

void NQRNetworkManager_WiiU::FriendPresenceLoginCallback(unsigned int, void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
        manager->m_friendPresenceUpdated = true;
}

void NQRNetworkManager_WiiU::Session_CleanupSession()
{
    m_connectedStations.clear();
    m_sessionId = 0;
    m_receiving = false;
}

bool NQRNetworkManager_WiiU::Inet_Logout()
{
    SetNexAsyncProcess(eNEXAsyncProcess_Logout);
    return true;
}

void NQRNetworkManager_WiiU::Inet_FinalizeNetwork()
{
    m_online = false;
}

void NQRNetworkManager_WiiU::Session_FinalizePiaSession()
{
    Session_CleanupSession();
}

void NQRNetworkManager_WiiU::Inet_FinalizePiaInet()
{
    m_applicationConnected = false;
}

void NQRNetworkManager_WiiU::Pia_FinalizeCommon()
{
    m_initialised = false;
}

void NQRNetworkManager_WiiU::Pia_Finalize()
{
    Session_FinalizePiaSession();
    Inet_FinalizePiaInet();
    Inet_FinalizeNetwork();
    Pia_FinalizeCommon();
}

void NQRNetworkManager_WiiU::Pia_Shutdown()
{
    ClearSendQueue();
    RemoveAllPlayersOnClient();
    Pia_Finalize();
}

void NQRNetworkManager_WiiU::SetLastNetworkError(int result)
{
    m_lastNetworkError = result;
}

void NQRNetworkManager_WiiU::UpdateApplicationConnected()
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

bool NQRNetworkManager_WiiU::Pia_CreateSession()
{
    m_createRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_CreateSession);
    return true;
}

bool NQRNetworkManager_WiiU::canRunRateControlledEvent()
{
    unsigned int now = GetTickCount();
    if (now - m_lastRateControlledEvent < 1000)
        return false;
    m_lastRateControlledEvent = now;
    return true;
}

bool NQRNetworkManager_WiiU::Pia_JoinSession()
{
    m_joinRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_JoinSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_JoinMatchmakeSession()
{
    SetNexAsyncProcess(eNEXAsyncProcess_Search);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_LeaveSession()
{
    m_leaveRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_LeaveSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_BrowseSession()
{
    m_browseRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_BrowseSession);
    return true;
}

bool NQRNetworkManager_WiiU::Pia_JoinRandomSession()
{
    m_randomJoinRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_JoinRandomSession);
    return true;
}

void NQRNetworkManager_WiiU::UpdateExternalRoomDataFromThread()
{
    m_updateRequested = false;
    SetNexAsyncProcess(eNEXAsyncProcess_UpdateSession);
}

void NQRNetworkManager_WiiU::UpdateMatchmakingAttributes()
{
    SetNexAsyncProcess(eNEXAsyncProcess_Update);
}

bool NQRNetworkManager_WiiU::Pia_StartupSession()
{
    m_receiving = true;
    return true;
}

void NQRNetworkManager_WiiU::Pia_SignalCreateSession()
{
    m_createRequested = true;
}

void NQRNetworkManager_WiiU::Pia_SignalJoinSession()
{
    m_joinRequested = true;
}

void NQRNetworkManager_WiiU::SetLocalPlayersAndSync()
{
    MapRoomSlotPlayers(-1);
    SyncRoomData();
}

void NQRNetworkManager_WiiU::ClearIsHandlingBootInvite()
{
    s_handlingBootInvite = false;
}

void NQRNetworkManager_WiiU::SetNQRPresenceInfoFromExtData(PresenceSyncInfo* presenceInfo, void* extData)
{
    if (presenceInfo != NULL && extData != NULL)
        std::memcpy(presenceInfo, extData, sizeof(PresenceSyncInfo));
}

void NQRNetworkManager_WiiU::UpdateRichPresenceCustomData(void* extData, unsigned int extDataSize)
{
    delete[] static_cast<unsigned char*>(m_externalData);
    m_externalData = NULL;
    m_externalDataSize = 0;
    if (extData != NULL && extDataSize != 0)
    {
        unsigned char* copy = new unsigned char[extDataSize];
        std::memcpy(copy, extData, extDataSize);
        m_externalData = copy;
        m_externalDataSize = extDataSize;
    }
}

void NQRNetworkManager_WiiU::MatchmakingExtDataUpdatedCallback(bool success, void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
        manager->m_updateRequested = !success;
}

void NQRNetworkManager_WiiU::RemoveAllPlayersOnClient()
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

void NQRNetworkManager_WiiU::ResetToIdle()
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

void NQRNetworkManager_WiiU::TickNexAsyncProcess()
{
    if (m_nexAsyncProcess != eNEXAsyncProcess_None)
    {
        SetNexAsyncProcess(eNEXAsyncProcess_None);
        return;
    }
    if (m_createRequested)
        Pia_CreateSession();
    else if (m_joinRequested)
        Pia_JoinSession();
    else if (m_leaveRequested)
        Pia_LeaveSession();
    else if (m_browseRequested)
        Pia_BrowseSession();
    else if (m_randomJoinRequested)
        Pia_JoinRandomSession();
    else if (m_updateRequested)
        UpdateExternalRoomDataFromThread();
}

void NQRNetworkManager_WiiU::SendJoinData()
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

void NQRNetworkManager_WiiU::NetworkPlayerConnectionComplete(NQRNetworkPlayer* player)
{
    if (player != NULL)
    {
        player->ConnectionComplete();
        if (player->IsReady())
            HandlePlayerJoined(player);
    }
}

void NQRNetworkManager_WiiU::PollUnconnectedStations()
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

bool NQRNetworkManager_WiiU::AddRemotePlayersAndSync(unsigned long long stationId, ClientJoinData& joinData, EJoinFailedReason* reason)
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

void NQRNetworkManager_WiiU::__CPR95__SendJJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason)
{
    unsigned int packet[2];
    packet[0] = playerMask;
    packet[1] = static_cast<unsigned int>(reason);
    RemoteDataSend(stationId, 2, packet, sizeof(packet));
}

void NQRNetworkManager_WiiU::PollReceiveData()
{
    if (!m_receiving)
        return;
    PollUnconnectedStations();
}

void NQRNetworkManager_WiiU::ShowErrorViewer()
{
    if (s_networkManager != NULL)
        s_networkManager->ErrorHandlingTick();
}

void NQRNetworkManager_WiiU::RemoveLocalClientPlayers(int userIndex)
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

void NQRNetworkManager_WiiU::ErrorHandlingTick()
{
    if (m_lastNetworkError != 0 && m_listener != NULL)
        m_listener->HandleDisconnect(true);
}

void NQRNetworkManager_WiiU::TryToConnectOnline()
{
    if (!s_cancelConnect)
    {
        m_applicationConnected = true;
        ConnectOnlineNintendo_Returned(this, true, 0);
    }
}

void NQRNetworkManager_WiiU::TickFromThread()
{
    OnlineCheck();
    UpdateApplicationConnected();
    TickNexAsyncProcess();
    PollReceiveData();
    TickSendQueue();
    FriendSearchTick();
    ErrorHandlingTick();
}

void NQRNetworkManager_WiiU::RunTickThread()
{
    while (m_running)
    {
        if (m_tickEvent != NULL)
            m_tickEvent->WaitForSignal(100);
        TickFromThread();
    }
}

int NQRNetworkManager_WiiU::TickThreadProc(void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
        manager->RunTickThread();
    return 0;
}

void NQRNetworkManager_WiiU::MallocAndSetExtDataFromNQRPresenceInfo(void** extData, PresenceSyncInfo* presenceInfo)
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

void NQRNetworkManager_WiiU::UpdateFriendFromPresence(unsigned int friendIndex)
{
    EnterCriticalSection(&m_friendSearchMutex);
    if (friendIndex < m_friendSearchResults.size())
        m_friendSearchResults[friendIndex].ownerLocalFriendIndex = friendIndex;
    m_friendPresenceUpdated = true;
    LeaveCriticalSection(&m_friendSearchMutex);
}

void NQRNetworkManager_WiiU::UpdateFriendFromPresence_callback(unsigned int friendIndex, void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
        manager->UpdateFriendFromPresence(friendIndex);
}

void NQRNetworkManager_WiiU::Initialise()
{
    if (m_initialised)
        return;
    m_tickEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_tickThread = new C4JThreadImpl(TickThreadProc, this, "NQRNetworkManager", 0x10000);
    m_running = true;
    m_tickThread->Run();
    SetState(eNQRNetworkManagerInternalState_Initialising);
    TryToConnectOnline();
}

void NQRNetworkManager_WiiU::Terminate()
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

NQRNetworkManager::eNQRNetworkManagerState NQRNetworkManager_WiiU::GetState()
{
    return m_state;
}

bool NQRNetworkManager_WiiU::IsHost()
{
    return m_host;
}

bool NQRNetworkManager_WiiU::IsReadyToPlayOrIdle()
{
    return m_localPlayerRequests.empty() &&
        (m_internalState == eNQRNetworkManagerInternalState_HostRoomReady ||
         m_internalState == eNQRNetworkManagerInternalState_Playing ||
         m_internalState == eNQRNetworkManagerInternalState_Idle);
}

bool NQRNetworkManager_WiiU::IsInSession()
{
    return m_receiving;
}

bool NQRNetworkManager_WiiU::IsMatchmakingEnabled()
{
    return m_matchmaking;
}

void NQRNetworkManager_WiiU::CreateAndJoinRoom(int userIndexMask, int maxPlayers, void* extData, int extDataSize, int*, int, bool matchmaking, unsigned char gameMode)
{
    m_pendingUserMask = userIndexMask;
    m_maxPlayers = maxPlayers;
    m_matchmaking = matchmaking;
    m_gameMode = gameMode;
    UpdateRichPresenceCustomData(extData, static_cast<unsigned int>(extDataSize));
    m_host = true;
    SetState(eNQRNetworkManagerInternalState_HostCreatingSession);
    if (m_online)
        Pia_SignalCreateSession();
    else
        SetLocalPlayersAndSync();
}

void NQRNetworkManager_WiiU::UpdateExternalRoomData()
{
    m_updateRequested = true;
}

bool NQRNetworkManager_WiiU::FriendRoomManagerIsBusy()
{
    return m_friendSearchBusy;
}

bool NQRNetworkManager_WiiU::FriendRoomManagerSearch()
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
    GetFriendsFromPresence();
    return true;
}

bool NQRNetworkManager_WiiU::FriendRoomManagerSearch2()
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

int NQRNetworkManager_WiiU::FriendRoomManagerGetCount()
{
    EnterCriticalSection(&m_friendSearchMutex);
    int count = static_cast<int>(m_friendSearchResults.size());
    LeaveCriticalSection(&m_friendSearchMutex);
    return count;
}

void NQRNetworkManager_WiiU::FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result)
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

NQRNetworkManager::SessionSearchResult* NQRNetworkManager_WiiU::GetFriendRoomSearchResults(int* count)
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

bool NQRNetworkManager_WiiU::JoinRoom(SessionSearchResult* result, int userIndexMask)
{
    if (result == NULL)
        return false;
    PresenceSyncInfo presence;
    SetNQRPresenceInfoFromExtData(&presence, result->m_pExtData);
    return JoinRoom(result->m_uiSessionId, userIndexMask, &presence);
}

bool NQRNetworkManager_WiiU::JoinRoom(unsigned int sessionId, int userIndexMask, const PresenceSyncInfo* presenceInfo)
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

bool NQRNetworkManager_WiiU::JoinRoom(int userIndexMask, const nn::fp::FriendPresence*)
{
    return JoinRoom(0, userIndexMask, NULL);
}

bool NQRNetworkManager_WiiU::FriendPresenceUpdatedRoomList()
{
    return m_friendPresenceUpdated;
}

void NQRNetworkManager_WiiU::GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*), void* callbackData)
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

bool NQRNetworkManager_WiiU::FindAndJoinRoom(int userIndexMask, unsigned int searchFlags, CMatchMakeSearchParam*, unsigned char gameMode)
{
    m_pendingUserMask = userIndexMask;
    m_gameMode = gameMode;
    m_matchmaking = true;
    m_browseRequested = (searchFlags & 15) != 0;
    SetState(eNQRNetworkManagerInternalState_JoinSearchingMatchmakeSession);
    Pia_SignalBrowseSession();
    return true;
}

void NQRNetworkManager_WiiU::Pia_SignalBrowseSession()
{
    m_browseRequested = true;
}

unsigned int NQRNetworkManager_WiiU::GetFriendsFromPresence()
{
    EnterCriticalSection(&m_friendSearchMutex);
    m_friendPresenceCount = static_cast<unsigned int>(m_friendSearchResults.size());
    m_friendPresenceUpdated = false;
    LeaveCriticalSection(&m_friendSearchMutex);
    return 0;
}

void NQRNetworkManager_WiiU::GetFriendsThreadProc(void* context)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL)
        manager->GetFriendsFromPresence();
}

void NQRNetworkManager_WiiU::StartGame()
{
    SetState(eNQRNetworkManagerInternalState_StartingGame);
    SetState(eNQRNetworkManagerInternalState_Playing);
}

void NQRNetworkManager_WiiU::ClearRichPresenceCustomData()
{
    delete[] static_cast<unsigned char*>(m_externalData);
    m_externalData = NULL;
    m_externalDataSize = 0;
}

void NQRNetworkManager_WiiU::LeaveRoom(bool error)
{
    if (error)
        m_lastNetworkError = 1;
    SetState(eNQRNetworkManagerInternalState_LeavingRoom);
    SendLeaveData(m_roomPlayerMask);
    Pia_SignalLeaveSession();
}

void NQRNetworkManager_WiiU::EndGame()
{
}

bool NQRNetworkManager_WiiU::SessionHasSpace(int extraPlayers)
{
    return GetPlayerCount() + extraPlayers <= m_maxPlayers;
}

bool NQRNetworkManager_WiiU::SessionHasMii(int userIndex)
{
    NQRNetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
    return player != NULL && player->GetMiiName()[0] != 0;
}

void NQRNetworkManager_WiiU::UpdateRemotePlay()
{
    m_presenceInfo.m_ucPlayerCount = static_cast<unsigned char>(GetPlayerCount());
    m_presenceInfo.m_ucMaxPlayers = static_cast<unsigned char>(m_maxPlayers);
    m_presenceInfo.m_bJoinable = SessionHasSpace(1);
}

bool NQRNetworkManager_WiiU::AddLocalPlayerByUserIndex(int userIndex)
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

void NQRNetworkManager_WiiU::SendLeaveData(unsigned int playerMask)
{
    RemoteDataSend(0, 3, &playerMask, sizeof(playerMask));
}

bool NQRNetworkManager_WiiU::RemoveLocalPlayerByUserIndex(int userIndex)
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

bool NQRNetworkManager_WiiU::IsHandlingBootInvite()
{
    return s_handlingBootInvite;
}

int NQRNetworkManager_WiiU::GetPlayerCount()
{
    return static_cast<int>((m_roomSessionData >> 16 & 0xf) + 1);
}

int NQRNetworkManager_WiiU::GetOnlinePlayerCount()
{
    int count = 0;
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if (!(*player)->IsLocal())
            ++count;
    return count;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerIfReady(NQRNetworkPlayer* player)
{
    return player != NULL && player->IsReady() ? player : NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerByIndex(int index)
{
    if (index < 0 || index >= static_cast<int>(m_players.size()))
        return NULL;
    return GetPlayerIfReady(m_players[index]);
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerBySmallId(int smallId)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetSmallId() == static_cast<unsigned char>(smallId))
            return GetPlayerIfReady(*player);
    return NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerByXuid(PlayerUID uid)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetUID() == uid)
            return GetPlayerIfReady(*player);
    return NULL;
}

std::wstring NQRNetworkManager_WiiU::GetDisplayNameByGamertag(std::wstring gamertag)
{
    PlayerNameMap::iterator name = m_miiNames.find(gamertag);
    return name == m_miiNames.end() ? gamertag : name->second;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetLocalPlayerByUserIndex(int userIndex)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->IsLocal() && (*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(userIndex))
            return GetPlayerIfReady(*player);
    return NULL;
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetHostPlayer()
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->IsHost())
            return GetPlayerIfReady(*player);
    return NULL;
}

void NQRNetworkManager_WiiU::SetAppSuspended(bool suspended)
{
    s_appSuspended = suspended;
}

void NQRNetworkManager_WiiU::LocalDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size)
{
    if (m_listener != NULL && from != NULL && to != NULL && data != NULL)
        m_listener->HandleDataReceived(from, to, static_cast<unsigned char*>(const_cast<void*>(data)), size);
}

void NQRNetworkManager_WiiU::RemoteDataSend(NQRNetworkPlayer*, NQRNetworkPlayer* to, const void* data, unsigned int size)
{
    if (to != NULL)
        RemoteDataSend(static_cast<unsigned long long>(to->GetSessionIndex()), 0, data, size);
}

int NQRNetworkManager_WiiU::GetSessionIndex(NQRNetworkPlayer* player)
{
    for (unsigned int index = 0; index < m_players.size(); ++index)
        if (m_players[index] == player)
            return static_cast<int>(index);
    return -1;
}

void NQRNetworkManager_WiiU::SetPresenceDataStartHostingGame()
{
    m_presenceInfo.m_ucGameMode = m_gameMode;
    m_presenceInfo.m_ucPlayerCount = static_cast<unsigned char>(GetPlayerCount());
    m_presenceInfo.m_ucMaxPlayers = static_cast<unsigned char>(m_maxPlayers);
    m_presenceInfo.m_bJoinable = SessionHasSpace(1);
    UpdateRichPresenceCustomData(&m_presenceInfo, sizeof(m_presenceInfo));
}

int NQRNetworkManager_WiiU::GetJoiningReadyPercentage()
{
    if (m_internalState == eNQRNetworkManagerInternalState_HostCreatingSession ||
        m_internalState == eNQRNetworkManagerInternalState_JoinCreatingSession)
        return 50;
    return 100;
}

void NQRNetworkManager_WiiU::SetApplicationDisconnected()
{
    m_applicationConnected = false;
    m_online = false;
}

void NQRNetworkManager_WiiU::Pia_ShutdownForLan()
{
    m_applicationConnected = false;
    m_online = false;
    m_leaveRequested = true;
}

void NQRNetworkManager_WiiU::TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData, bool showProgress)
{
    s_connectCallback = callback;
    s_connectCallbackData = callbackData;
    s_cancelConnect = false;
    if (showProgress)
        ShowConnectOnlineNintendoProgress(0, callback, callbackData);
    else
        TryToConnectOnline();
}

bool NQRNetworkManager_WiiU::isAdhocConnected()
{
    return false;
}

bool NQRNetworkManager_WiiU::connectToAdhoc()
{
    return false;
}

bool NQRNetworkManager_WiiU::isLanConnected()
{
    return false;
}

void NQRNetworkManager_WiiU::SendInviteGUI()
{
}
