#include "platform/wiiu/NQRNetworkManager_WiiU.hpp"
#include "game/core/StringIds.hpp"


#include <algorithm>
#include <cstddef>
#include <cstring>
#include <cwchar>
#include <cstdlib>
#include <nn/fp/fp_cpp.h>

#include "game/core/System.hpp"
#include "game/core/Compression.hpp"
#include "game/core/IntCache.hpp"
#include "game/core/Minecraft.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/block/Block.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/SampleNotificationEventHandler.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"
#include "nn/nex/AnyObjectHolder.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/DataStoreClientTemplate.hpp"
#include "nn/nex/DataStoreSearchParam.hpp"
#include "nn/nex/DataStoreSearchResult.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Reconstruction.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/qResult.hpp"
#include "nn/nex/MatchmakeSession.hpp"
#include "nn/nex/MemoryManager.hpp"
#include "nn/pia/Common.hpp"
#include "nn/pia/common/CachedPrint.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/Trace.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/inet/Api.hpp"
#include "nn/pia/inet/NexFacade.hpp"
#include "nn/pia/inet/NexSessionStartupSetting.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/session/SessionSupport.hpp"
#include "nn/pia/transport/Station.hpp"
#include "platform/wiiu/Shared_Nex_Setup.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

static NQRNetworkManager_WiiU* s_networkManager;
static unsigned int s_connectedLocalPlayerMask;
static int (*s_connectCallback)(void*, bool, int);
static void* s_connectCallbackData;
static int (*s_connectProgressCallback)(void*, bool, int, int);
static void* s_connectProgressCallbackData;
static int s_connectProgressUserIndex;
static bool s_cancelConnect;
static bool s_appSuspended;
static bool s_handlingBootInvite;
static int s_lastLoginErrorCode;
static bool s_connectFailed;
static int s_tickThreadProcessor = 2;
static bool s_tickConnectionsOnNetworkThread = false;
static const int cTickThreadWaitTime = 32;
static const unsigned int s_nexDispatchTimeout = 0xFFFFFFFFu;
static const int s_loginErrorServiceUnavailable = 0xf9bbf;
static const int s_rateControlledEventInterval = 10000;
static const unsigned int s_nexGameServerId = 0x101D9D00u;
static const unsigned int cPresenceMatchmakeSystemType = 2;
static const wchar_t* const s_nexAccessKey =
#include "platform/wiiu/shaders/NexAccessKey.inc"
    ;
static const unsigned int cPiaCachedPrintBufferSize = 512 * 1024;
static const int cPiaSchedulerThreadPriority = 16;
static const unsigned int cPiaWorkMemorySize = 4 * 1024 * 1024;
static const unsigned long long cPiaTraceFlags = 0x4008123;

static unsigned char s_piaWorkMemory[cPiaWorkMemorySize];

bool NQRNetworkManager_WiiU::s_applicationConnected;

void checkPiaCoreToUse();

static nn::nex::DataStoreClient* s_dataStoreClient;
static nn::nex::MatchmakeExtensionClient* s_matchmakeClient;
static bool s_matchmakeHandlerRegistered;
static unsigned int s_lastPiaErrorCode;
static nn::pia::Result s_lastPiaResult = 0;
static bool s_friendPresenceLoginPending;

static SampleNotificationEventHandler s_matchmakeNotificationHandler;
static bool s_piaInitialised;
static bool s_piaShutdownPending;
static bool s_piaSessionStarted;

nn::pia::Result NQRNetworkManager_WiiU_Pia_StartupSession(
    nn::pia::transport::Station::IdentificationToken* token, char* name)
{
    nn::pia::inet::NexSessionStartupSetting setting;
    nn::pia::transport::Station::PlayerInfo playerInfo;

    setting.m_field00 = 10000;
    setting.m_field04 = 500;
    setting.m_identificationToken = token;
    setting.m_hostMigrationEnabled = false;
    setting.m_bandwidthCheckMeasurementTime = 200;
    setting.m_bandwidthCheckIpPacketSize = 0x400;
    setting.m_recentPlayRecordEnabled = false;

    playerInfo.m_language = 1;
    playerInfo.m_name.Copy(nn::pia::common::String(name));
    setting.m_playerInfo[0] = playerInfo;

    return nn::pia::session::g_piaSession->Startup(setting);
}

static unsigned int Nex_FinalizeDataStore()
{
    if (s_dataStoreClient != NULL)
    {
        s_dataStoreClient->Unbind();
        delete s_dataStoreClient;
        s_dataStoreClient = NULL;
    }
    return 0;
}

unsigned int Nex_FinalizeMatchmake()
{
    if (s_matchmakeClient != NULL)
    {
        if (s_matchmakeHandlerRegistered)
        {
            s_matchmakeClient->Unbind();
            s_matchmakeHandlerRegistered = false;
        }
        if (Nex_GetNgsFacadeInstance() != NULL)
            Nex_GetNgsFacadeInstance()->UnregisterNotificationEventHandler(&s_matchmakeNotificationHandler);
        delete s_matchmakeClient;
        s_matchmakeClient = NULL;
    }
    return 0;
}

void checkPiaCoreToUse()
{
}

void AssertPiaError(nn::pia::Result& result)
{
}

void* Nex_GetMatchmakeClient()
{
    return s_matchmakeClient;
}

unsigned int Nex_InitializeMatchmake()
{
    s_matchmakeClient = new nn::nex::MatchmakeExtensionClient();
    if (!s_matchmakeClient->Bind(Nex_GetNgsFacadeInstance()->GetCredentials()))
        return 0;
    Nex_GetNgsFacadeInstance()->RegisterNotificationEventHandler(&s_matchmakeNotificationHandler);
    s_matchmakeHandlerRegistered = true;
    return 1;
}

unsigned int Nex_SearchObject()
{
    nn::nex::ProtocolCallContext callContext(NULL);
    nn::nex::DataStoreSearchParam searchParam;
    searchParam.Reset();
    nn::nex::DataStoreSearchResult searchResult;
    nn::nex::qResult result;

    searchParam.m_uiSearchTarget = 10;
    s_dataStoreClient->SearchObject(&callContext, searchParam, &searchResult);
    callContext.Wait(0xFFFFFFFF);
    result = callContext.GetResult();
    if (!result)
        return 0;

    return 0;
}

void StartNatSessionCallback(nn::nex::CallContext* context, const void* userContext)
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
    : m_friendPresenceUpdated(false), m_tickThread(NULL), m_tickEvent(NULL), m_gatheringHolder(NULL),
      m_listener(listener), m_internalState(eNQRNetworkManagerInternalState_Uninitialised),
      m_state(eNQRNetworkManagerState_Uninitialised), m_initialised(false), m_host(false),
      m_localStationId(0), m_hostStationId(0), m_localPlayerCount(0), m_localPlayersJoined(0),
      m_nextSmallId(0), m_localGame(false), m_receiving(false), m_matchmaking(false),
      m_externalData(NULL), m_externalDataSize(0), m_roomAttributes(NULL), m_roomAttributeCount(0),
      m_pendingSessionId(0), m_pendingUserMask(0), m_friendSearchBusy(false),
      m_nexCallContext(NULL), m_nexAsyncProcess(eNEXAsyncProcess_None), m_nexLoginState(eNEXLoginState_Failed), m_createRequested(false),
      m_joinRequested(false), m_joinMatchmakeRequested(false), m_leaveRequested(false),
      m_browseRequested(false), m_randomJoinRequested(false), m_updateRequested(false),
      m_lastRateControlledEvent(0), m_matchmakingExtData(NULL), m_gameMode(0), m_lastNetworkError(0),
      m_online(false), m_sessionId(0), m_roomPlayerMask(0), m_maxPlayers(8),
      m_running(false), m_friendPresenceCount(0), m_nexContext(NULL)
{
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    std::memset(&m_presenceInfo, 0, sizeof(m_presenceInfo));
    std::memset(m_roomPlayers, 0, sizeof(PlayerSyncData) * 2);
    InitializeCriticalSection(&m_playerMutex);
    InitializeCriticalSection(&m_connectedPlayerMutex);
    InitializeCriticalSection(&m_playerConnectionMutex);
    InitializeCriticalSection(&m_sendQueueMutex);
    InitializeCriticalSection(&m_friendSearchMutex);
    InitializeCriticalSection(&m_connectedStationMutex);
    s_networkManager = this;
}

NQRNetworkManager_WiiU::~NQRNetworkManager_WiiU()
{
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
        s_applicationConnected = connected;
        if (connected)
            manager->InitialiseAfterOnline();
        else
            manager->m_lastNetworkError = error;
    }
    if (s_connectProgressCallback != NULL)
    {
        int (*callback)(void*, bool, int, int) = s_connectProgressCallback;
        s_connectProgressCallback = NULL;
        return callback(s_connectProgressCallbackData, connected, error, s_connectProgressUserIndex);
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
    s_connectProgressCallback = NULL;
    s_connectProgressCallbackData = NULL;
    s_cancelConnect = false;
}

void NQRNetworkManager_WiiU::CancelConnectOnlineNintendo(void*)
{
    s_cancelConnect = true;
    s_connectCallback = NULL;
    s_connectCallbackData = NULL;
    s_connectProgressCallback = NULL;
    s_connectProgressCallbackData = NULL;
}

void NQRNetworkManager_WiiU::ShowConnectOnlineNintendoProgress(int userIndex, int (*callback)(void*, bool, int, int), void* callbackData)
{
    s_connectProgressCallback = callback;
    s_connectProgressCallbackData = callbackData;
    s_connectProgressUserIndex = userIndex;
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

int NQRNetworkManager_WiiU::InvitationMustConnectReturned(void* context, int pad, C4JStorage::EMessageResult)
{
    NQRNetworkManager_WiiU* manager = static_cast<NQRNetworkManager_WiiU*>(context);
    if (manager != NULL && pad == 0)
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
    m_tickEvent->Set();
}

NQRNetworkPlayer* NQRNetworkManager_WiiU::GetPlayerFromStationIdAndLocalIdx(unsigned long long stationId, int localIndex)
{
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_players.begin(); player != m_players.end(); ++player)
        if ((*player)->GetStationId() == stationId &&
            (*player)->GetLocalPlayerIndex() == static_cast<unsigned int>(localIndex))
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
    EnterCriticalSection(&m_connectedPlayerMutex);
    const unsigned long long stationId = player->GetStationId();
    if (player->IsLocal())
        s_connectedLocalPlayerMask &= ~(1u << player->GetLocalPlayerIndex());
    std::vector<NQRNetworkPlayer*>::iterator position = std::find(m_players.begin(), m_players.end(), player);
    if (position != m_players.end())
    {
        *position = m_players.back();
        m_players.pop_back();
    }
    NintendoVoiceChat_WiiU::RemovePlayer(player);
    bool stationHasPlayers = false;
    for (std::vector<NQRNetworkPlayer*>::iterator connected = m_players.begin();
         connected != m_players.end(); ++connected)
    {
        if ((*connected)->GetStationId() == stationId)
        {
            stationHasPlayers = true;
            break;
        }
    }
    if (!stationHasPlayers)
        RemoveConnectedStation(stationId);
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

void NQRNetworkManager_WiiU::NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId)
{
    player->ConnectionComplete();
    player->SmallIdAllocated(smallId);
}

void NQRNetworkManager_WiiU::AddConnectedPlayer(NQRNetworkPlayer* player)
{
    EnterCriticalSection(&m_connectedPlayerMutex);
    m_players.push_back(player);
    if (player->IsLocal())
        s_connectedLocalPlayerMask |= 1u << player->GetLocalPlayerIndex();
    NintendoVoiceChat_WiiU::AddPlayer(player);
    if (player->IsLocal())
    {
        const std::wstring networkName(player->GetNetworkNameW());
        PlayerNameMap::iterator existing = m_miiNames.find(networkName);
        if (existing != m_miiNames.end())
            m_miiNames.erase(existing);
        m_miiNames.insert(std::make_pair(networkName, std::wstring(player->GetMiiName())));
    }
    LeaveCriticalSection(&m_connectedPlayerMutex);
}

void NQRNetworkManager_WiiU::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (m_listener != NULL)
        m_listener->HandlePlayerJoined(player);
    if (!m_host && player->IsLocal())
        ++m_localPlayersJoined;
}

void NQRNetworkManager_WiiU::FindOrCreateNonNetworkPlayer(int slotIndex, int type, PlayerSyncData* syncData)
{
    for (std::vector<NQRNetworkPlayer*>::iterator candidate = m_unconnectedPlayers.begin();
         candidate != m_unconnectedPlayers.end(); ++candidate)
    {
        NQRNetworkPlayer* player = *candidate;
        if (player->GetType() == type && player->GetLocalIndex() == syncData->m_uiLocalPlayerIndex &&
            (type != NQRNetworkPlayer::eNQRNetworkPlayerType_Remote ||
             player->GetStationId() == syncData->m_uiStationId))
        {
            m_unconnectedPlayers.erase(candidate);
            m_roomSlotPlayers[slotIndex] = player;
            return;
        }
    }

    PlayerUID localUID(0);
    PlayerUID* uid;
    if (type == NQRNetworkPlayer::eNQRNetworkPlayerType_Local ||
        (m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Host))
    {
        gProfile->GetXUID(syncData->m_uiLocalPlayerIndex, &localUID, true);
        if (localUID == PlayerUID(0))
            gProfile->GetXUID(syncData->m_uiLocalPlayerIndex, &localUID, false);
        uid = &localUID;
    }
    else
    {
        uid = syncData;
    }
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        if (m_roomPlayers[index] == *uid)
            uid->flags |= PlayerUID::FlagMiiPrincipal;

    NQRNetworkPlayer* player = new NQRNetworkPlayer(this,
        static_cast<NQRNetworkPlayer::eNQRNetworkPlayerType>(type), m_host,
        syncData->m_uiStationId, syncData->m_uiLocalPlayerIndex, uid,
        syncData->m_szNetworkName, syncData->m_szMiiName);
    NonNetworkPlayerComplete(player, syncData->m_uiSmallId);
    AddConnectedPlayer(player);
    m_roomSlotPlayers[slotIndex] = player;
    HandlePlayerJoined(player);
}

void NQRNetworkManager_WiiU::NetworkPlayerSmallIdAllocated(NQRNetworkPlayer* player, unsigned char smallId)
{
    EnterCriticalSection(&m_playerConnectionMutex);
    const bool wasReady = player->IsReady();
    player->SmallIdAllocated(smallId);
    const bool isReady = player->IsReady();
    LeaveCriticalSection(&m_playerConnectionMutex);
    if (!wasReady && isReady)
        HandlePlayerJoined(player);
}

void NQRNetworkManager_WiiU::MapRoomSlotPlayers(int previousPlayerCount)
{
    EnterCriticalSection(&m_playerMutex);
    const bool removingPlayer = previousPlayerCount != -1;
    if (!removingPlayer)
        previousPlayerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;

    for (int index = 0; index < 8; ++index)
    {
        NQRNetworkPlayer* player = m_roomSlotPlayers[index];
        if (player != NULL && (!m_host || player->IsHost() || player->IsLocal()))
        {
            m_unconnectedPlayers.push_back(player);
            m_roomSlotPlayers[index] = NULL;
        }
    }

    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        PlayerSyncData& record = m_roomPlayers[index];
        if (index == 0)
        {
            FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Host, &record);
            if (m_host)
                static_cast<PlayerUID&>(record) = m_roomSlotPlayers[index]->GetUID();
            else
                m_roomSlotPlayers[index]->SetUID(record);
        }
        else if (record.m_uiStationId == m_localStationId)
        {
            if (m_host || gProfile->IsSignedIn(record.m_uiLocalPlayerIndex))
            {
                FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Local, &record);
                if (m_host)
                    static_cast<PlayerUID&>(record) = m_roomSlotPlayers[index]->GetUID();
            }
        }
        else if (m_host)
        {
            EnterCriticalSection(&m_connectedPlayerMutex);
            NQRNetworkPlayer* player = GetPlayerFromStationIdAndLocalIdx(
                record.m_uiStationId, record.m_uiLocalPlayerIndex);
            m_roomSlotPlayers[index] = player;
            if (player != NULL)
                NetworkPlayerSmallIdAllocated(player, record.m_uiSmallId);
            LeaveCriticalSection(&m_connectedPlayerMutex);
        }
        else
        {
            FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Remote, &record);
            m_roomSlotPlayers[index]->SetUID(record);
        }
    }

    if (m_host)
    {
        if (removingPlayer && previousPlayerCount != 0)
            m_roomSlotPlayers[previousPlayerCount - 1] = NULL;
        if (m_listener != NULL)
            m_listener->HandleResyncPlayerRequest(m_roomSlotPlayers);
    }
    for (std::vector<NQRNetworkPlayer*>::iterator player = m_unconnectedPlayers.begin();
         player != m_unconnectedPlayers.end(); ++player)
    {
        if (m_listener != NULL)
            m_listener->HandlePlayerLeaving(*player);
        RemoveConnectedPlayer(*player);
        delete *player;
    }
    m_unconnectedPlayers.clear();
    LeaveCriticalSection(&m_playerMutex);
}

void NQRNetworkManager_WiiU::UpdateRoomSyncUIDsFromPlayers()
{
    EnterCriticalSection(&m_playerMutex);
    if (m_host)
        for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
            if (m_roomSlotPlayers[index] != NULL)
                static_cast<PlayerUID&>(m_roomPlayers[index]) = m_roomSlotPlayers[index]->GetUID();
    LeaveCriticalSection(&m_playerMutex);
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
    if (!m_localGame)
    {
        UpdateRoomSyncUIDsFromPlayers();
        RemoteDataSend(0, 0xff, m_roomPlayers, sizeof(m_roomPlayers));
    }
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
    if (!m_sendQueue.empty() && s_applicationConnected)
        m_sendQueue.pop_front();
    LeaveCriticalSection(&m_sendQueueMutex);
}

nn::pia::Result NQRNetworkManager_WiiU::Pia_InitializeCommon(unsigned char* memory, unsigned int memorySize, unsigned long long traceFlags, bool enableCachedPrint, bool)
{
    checkPiaCoreToUse();
    nn::pia::Result result = 0;
    result = nn::pia::common::Initialize(memory, memorySize);
    AssertPiaError(result);
    result = nn::pia::common::BeginSetup();
    AssertPiaError(result);
    if (enableCachedPrint)
    {
        result = nn::pia::common::CachedPrint::CreateInstance(cPiaCachedPrintBufferSize);
        AssertPiaError(result);
    }
    result = nn::pia::common::Trace::CreateInstance();
    AssertPiaError(result);
    nn::pia::common::Trace::GetInstance()->SetFlag(traceFlags);
    result = nn::pia::common::Scheduler::CreateInstance(cPiaSchedulerThreadPriority);
    AssertPiaError(result);
    result = nn::pia::common::WatermarkManager::CreateInstance();
    AssertPiaError(result);
    nn::pia::common::WatermarkManager::GetInstance()->EnableAllWatermark();
    result = nn::pia::common::EndSetup();
    AssertPiaError(result);
    return result;
}

nn::pia::Result NQRNetworkManager_WiiU::Pia_InitializeInet()
{
    checkPiaCoreToUse();
    nn::pia::Result result = 0;
    result = nn::pia::inet::Initialize();
    AssertPiaError(result);
    result = nn::pia::inet::BeginSetup();
    AssertPiaError(result);
    result = nn::pia::inet::NexFacade::CreateInstance();
    AssertPiaError(result);
    result = nn::pia::inet::EndSetup();
    AssertPiaError(result);
    return result;
}

nn::pia::Result NQRNetworkManager_WiiU::Pia_InitializeSession(const nn::pia::session::Session::Setting& setting)
{
    checkPiaCoreToUse();
    nn::pia::Result result = 0;
    result = nn::pia::session::Initialize();
    AssertPiaError(result);
    result = nn::pia::session::BeginSetup();
    AssertPiaError(result);
    result = nn::pia::session::Session::CreateInstance(setting);
    AssertPiaError(result);
    result = nn::pia::session::EndSetup();
    AssertPiaError(result);
    return result;
}

NQRNetworkManager_WiiU::ConnectedStation* NQRNetworkManager_WiiU::GetConnectedStation(unsigned long long stationId)
{
    EnterCriticalSection(&m_connectedStationMutex);
    for (unsigned int index = 0; index < m_connectedStations.size(); ++index)
    {
        ConnectedStation* station = &m_connectedStations[index];
        if (station->stationId == stationId)
        {
            LeaveCriticalSection(&m_connectedStationMutex);
            return station;
        }
    }
    LeaveCriticalSection(&m_connectedStationMutex);
    return NULL;
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

void NQRNetworkManager_WiiU::Pia_SessionCallback(nn::pia::session::Session::EventType eventType, unsigned long long stationId, void*)
{
    if (s_networkManager == NULL)
        return;
    if (eventType == nn::pia::session::Session::cEventTypeJoin)
        s_networkManager->AddConnectedStation(stationId);
    else if (eventType == nn::pia::session::Session::cEventTypeLeave)
    {
        s_networkManager->RemoveRemotePlayersAndSync(stationId, 15);
        s_networkManager->RemoveConnectedStation(stationId);
    }
}

bool NQRNetworkManager_WiiU::NexFacadeLogin(unsigned int principalId)
{
    m_pendingSessionId = principalId;
    SetNexAsyncProcess(eNEXAsyncProcess_Login);
    return true;
}

void NQRNetworkManager_WiiU::InitialiseAfterOnline()
{
    if (gProfile->IsFullVersion())
    {
        nn::pia::Result result = Pia_InitializeCommon(s_piaWorkMemory, cPiaWorkMemorySize, cPiaTraceFlags, true, true);
        s_piaInitialised = true;
        result = Pia_InitializeInet();
        if (!IsInSession())
            SetState(eNQRNetworkManagerInternalState_IdleOnline);
        return;
    }
    SetState(eNQRNetworkManagerInternalState_Initialising);
}

void NQRNetworkManager_WiiU::OnlineCheck()
{
    if (s_applicationConnected && !s_piaInitialised)
        InitialiseAfterOnline();
    else if (!s_applicationConnected && s_piaInitialised)
        SetApplicationDisconnected();
}

void NQRNetworkManager_WiiU::FriendSearchTick()
{
    if (m_friendPresenceUpdated)
        UpdateFriendFromPresence(0);
}

void NQRNetworkManager_WiiU::FriendPresenceLoginCallback(nn::Result result, void*)
{
    const bool online = nn::fp::IsOnline();
    s_friendPresenceLoginPending = false;
    s_lastPiaErrorCode = nn::fp::ResultToErrorCode(result);
    s_lastPiaResult = static_cast<nn::pia::Result>(result);
    if (s_networkManager != NULL)
    {
        s_networkManager->m_online = online;
        if (!online || result != 0)
        {
            s_networkManager->m_lastNetworkError = static_cast<int>(s_lastPiaErrorCode);
            s_applicationConnected = false;
        }
        s_networkManager->m_friendPresenceUpdated = true;
    }
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
    nn::pia::inet::NexFacade::DestroyInstance();
    nn::pia::inet::Finalize();
}

void NQRNetworkManager_WiiU::Pia_FinalizeCommon()
{
    nn::pia::common::WatermarkManager::DestroyInstance();
    nn::pia::common::Scheduler::DestroyInstance();
    nn::pia::common::Trace::DestroyInstance();
    nn::pia::common::Finalize();
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
    if (s_piaInitialised)
    {
        s_piaShutdownPending = true;
        if (m_nexAsyncProcess == eNEXAsyncProcess_None)
        {
            nn::nex::Core* core = nn::nex::Core::GetInstance();
            if (core != NULL)
                core->SetTerminateImmediately(true);
            m_unknown13A0 = false;
            m_listener->HandleDisconnect(false);
            System::processTimeInMilliSecs();
            if (nn::pia::session::g_piaSession != NULL)
            {
                Session_CleanupSession();
                nn::pia::session::g_piaSession->UnregisterSessionEventCallback();
            }
            Inet_Logout();
            Nex_FinalizeDataStore();
            Pia_Finalize();
            s_piaShutdownPending = false;
            s_piaSessionStarted = false;
            s_piaInitialised = false;
        }
    }
}

void NQRNetworkManager_WiiU::SetLastNetworkError(nn::pia::Result result)
{
    s_lastPiaErrorCode = result.GetErrorCode();
    s_lastPiaResult = result;
}

void NQRNetworkManager_WiiU::UpdateApplicationConnected()
{
    if (s_appSuspended)
        return;
    if (!s_applicationConnected && m_state != eNQRNetworkManagerState_Idle)
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
    unsigned long long now = System::processTimeInMilliSecs();
    if (m_lastRateControlledEvent == 0 || static_cast<int>(now - m_lastRateControlledEvent) >= s_rateControlledEventInterval)
    {
        m_lastRateControlledEvent = now;
        return true;
    }
    return false;
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
    char tokenData[32] = "Friend";
    nn::pia::transport::Station::IdentificationToken token;
    std::memcpy(token.m_token, tokenData, sizeof(token.m_token));
    char* name = gProfile->GetNetworkAccountID();
    nn::pia::Result result = NQRNetworkManager_WiiU_Pia_StartupSession(&token, name);
    if (result != 0)
    {
        SetLastNetworkError(result);
        return false;
    }
    nn::pia::session::g_piaSession->SetSyncClockPulseInterval(1000);
    s_piaSessionStarted = true;
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
    for (int index = 0; index < m_localPlayerCount; ++index)
        m_roomPlayers[index].m_uiStationId = m_localStationId;
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
    if (nn::nex::Core::GetInstance()->GetScheduler() == NULL)
        return;
    nn::nex::Core::GetInstance()->GetScheduler()->Dispatch(s_nexDispatchTimeout, 0);
    nn::pia::common::g_piaJobScheduler->Dispatch(0);
    if (m_nexAsyncProcess == eNEXAsyncProcess_None)
    {
        if (m_createRequested)
        {
            Pia_CreateSession();
            m_createRequested = false;
            return;
        }
        if (m_joinRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            Pia_JoinSession();
            m_joinRequested = false;
            return;
        }
        if (m_joinMatchmakeRequested)
        {
            Pia_JoinMatchmakeSession();
            m_joinMatchmakeRequested = false;
            return;
        }
        if (m_leaveRequested)
        {
            Pia_LeaveSession();
            m_leaveRequested = false;
            return;
        }
        if (m_browseRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            Pia_BrowseSession();
            m_browseRequested = false;
            return;
        }
        if (m_randomJoinRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            Pia_JoinRandomSession();
            m_randomJoinRequested = false;
            return;
        }
        if (m_updateRequested)
            UpdateExternalRoomDataFromThread();
        else
            UpdateMatchmakingAttributes();
        return;
    }
    if (m_nexCallContext != NULL && m_nexCallContext->GetState() == nn::nex::CallContext::CallInProgress)
        return;
    switch (m_nexAsyncProcess)
    {
    case eNEXAsyncProcess_Login:
    {
        if (m_nexCallContext->GetState() == nn::nex::CallContext::CallInProgress)
            return;
        SetNexAsyncProcess(eNEXAsyncProcess_None);
        nn::nex::NgsFacade* ngsFacade = Nex_GetNgsFacadeInstance();
        nn::nex::qResult loginResult(m_nexCallContext->GetResult());
        if (!static_cast<bool>(loginResult))
        {
            m_nexLoginState = eNEXLoginState_Failed;
            if (m_internalState == eNQRNetworkManagerInternalState_HostCreatingSession)
                SetState(eNQRNetworkManagerInternalState_HostRegisteringSession);
            else
                SetState(m_internalState == eNQRNetworkManagerInternalState_JoinCreatingSession ?
                    eNQRNetworkManagerInternalState_JoinSessionJoined :
                    eNQRNetworkManagerInternalState_Initialising);
            nn::nex::qResult failure(m_nexCallContext->GetResult());
            nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(failure);
            s_lastLoginErrorCode = ngsFacade->GetLastLoginErrorCode();
            if (s_lastLoginErrorCode == s_loginErrorServiceUnavailable)
            {
                s_applicationConnected = false;
                s_connectFailed = true;
            }
        }
        else
        {
            nn::nex::qResult connected = ngsFacade->IsConnected();
            if (!static_cast<bool>(connected))
            {
                s_lastLoginErrorCode = ngsFacade->GetLastLoginErrorCode();
            }
            else
            {
                bool nexFacadeBound = true;
                if (nn::pia::inet::NexFacade::GetInstance() != NULL)
                {
                    nn::pia::common::String accessKey(s_nexAccessKey);
                    nn::pia::inet::NexFacade::LoginInfo loginInfo;
                    loginInfo.ngsFacade = ngsFacade;
                    loginInfo.gameServerId = s_nexGameServerId;
                    loginInfo.accessKey = &accessKey;
                    nexFacadeBound = nn::pia::inet::NexFacade::GetInstance()->Bind(&loginInfo) == 0;
                }
                if (nexFacadeBound && Nex_InitializeMatchmake() && Pia_StartupSession())
                {
                    int internalState = m_internalState;
                    m_nexLoginState = eNEXLoginState_LoggedIn;
                    switch (internalState)
                    {
                    case eNQRNetworkManagerInternalState_IdleOnline:
                        SetState(eNQRNetworkManagerInternalState_Idle);
                        break;
                    case eNQRNetworkManagerInternalState_HostCreatingSession:
                        Pia_SignalCreateSession();
                        break;
                    case eNQRNetworkManagerInternalState_JoinCreatingSession:
                        Pia_SignalJoinSession();
                        break;
                    default:
                        break;
                    }
                    return;
                }
            }
        }
        m_nexLoginState = eNEXLoginState_Failed;
        Pia_Shutdown();
        if (m_internalState == eNQRNetworkManagerInternalState_HostCreatingSession)
        {
            SetState(eNQRNetworkManagerInternalState_HostRegisteringSession);
            return;
        }
        if (m_internalState == eNQRNetworkManagerInternalState_JoinCreatingSession)
        {
            SetState(eNQRNetworkManagerInternalState_JoinSessionJoined);
            return;
        }
        if (!IsInSession())
            SetState(eNQRNetworkManagerInternalState_Initialising);
        break;
    }
    case eNEXAsyncProcess_Shutdown:
        if (m_nexCallContext->GetState() != nn::nex::CallContext::CallInProgress)
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            m_nexLoginState = eNEXLoginState_Failed;
            nn::nex::qResult shutdownResult(m_nexCallContext->GetResult());
            if (!static_cast<bool>(shutdownResult))
            {
                nn::nex::qResult failure(m_nexCallContext->GetResult());
                nn::nex::ErrorCodeConverter::ConvertToNetworkErrorCode(failure);
            }
            Pia_Finalize();
            return;
        }
        break;
    case eNEXAsyncProcess_Search:
        if (m_nexCallContext->GetState() != nn::nex::CallContext::CallInProgress)
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            if (m_nexCallContext->GetState() != nn::nex::CallContext::CallSucceeded)
            {
                m_extDataCallback(false, m_extDataCallbackData);
                delete m_gatheringHolder;
                m_gatheringHolder = NULL;
                return;
            }
            if (m_gatheringHolder->GetObject() != NULL)
            {
                const nn::nex::MatchmakeSession* session =
                    static_cast<const nn::nex::MatchmakeSession*>(m_gatheringHolder->GetObject());
                std::vector<unsigned char, nn::nex::MemAllocator<unsigned char> > applicationBuffer;
                applicationBuffer.assign(session->m_ApplicationBuffer.begin(), session->m_ApplicationBuffer.end());
                std::memcpy(m_extDataBuffer, &applicationBuffer[0], applicationBuffer.size());
                m_extDataCallback(true, m_extDataCallbackData);
            }
            delete m_gatheringHolder;
            m_gatheringHolder = NULL;
            return;
        }
        break;
    default:
        break;
    }
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
    EnterCriticalSection(&m_playerConnectionMutex);
    bool wasReady = player->IsReady();
    player->ConnectionComplete();
    bool isReady = player->IsReady();
    LeaveCriticalSection(&m_playerConnectionMutex);
    if (!wasReady && isReady)
        HandlePlayerJoined(player);
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
    EnterCriticalSection(&m_playerMutex);

    PlayerUID* playerUIDs[4] = { &joinData.playerUID0, &joinData.playerUID1, &joinData.playerUID2, &joinData.playerUID3 };
    int incoming = 0;
    for (int index = 0; index < 4; ++index)
    {
        if ((joinData.playerMask & (1u << index)) == 0)
            continue;
        ++incoming;
        for (int existing = 0; existing < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++existing)
        {
            PlayerSyncData& record = m_roomPlayers[existing];
            if (static_cast<PlayerUID&>(record) == *playerUIDs[index] && (record.flags & PlayerUID::FlagMiiPrincipal) == 0)
            {
                if (reason != NULL)
                    *reason = eJoinFailedReason_Full;
                LeaveCriticalSection(&m_playerMutex);
                return false;
            }
        }
    }

    if (!SessionHasSpace(incoming))
    {
        if (reason != NULL)
            *reason = eJoinFailedReason_None;
        LeaveCriticalSection(&m_playerMutex);
        return false;
    }

    for (int index = 0; index < 4; ++index)
    {
        if ((joinData.playerMask & (1u << index)) == 0)
            continue;
        PlayerSyncData& record = m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1];
        static_cast<PlayerUID&>(record) = *playerUIDs[index];
        record.m_uiStationId = stationId;
        record.m_uiLocalPlayerIndex = index;
        record.m_uiSmallId = m_nextSmallId++;
        std::wcscpy(record.m_szMiiName, joinData.displayNames[index]);
        std::strncpy(record.m_szNetworkName, joinData.onlineIds[index], sizeof(record.m_szNetworkName));
        ++m_roomPlayers[0].m_uiPlayerCountMinusOne;
    }

    MapRoomSlotPlayers(-1);
    SyncRoomData();
    LeaveCriticalSection(&m_playerMutex);
    if (reason != NULL)
        *reason = eJoinFailedReason_None;
    return true;
}

void NQRNetworkManager_WiiU::SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason)
{
    if (m_localGame)
        return;
    unsigned int packet[2];
    packet[0] = playerMask;
    packet[1] = static_cast<unsigned int>(reason);
    RemoteDataSend(stationId, 0xfc, packet, sizeof(packet));
}

void NQRNetworkManager_WiiU::PollReceiveData()
{
    if (!m_receiving)
        return;

    if (nn::pia::transport::g_piaTransport == NULL)
        return;
    nn::pia::transport::Protocol* protocol =
        nn::pia::transport::g_piaTransport->m_protocolManager.SearchProtocol(
            static_cast<nn::pia::transport::ProtocolId>(m_sessionId),
            static_cast<unsigned char>(nn::pia::transport::ReliableProtocol::cProtocolType));
    nn::pia::transport::ReliableProtocol* reliable =
        dynamic_cast<nn::pia::transport::ReliableProtocol*>(protocol);
    if (reliable == NULL)
        return;

    unsigned char* packet = static_cast<unsigned char*>(std::malloc(0x20000));
    if (packet == NULL)
        return;

    unsigned long long stationId = 0;
    unsigned int size = 0;
    while (reliable->Receive(&stationId, packet, &size, 0x20000) == 0)
    {
        if (size == 0)
            continue;
        const unsigned char channel = packet[0];
        if (channel < 4)
        {
            EnterCriticalSection(&m_connectedPlayerMutex);
            NQRNetworkPlayer* sender = GetPlayerFromStationIdAndLocalIdx(stationId, channel);
            NQRNetworkPlayer* receiver = GetLocalPlayerByUserIndex(channel);
            if (sender != NULL && receiver != NULL && size > 1)
            {
                if (size > 2)
                {
                    receiver->ReadAckRequest(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]), sender);
                    if (m_listener != NULL)
                        m_listener->HandleDataReceived(sender, receiver, packet + 2, size - 2);
                }
                else
                    receiver->ReadAckReturn(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]));
            }
            LeaveCriticalSection(&m_connectedPlayerMutex);
        }
        else if (channel == 0xfc && size >= 9)
        {
            unsigned long long mask = 0;
            std::memcpy(&mask, packet + 1, sizeof(mask));
            if ((mask & 1) == 0)
            {
                for (unsigned int index = 1; index < 4; ++index)
                    if ((mask & (1ull << index)) != 0)
                        gApp.SetAction(static_cast<int>(index), eXuiAction_RemoveLocalPlayer, NULL);
                int parameter = static_cast<int>(IDS_CONFIRM_OK);
                if (m_localGame == false)
                    gConsoleUIController.RequestErrorMessage(0x1afa54b5, 0xf18fccd0,
                        &parameter, 1, 0xff, NULL, NULL, NULL, true);
                else if (m_localGame == true)
                    gConsoleUIController.RequestErrorMessage(0x624aead3, 0x3103be06,
                        &parameter, 1, 0xff, NULL, NULL, NULL, true);
                else
                    gConsoleUIController.RequestErrorMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK,
                        &parameter, 1, 0xff, NULL, NULL, NULL, true);
            }
            else
            {
                const DisconnectPacket::eDisconnectReason reason =
                    m_localGame == false ? DisconnectPacket::eDisconnectReason_GameFull :
                    m_localGame == true ? DisconnectPacket::eDisconnectReason_AgeRestricted :
                    DisconnectPacket::eDisconnectReason_None;
                gApp.SetDisconnectReason(reason);
                if (m_listener != NULL)
                    m_listener->HandleDisconnect(true);
            }
        }
        else if (channel == 0xfd && size >= 5)
        {
            unsigned int mask = 0;
            std::memcpy(&mask, packet + 1, sizeof(mask));
            RemoveRemotePlayersAndSync(stationId, static_cast<int>(mask));
            if (stationId == m_hostStationId && (mask & 1) != 0)
                SetState(eNQRNetworkManagerInternalState_JoinRequested);
        }
        else if (channel == 0xfe && size >= 257)
        {
            ClientJoinData joinData;
            std::memcpy(&joinData, packet + 1, sizeof(joinData));
            EJoinFailedReason reason = eJoinFailedReason_None;
            if (!AddRemotePlayersAndSync(stationId, joinData, &reason))
                SendJoinFailedData(stationId, joinData.playerMask, reason);
        }
        else if (channel == 0xff && size >= 0x281)
        {
            std::memcpy(m_roomPlayers, packet + 1, sizeof(m_roomPlayers));
            MapRoomSlotPlayers(-1);
        }
        size = 0;
    }
    std::free(packet);
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
        s_applicationConnected = true;
        ConnectOnlineNintendo_Returned(this, true, 0);
    }
}

void NQRNetworkManager_WiiU::TickFromThread()
{
    if (s_piaShutdownPending)
        Pia_Shutdown();
    TickSendQueue();
    OnlineCheck();
    FriendSearchTick();
    UpdateApplicationConnected();
    TickNexAsyncProcess();
    PollUnconnectedStations();
    PollReceiveData();
    ErrorHandlingTick();
    NintendoVoiceChat_WiiU::Tick();
}

void NQRNetworkManager_WiiU::RunTickThread()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    Vec3::CreateNewThreadStorage();
    AABB::CreateNewThreadStorage();
    IntCache::CreateNewThreadStorage();
    Compression::UseDefaultThreadStorage();
    Block::CreateNewThreadStorage();
    while (true)
    {
        if (m_tickEvent->WaitForSignal(cTickThreadWaitTime) == 0)
        {
            TickFromThread();
        }
        else if (s_tickConnectionsOnNetworkThread)
        {
            if (!IsHost())
            {
                TickFromThread();
                if (IsInSession())
                    LeaveRoom(true);
                minecraft->tickAllConnections();
            }
            else
            {
                TickNexAsyncProcess();
            }
        }
    }
}

int NQRNetworkManager_WiiU::TickThreadProc(void* context)
{
    static_cast<NQRNetworkManager_WiiU*>(context)->RunTickThread();
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
    static_cast<NQRNetworkManager_WiiU*>(context)->UpdateFriendFromPresence(friendIndex);
}

void NQRNetworkManager_WiiU::Initialise()
{
    s_lastPiaErrorCode = 0;
    s_lastPiaResult = 0;
    gProfile->SetFriendPresenceChangeCallback(UpdateFriendFromPresence_callback, this);
    m_tickEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_tickThread = new C4JThreadImpl(TickThreadProc, this, "NQRNetworkManager_WiiU::TickThreadProc", 0);
    m_tickThread->SetProcessor(s_tickThreadProcessor);
    m_tickThread->Run();
    TryToConnectOnline();
    SetState(eNQRNetworkManagerInternalState_Idle);
    NintendoVoiceChat_WiiU::Initialise(this);
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

void NQRNetworkManager_WiiU::CreateAndJoinRoom(int userIndex, int localUserMask, void* extData,
                                             int extDataSize, int* attributes, int attributeCount,
                                             bool localGame, unsigned char gameMode)
{
    if (!localGame)
        OSEnableHomeButtonMenu(0);

    m_externalDataSize = extDataSize;
    m_receiving = true;
    m_host = true;
    m_gameMode = gameMode;
    m_localGame = localGame;
    m_roomAttributes = attributes;
    m_roomAttributeCount = attributeCount;
    m_externalData = extData;
    if (attributeCount > 0)
        m_matchmaking = true;

    std::memset(m_roomPlayers, 0, sizeof(PlayerSyncData) * 2);
    PlayerSyncData& primary = m_roomPlayers[0];
    primary.m_uiLocalPlayerIndex = userIndex;
    primary.m_uiSmallId = m_nextSmallId++;
    gProfile->GetXUID(userIndex, &primary, !localGame);
    const std::wstring primaryMiiName(gProfile->GetMiiName(userIndex));
    const std::wstring primaryDisplayName(gProfile->GetDisplayName(userIndex));
    std::wcscpy(primary.m_szMiiName, primaryMiiName.c_str());
    std::wcstombs(primary.m_szNetworkName, primaryDisplayName.c_str(), 20);

    const unsigned int remainingMask = static_cast<unsigned int>(localUserMask) & ~(1u << userIndex);
    for (int localIndex = 0; localIndex < 4; ++localIndex)
    {
        if ((remainingMask & (1u << localIndex)) == 0)
            continue;
        PlayerSyncData& record = m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1];
        record.m_uiSmallId = m_nextSmallId++;
        record.m_uiLocalPlayerIndex = localIndex;
        gProfile->GetXUID(userIndex, &record, !localGame);
        const std::wstring miiName(gProfile->GetMiiName(localIndex));
        const std::wstring displayName(gProfile->GetDisplayName(localIndex));
        std::wcscpy(record.m_szMiiName, miiName.c_str());
        std::wcstombs(record.m_szNetworkName, displayName.c_str(), 20);
        ++m_roomPlayers[0].m_uiPlayerCountMinusOne;
    }
    m_localPlayerCount = m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;

    if (m_localGame)
    {
        SetState(eNQRNetworkManagerInternalState_HostCreatingRoom);
        SetState(eNQRNetworkManagerInternalState_HostRoomReady);
        SetLocalPlayersAndSync();
    }
    else
    {
        Pia_SignalCreateSession();
    }
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
    EnterCriticalSection(&m_friendSearchMutex);
    result->m_uiOwnerPrincipalId = m_friendSearchResults[index].ownerPrincipalId;
    std::memcpy(result->m_szOwnerName, m_friendSearchResults[index].ownerName, sizeof(result->m_szOwnerName));
    result->m_uiOwnerLocalFriendIndex = m_friendSearchResults[index].ownerLocalFriendIndex;
    result->m_pExtData = m_friendSearchResults[index].extData;
    result->m_uiSessionId = m_friendSearchResults[index].sessionId;
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
    OSEnableHomeButtonMenu(0);
    m_gameMode = gameMode;
    m_roomPlayerMask = static_cast<unsigned int>(userIndexMask);
    m_receiving = true;
    m_host = false;
    m_localGame = false;
    m_roomAttributeCount = 0;
    if ((searchFlags & 1u) != 0)
        ++m_roomAttributeCount;
    if ((searchFlags & 2u) != 0)
        ++m_roomAttributeCount;
    if ((searchFlags & 4u) != 0)
        ++m_roomAttributeCount;
    if ((searchFlags & 8u) != 0)
        ++m_roomAttributeCount;
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
    m_friendSearchResults.clear();
    m_friendPresenceCount = 0;

    unsigned int friendIds[100];
    unsigned int friendCount = 0;
    const unsigned int principalId = nn::act::GetPrincipalId();
    nn::Result result = nn::fp::GetFriendList(friendIds, &friendCount, 0, 100);
    if (result.IsSuccess())
    {
        nn::fp::FriendPresence presences[100];
        char accountIds[100][17];
        result = nn::fp::GetFriendPresence(presences, friendIds, friendCount);
        if (result.IsSuccess())
        {
            result = nn::fp::GetFriendAccountId(&accountIds[0][0], friendIds, friendCount);
            if (result.IsSuccess())
            {
                for (unsigned int index = 0; index < friendCount; ++index)
                {
                    ++m_friendPresenceCount;
                    const nn::fp::FriendPresence& presence = presences[index];
                    if (!presence.isValid || !presence.isOnline ||
                        presence.gameMode.ownerPrincipalId == principalId)
                        continue;

                    FriendSearchResult friendResult;
                    friendResult.ownerPrincipalId = friendIds[index];
                    friendResult.ownerLocalFriendIndex = index;
                    unsigned char accountData[0x22];
                    std::memcpy(accountData, accountIds[index], sizeof(accountData));
                    std::memcpy(friendResult.ownerName, accountData, sizeof(friendResult.ownerName));
                    if (presence.gameMode.applicationArg[4] == 0x08 &&
                        presence.gameMode.applicationArg[5] == 0x81 &&
                        presence.gameMode.applicationArg[9] == 0)
                    {
                        friendResult.presence.m_uiWorldId = presence.gameMode.ownerPrincipalId;
                        friendResult.presence.m_uiHostAttributes = presence.gameMode.joinGroupId;
                        friendResult.presence.m_bJoinable = true;
                        friendResult.sessionId = presence.gameMode.joinGameId;
                        friendResult.joinable = true;
                        MallocAndSetExtDataFromNQRPresenceInfo(&friendResult.extData, &friendResult.presence);
                        m_friendSearchResults.push_back(friendResult);
                    }
                }
            }
        }
    }
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
    m_presenceGameMode.joinAvailabilityFlag = FALSE;
    m_presenceGameMode.matchmakeSystemType = cPresenceMatchmakeSystemType;
    m_presenceGameMode.joinGameId = s_nexGameServerId;
    m_presenceGameMode.joinGameMode = 0;
    m_presenceGameMode.ownerPrincipalId = 0;
    m_presenceGameMode.joinGroupId = 0;
    std::memset(m_presenceGameMode.applicationArg, 0, sizeof(m_presenceGameMode.applicationArg));
    gProfile->SetRichPresenceGameModeData(m_presenceGameMode);
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
    return;
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
    if (!m_host)
    {
        if (!SessionHasSpace(1))
            return false;
        const unsigned int bit = 1u << userIndex;
        if ((m_roomPlayerMask & bit) != 0)
            return false;
        m_roomPlayerMask |= bit;
        SendJoinData();
        UpdateRemotePlay();
        return true;
    }

    if (!SessionHasSpace(1))
        return false;

    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        PlayerSyncData& existing = m_roomPlayers[index];
        if (existing.m_uiStationId == m_localStationId && existing.m_uiLocalPlayerIndex == static_cast<unsigned int>(userIndex))
            return false;
    }

    PlayerSyncData& record = m_roomPlayers[m_roomPlayers[0].m_uiPlayerCountMinusOne + 1];
    record.m_uiLocalPlayerIndex = userIndex;
    record.m_uiStationId = m_localStationId;
    record.m_uiSmallId = m_nextSmallId++;
    const std::wstring miiName(gProfile->GetMiiName(userIndex));
    const std::wstring displayName(gProfile->GetDisplayName(userIndex));
    std::wcscpy(record.m_szMiiName, miiName.c_str());
    std::wcstombs(record.m_szNetworkName, displayName.c_str(), 20);
    ++m_roomPlayers[0].m_uiPlayerCountMinusOne;

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
    return m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
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
    return index < 8 ? GetPlayerIfReady(m_roomSlotPlayers[index]) : NULL;
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
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomPlayers[index] == uid)
        {
            NQRNetworkPlayer* player = GetPlayerIfReady(m_roomSlotPlayers[index]);
            LeaveCriticalSection(&m_playerMutex);
            return player;
        }
    }
    LeaveCriticalSection(&m_playerMutex);
    return NULL;
}

std::wstring NQRNetworkManager_WiiU::GetDisplayNameByGamertag(std::wstring gamertag)
{
    if (m_miiNames.find(gamertag) != m_miiNames.end())
        return m_miiNames[gamertag];
    return gamertag;
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
    EnterCriticalSection(&m_playerMutex);
    NQRNetworkPlayer* player = m_roomSlotPlayers[0];
    LeaveCriticalSection(&m_playerMutex);
    return player;
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
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
        if (m_roomSlotPlayers[index] == player)
            return index;
    return 0;
}

void NQRNetworkManager_WiiU::SetPresenceDataStartHostingGame()
{
    if (m_localGame)
    {
        ClearRichPresenceCustomData();
        return;
    }
    PresenceSyncInfo presenceInfo;
    SetNQRPresenceInfoFromExtData(&presenceInfo, m_externalData);
    UpdateRichPresenceCustomData(&presenceInfo, sizeof(presenceInfo));
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
    s_applicationConnected = false;
    m_online = false;
}

void NQRNetworkManager_WiiU::Pia_ShutdownForLan()
{
    s_applicationConnected = false;
    m_online = false;
    m_leaveRequested = true;
}

void NQRNetworkManager_WiiU::TryToConnectOnline(int (*callback)(void*, bool, int), void* callbackData, bool showProgress)
{
    s_connectCallback = callback;
    s_connectCallbackData = callbackData;
    s_cancelConnect = false;
    if (showProgress)
    {
        if (s_networkManager != NULL)
            s_networkManager->TryToConnectOnline();
    }
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
