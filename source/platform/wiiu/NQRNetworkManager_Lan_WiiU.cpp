#include "platform/wiiu/NQRNetworkManager_Lan_WiiU.hpp"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameSessionData.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/System.hpp"
#include "game/network/DisconnectPacket.hpp"
#include "game/network/CGameNetworkManager.hpp"
#include "nn/nex/DataStoreClientTemplate.hpp"
#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/pia/Reconstruction.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/CachedPrint.hpp"
#include "nn/pia/common/Scheduler.hpp"
#include "nn/pia/common/Trace.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/lan/LanCreateSessionSetting.hpp"
#include "nn/pia/lan/LanFacade.hpp"
#include "nn/pia/lan/LanJoinSessionSetting.hpp"
#include "nn/pia/lan/LanSessionSearchCriteria.hpp"
#include "nn/pia/lan/LanSupport.hpp"
#include "nn/pia/lan/LanUpdateSessionSetting.hpp"
#include "nn/pia/session/ISessionInfo.hpp"
#include "nn/pia/session/ISessionInfoList.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/Station.hpp"
#include "platform/wiiu/C4JEventImpl.hpp"
#include "platform/wiiu/C4JThreadImpl.hpp"
#include "platform/wiiu/CInput.hpp"
#include "platform/wiiu/CProfile.hpp"
#include "platform/wiiu/NQRNetworkPlayer.hpp"
#include "platform/wiiu/NintendoVoiceChat_WiiU.hpp"
#include "platform/wiiu/Shared_Transport_Setup.hpp"
#include "platform/wiiu/WiiU_Runtime.hpp"

void AssertPiaError(nn::pia::Result& result);
nn::pia::Result NQRNetworkManager_Lan_WiiU_Pia_StartupSession(
    nn::pia::transport::Station::IdentificationToken* token, char* name);

static NQRNetworkManager_Lan_WiiU* s_networkManager;
static int (*s_connectCallback)(void*, bool, int);
static void* s_connectCallbackData;
static bool s_cancelConnect;
static bool s_appSuspended;
static bool s_handlingBootInvite;
static bool s_initialiseAfterOnline;
static bool s_sessionDisconnectPending;
static bool s_piaSessionStarted = false;
static bool s_piaInitialised;
static bool s_extDataRequested = false;
static unsigned int s_extDataSessionId;
static nn::pia::Result s_lastPiaResult;
static bool s_piaFinalizing;
static nn::nex::DataStoreClient* s_dataStoreClient;

extern "C" int socket_lib_init();
extern "C" int socket_lib_finish();
extern "C" void OSFastMutex_Unlock(OSFastMutex* mutex);

static bool s_socketLibraryInitialised;
static bool s_localCommunicationInitialised;
static bool s_piaShutdownPending;
static bool s_shutdownTickPending;
static bool s_shutdownForOnline;
static long long s_shutdownTickStartTime = 0;
static int s_lastNetworkError = 0;
static bool s_connectNetworkErrorPending;
static unsigned int s_connectedLocalPlayerMask;
static int s_tickThreadProcessor = 2;
static const unsigned int cPiaCachedPrintBufferSize = 512 * 1024;
static const int cPiaLanSchedulerThreadPriority = 24;
static const unsigned int cPiaWorkMemorySize = 4 * 1024 * 1024;
static const unsigned long long cPiaTraceFlags = 0x4008123;
static const int cPiaResultInvalidState = 0x10c08;
static const unsigned int cPresenceMatchmakeSystemType = 2;
static const int cSaveTransferPacketSize = 16 * 1024;
static const unsigned int cPresenceJoinGameId = 0x28c77600;
static const int cPiaResultSessionNotCreated = 0xc406;
static const int cPiaResultSessionDestroyed = 0x10c01;
static const unsigned int cReceiveBufferSize = 128 * 1024;
static const unsigned int cBrowseSessionPollInterval = 10;
static const unsigned short cSaveTransferDescriptorKind = 11;
static const unsigned short cSaveTransferDescriptorVersion = 2;
static const int cSaveTransferNameLength = 64;
static const unsigned char cSaveTransferGameMode = 78;
static const unsigned char cChannelSaveTransferJoin = 200;
static const unsigned char cChannelSaveTransferData = 201;
static const unsigned char cChannelSaveTransferAck = 202;
static const unsigned char cChannelJoinFailed = 252;
static const unsigned char cChannelLeave = 253;
static const unsigned char cChannelJoin = 254;
static const unsigned char cChannelRoomSync = 255;
static const int cGameFullTitleId = 0x1afa54b5;
static const int cGameFullTextId = 0xf18fccd0;
static const int cMiiInUseTitleId = 0x624aead3;
static const int cMiiInUseTextId = 0x3103be06;
static const unsigned int cErrorCode118_0008 = 0x120168;
static const int cPiaResultSessionVoid = 0x6c4b;
static const int cPiaResultDeniedByParticipants = 0x6c4d;
static const int cPiaResultJoinSessionRefused = 25652;
static const int cPiaResultJoinSessionFull = 27724;
static const int cAnyPad = 255;
static const long long cShutdownTickDuration = 1000;
static const long long cRoomDataUpdateInterval = 6000;
static const unsigned short cMaxSessionParticipants = 8;
static const int cAllLocalPlayersMask = 0xf;

static unsigned char s_piaWorkMemory[cPiaWorkMemorySize];
static NQRNetworkManager::PresenceSyncInfo s_presenceInfo;

int Net_InitialiseSocket()
{
    if (socket_lib_init() != 0)
        return 3;
    return 0;
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

void checkPiaCoreToUse();

void Local_FinalizeLocalCommunication()
{
    if (s_localCommunicationInitialised)
        s_localCommunicationInitialised = false;
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
      m_state(eNQRNetworkManagerState_Uninitialised), m_nextIdleReasonIsFailure(false), m_host(false),
      m_receiving(false), m_signedInLive(0), m_searchCriteria(NULL), m_unknownCC(0), m_applicationConnected(false), m_online(false),
      m_sessionId(0), m_matchmaking(false), m_hostStationIdCached(false), m_hostStationId(0),
      m_localStationId(0), m_roomPlayerMask(0), m_friendSearchBusy(false),
      m_friendPresenceCount(0), m_nextSmallId(0),
      m_nexCallContext(NULL), m_nexAsyncProcess(eNEXAsyncProcess_None), m_pendingSessionId(0),
      m_localPlayerCount(0), m_localPlayersJoined(8), m_gameMode(0), m_externalData(NULL),
      m_externalDataSize(0), m_running(false), m_leaveRequested(false), m_createRequested(false),
      m_joinRequested(false), m_browseRequested(false), m_randomJoinRequested(false),
      m_updateRequested(false), m_lastRateControlledEvent(0), m_saveTransferState(eSaveTransfer_Idle),
      m_saveTransferData(NULL), m_saveTransferSize(0), m_saveTransferOffset(0),
      m_saveTransferPacketPending(false), m_saveTransferSessionId(0)
{
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    InitializeCriticalSection(&m_playerMutex);
    InitializeCriticalSection(&m_playerConnectionMutex);
    InitializeCriticalSection(&m_sendQueueMutex);
    InitializeCriticalSection(&m_friendSearchMutex);
    InitializeCriticalSection(&m_connectedPlayerMutex);
    InitializeCriticalSection(&m_connectedStationMutex);
    s_networkManager = this;
    m_saveTransferDescriptor = new SaveTransferDescriptor;
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
    OSFastMutex* mutex = &m_connectedStationMutex;
    EnterCriticalSection(mutex);
    for (unsigned int index = 0; index < m_connectedStations.size(); ++index)
    {
        ConnectedStation& station = m_connectedStations[index];
        if (station.stationId == stationId)
        {
            station = m_connectedStations.back();
            m_connectedStations.pop_back();
            LeaveCriticalSection(mutex);
            return;
        }
    }
    LeaveCriticalSection(mutex);
}

void NQRNetworkManager_Lan_WiiU::RemoveConnectedPlayer(NQRNetworkPlayer* player)
{
    OSFastMutex* mutex = &m_connectedPlayerMutex;
    EnterCriticalSection(mutex);
    const unsigned long long stationId = player->GetStationId();
    if (player->IsLocal())
        s_connectedLocalPlayerMask &= ~(1u << player->GetLocalPlayerIndex());
    for (unsigned int index = 0; index < m_players.size(); ++index)
    {
        if (m_players[index] == player)
        {
            m_players[index] = m_players.back();
            m_players.pop_back();
            break;
        }
    }
    bool stationHasPlayers = false;
    for (unsigned int index = 0; index < m_players.size(); ++index)
    {
        if (m_players[index]->GetStationId() == stationId)
        {
            stationHasPlayers = true;
            break;
        }
    }
    if (!stationHasPlayers)
        RemoveConnectedStation(stationId);
    LeaveCriticalSection(mutex);
}

void NQRNetworkManager_Lan_WiiU::NonNetworkPlayerComplete(NQRNetworkPlayer* player, unsigned char smallId)
{
    NetworkPlayerSmallIdAllocated(player, smallId);
}

void NQRNetworkManager_Lan_WiiU::AddConnectedPlayer(NQRNetworkPlayer* player)
{
    OSFastMutex* mutex = &m_connectedPlayerMutex;
    EnterCriticalSection(mutex);
    m_players.push_back(player);
    if (player->IsLocal())
        s_connectedLocalPlayerMask |= 1u << player->GetLocalPlayerIndex();
    if (player->IsLocal())
    {
        PlayerNameMap::iterator existing = m_miiNames.find(player->GetNetworkNameW());
        if (existing != m_miiNames.end())
            m_miiNames.erase(existing);
        m_miiNames.insert(std::make_pair(player->GetNetworkNameW(), player->GetMiiName()));
    }
    LeaveCriticalSection(mutex);
}

void NQRNetworkManager_Lan_WiiU::HandlePlayerJoined(NQRNetworkPlayer* player)
{
    if (m_listener != NULL)
        m_listener->HandlePlayerJoined(player);
    if (!m_host && player->IsLocal())
        ++m_localPlayersJoined;
}

void NQRNetworkManager_Lan_WiiU::FindOrCreateNonNetworkPlayer(int slotIndex, int type, PlayerSyncData* syncData)
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
    NQRNetworkPlayer* player;
    if (type == NQRNetworkPlayer::eNQRNetworkPlayerType_Local ||
        (m_host && type == NQRNetworkPlayer::eNQRNetworkPlayerType_Host))
    {
        gProfile->GetXUID(syncData->m_uiLocalPlayerIndex, &localUID, false);
        player = new NQRNetworkPlayer(this, static_cast<NQRNetworkPlayer::eNQRNetworkPlayerType>(type), m_host,
            syncData->m_uiStationId, syncData->m_uiLocalPlayerIndex, &localUID,
            syncData->m_szNetworkName, syncData->m_szMiiName);
    }
    else
    {
        player = new NQRNetworkPlayer(this, static_cast<NQRNetworkPlayer::eNQRNetworkPlayerType>(type), m_host,
            syncData->m_uiStationId, syncData->m_uiLocalPlayerIndex, syncData,
            syncData->m_szNetworkName, syncData->m_szMiiName);
    }
    NonNetworkPlayerComplete(player, syncData->m_uiSmallId);
    AddConnectedPlayer(player);
    m_roomSlotPlayers[slotIndex] = player;
    HandlePlayerJoined(player);
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
    m_roomPlayers[0].m_uiPlayerCountMinusOne = count == 0 ? 0 : count - 1;
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
    SendQueueItem item;
    item.stationId = stationId;
    item.data = static_cast<unsigned char*>(std::malloc(size + 1));
    item.size = size + 1;
    item.data[0] = channel;
    if (data != NULL)
        std::memcpy(item.data + 1, data, size);
    m_sendQueue.push_back(item);
    LeaveCriticalSection(&m_sendQueueMutex);
}

void NQRNetworkManager_Lan_WiiU::SyncRoomData()
{
    if (!m_localGame)
    {
        UpdateRoomSyncUIDsFromPlayers();
        RemoteDataSend(0, 0xff, m_roomPlayers, sizeof(m_roomPlayers));
    }
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
    while (!m_sendQueue.empty())
    {
        const SendQueueItem& item = m_sendQueue.front();
        std::free(item.data);
        m_sendQueue.pop_front();
    }
}

void NQRNetworkManager_Lan_WiiU::SetState(eNQRNetworkManagerInternalState state)
{
    if (state == eNQRNetworkManagerInternalState_Idle)
        m_matchmaking = false;
    bool idleReasonIsFailure = false;
    if (state == eNQRNetworkManagerInternalState_Idle && m_nextIdleReasonIsFailure)
    {
        idleReasonIsFailure = true;
        m_nextIdleReasonIsFailure = false;
    }
    m_internalState = state;
}

void NQRNetworkManager_Lan_WiiU::TickSendQueue()
{
    checkPiaCoreToUse();
    nn::pia::transport::Transport* transport = nn::pia::transport::g_piaTransport;
    if (transport == NULL)
        return;
    EnterCriticalSection(&m_sendQueueMutex);
    nn::pia::transport::ReliableProtocol* protocol =
        transport->m_protocolManager.GetProtocol<nn::pia::transport::ReliableProtocol>(m_reliableProtocolHandle);
    while (!m_sendQueue.empty())
    {
        SendQueueItem item = m_sendQueue.front();
        if (item.stationId == 0)
        {
            nn::pia::Result result = protocol->SendToAll(item.data, item.size);
            if (result != 0)
            {
                if (result == nn::pia::transport::cResultWindowOverflow)
                    break;
            }
        }
        else if (protocol->IsInCommunication(item.stationId))
        {
            nn::pia::Result result = protocol->Send(item.stationId, item.data, item.size);
            if (result != 0)
            {
                if (result == nn::pia::transport::cResultWindowOverflow)
                    break;
            }
        }
        else if (m_host)
        {
            RemoveRemotePlayersAndSync(item.stationId, cAllLocalPlayersMask);
        }
        else if (m_internalState == eNQRNetworkManagerInternalState_Playing)
        {
            ClearSendQueue();
            SetState(eNQRNetworkManagerInternalState_Ending);
            LeaveCriticalSection(&m_sendQueueMutex);
            return;
        }
        std::free(item.data);
        m_sendQueue.pop_front();
    }
    LeaveCriticalSection(&m_sendQueueMutex);
}

int NQRNetworkManager_Lan_WiiU::InitialiseNetwork(NetworkType type)
{
    if (type != eNetworkType_Adhoc && type != eNetworkType_Lan)
        return 0;
    return Net_InitialiseSocket();
}

nn::pia::Result NQRNetworkManager_Lan_WiiU::Pia_InitializeCommon(unsigned char* memory, unsigned int memorySize, unsigned long long traceFlags, bool enableCachedPrint, bool)
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
    result = nn::pia::common::Scheduler::CreateInstance(cPiaLanSchedulerThreadPriority);
    AssertPiaError(result);
    result = nn::pia::common::WatermarkManager::CreateInstance();
    AssertPiaError(result);
    nn::pia::common::WatermarkManager::GetInstance()->EnableAllWatermark();
    result = nn::pia::common::EndSetup();
    AssertPiaError(result);
    return result;
}

nn::pia::Result NQRNetworkManager_Lan_WiiU::Lan_InitializePiaLan()
{
    nn::pia::Result result = 0;
    result = nn::pia::lan::Initialize();
    AssertPiaError(result);
    result = nn::pia::lan::BeginSetup();
    AssertPiaError(result);
    result = nn::pia::lan::LanFacade::CreateInstance();
    AssertPiaError(result);
    result = nn::pia::lan::EndSetup();
    AssertPiaError(result);
    return result;
}

nn::pia::Result NQRNetworkManager_Lan_WiiU::Pia_InitializeSession(const nn::pia::session::Session::Setting& setting)
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

NQRNetworkManager_Lan_WiiU::ConnectedStation* NQRNetworkManager_Lan_WiiU::GetConnectedStation(unsigned long long stationId)
{
    OSFastMutex* mutex = &m_connectedStationMutex;
    EnterCriticalSection(mutex);
    for (unsigned int index = 0; index < m_connectedStations.size(); ++index)
    {
        ConnectedStation* station = &m_connectedStations[index];
        if (station->stationId == stationId)
        {
            LeaveCriticalSection(mutex);
            return station;
        }
    }
    LeaveCriticalSection(mutex);
    return NULL;
}

void NQRNetworkManager_Lan_WiiU::AddConnectedStation(unsigned long long stationId)
{
    OSFastMutex* mutex = &m_connectedStationMutex;
    EnterCriticalSection(mutex);
    ConnectedStation* existing = GetConnectedStation(stationId);
    if (existing == NULL)
    {
        ConnectedStation station(stationId);
        m_connectedStations.push_back(station);
    }
    LeaveCriticalSection(mutex);
}

void NQRNetworkManager_Lan_WiiU::Pia_SignalLeaveSession()
{
    m_leaveRequested = true;
}

void NQRNetworkManager_Lan_WiiU::Pia_SessionCallback(nn::pia::session::Session::EventType eventType, unsigned long long stationId, void*)
{
    checkPiaCoreToUse();
    switch (eventType)
    {
    case nn::pia::session::Session::cEventTypeJoin:
    {
        bool host = s_networkManager->m_host;
        s_networkManager->m_localStationId = nn::pia::session::g_piaSession->m_localStationId;
        s_networkManager->m_hostStationId = nn::pia::session::g_piaSession->m_hostStationId;
        s_networkManager->m_hostStationIdCached = true;
        if (!host)
            s_networkManager->AddConnectedStation(stationId);
        else if (stationId != s_networkManager->m_localStationId)
            s_networkManager->AddConnectedStation(stationId);
        break;
    }
    case nn::pia::session::Session::cEventTypeLeave:
        if (!s_networkManager->m_host)
        {
            if (stationId == s_networkManager->m_hostStationId &&
                s_networkManager->m_internalState != eNQRNetworkManagerInternalState_LeavingRoom &&
                !s_sessionDisconnectPending)
            {
                s_networkManager->SetState(eNQRNetworkManagerInternalState_LeavingRoom);
                s_networkManager->Pia_SignalLeaveSession();
            }
        }
        else if (stationId != s_networkManager->m_localStationId)
        {
            s_networkManager->RemoveRemotePlayersAndSync(stationId, cAllLocalPlayersMask);
            s_networkManager->RemoveConnectedStation(stationId);
        }
        break;
    default:
        break;
    }
}


void NQRNetworkManager_Lan_WiiU::InitialiseAfterOnline()
{
    checkPiaCoreToUse();
    s_initialiseAfterOnline = false;
    if (!gProfile->IsFullVersion())
    {
        SetState(eNQRNetworkManagerInternalState_InitialiseFailed);
        return;
    }

    nn::pia::Result result = Pia_InitializeCommon(s_piaWorkMemory, cPiaWorkMemorySize, cPiaTraceFlags, true, true);
    AssertPiaError(result);
    result = Lan_InitializePiaLan();
    AssertPiaError(result);
    if (InitialiseNetwork(eNetworkType_Lan) == 0)
    {
        result = Lan_StartupPiaLan();
        if (result == 0 && Pia_StartupSession())
        {
            s_piaInitialised = true;
            return;
        }
    }
    if (!s_piaInitialised)
    {
        s_piaInitialised = true;
        Pia_Shutdown();
    }
}

bool NQRNetworkManager_Lan_WiiU::IsInitialisedAfterOnline()
{
    return s_piaInitialised;
}

void NQRNetworkManager_Lan_WiiU::OnlineCheck()
{
    if (s_piaShutdownPending)
        return;
    if (s_initialiseAfterOnline)
        InitialiseAfterOnline();
    m_signedInLive = gProfile->IsSignedInLive(gProfile->GetPrimaryPad());
}

void NQRNetworkManager_Lan_WiiU::FriendSearchTick()
{
    if (m_friendSearchState == eNQRNetworkManagerFriendSearchState_Searching)
        GetFriendsFromSessionBrowse();
}


void NQRNetworkManager_Lan_WiiU::Session_CleanupSession()
{
    checkPiaCoreToUse();
    if (nn::pia::session::g_piaSession != NULL)
        nn::pia::session::g_piaSession->Cleanup();
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
    checkPiaCoreToUse();
    if (nn::pia::session::g_piaSession != NULL)
    {
        nn::pia::session::g_piaSession->Cleanup();
        nn::pia::session::Session::DestroyInstance();
    }
    nn::pia::session::Finalize();
}

void NQRNetworkManager_Lan_WiiU::Lan_FinalizePiaLan()
{
    m_applicationConnected = false;
}

void NQRNetworkManager_Lan_WiiU::Pia_FinalizeCommon()
{
    checkPiaCoreToUse();
    nn::pia::common::WatermarkManager::DestroyInstance();
    nn::pia::common::Scheduler::DestroyInstance();
    nn::pia::common::Trace::DestroyInstance();
    if (nn::pia::common::CachedPrint::GetInstance())
    {
        nn::pia::common::CachedPrint::GetInstance()->Flush();
        nn::pia::common::CachedPrint::DestroyInstance();
    }
    nn::pia::common::Finalize();
}

void NQRNetworkManager_Lan_WiiU::Pia_Finalize()
{
    checkPiaCoreToUse();
    s_piaFinalizing = true;
    Local_FinalizeLocalCommunication();
    Net_FinaliseSocket();
    Lan_CleanupPiaLan();
    Session_FinalizePiaSession();
    Transport_FinalizePiaTransport(m_unreliableProtocolHandle, m_reliableProtocolHandle, NULL);
    Lan_FinalizePiaLan();
    Pia_FinalizeCommon();
    s_piaFinalizing = false;
}

void NQRNetworkManager_Lan_WiiU::Pia_Shutdown()
{
    if (!s_piaInitialised)
        return;
    ClearSendQueue();
    RemoveAllPlayersOnClient();
    Nex_FinalizeDataStore();
    Pia_Finalize();
    s_piaInitialised = false;
    s_piaSessionStarted = false;
    s_piaShutdownPending = false;
    if (s_shutdownTickPending)
        s_shutdownTickStartTime = System::processTimeInMilliSecs();
}

void NQRNetworkManager_Lan_WiiU::SetLastNetworkError(nn::pia::Result result)
{
    s_lastNetworkError = result.GetErrorCode();
    s_lastPiaResult = result;
}

void NQRNetworkManager_Lan_WiiU::Net_GetLocalAddress(unsigned int* address, unsigned int* subnetMask)
{
    if (address != NULL)
        *address = 0;
    if (subnetMask != NULL)
        *subnetMask = 0;
}

nn::pia::Result NQRNetworkManager_Lan_WiiU::Lan_StartupPiaLan()
{
    if (nn::pia::lan::LanFacade::GetInstance() == NULL)
        return cPiaResultInvalidState;

    unsigned int address;
    unsigned int subnetMask;
    Net_GetLocalAddress(&address, &subnetMask);

    nn::pia::lan::LanFacade::LanNetworkSetting setting;
    setting.m_field00 = address;
    setting.m_field04 = subnetMask;
    setting.m_field08 = 0;

    nn::pia::Result result = nn::pia::lan::LanFacade::GetInstance()->Bind(setting);
    if (result != 0)
        return result;
    return 0;
}

void NQRNetworkManager_Lan_WiiU::UpdateApplicationConnected()
{
    if (s_appSuspended)
        return;
    if (!m_applicationConnected && m_state != eNQRNetworkManagerState_Idle)
    {
        s_lastNetworkError = 1;
        Pia_Shutdown();
        ResetToIdle();
    }
}

bool NQRNetworkManager_Lan_WiiU::Pia_CreateSession()
{
    checkPiaCoreToUse();
    if (m_internalState == eNQRNetworkManagerInternalState_LoggingIn)
    {
        SetState(eNQRNetworkManagerInternalState_HostWaitingForLogin);
        return true;
    }
    nn::pia::lan::LanCreateSessionSetting setting;
    setting.SetGameMode(m_gameMode);
    setting.SetMaxParticipantNum(cMaxSessionParticipants);
    for (int index = 0; index < m_roomAttributeCount; ++index)
    {
        setting.SetAttribute(index, m_roomAttributes[index]);
        m_matchmakeAttributes[index] = m_roomAttributes[index];
    }
    setting.SetOpenSession(true);
    setting.SetSessionType(nn::pia::lan::cLanSessionTypeInvalid);
    setting.SetApplicationData(m_externalData, m_externalDataSize);
    m_lastRoomDataUpdateTime = System::processTimeInMilliSecs();
    setting.SetMaxParticipantNum(cMaxSessionParticipants);
    nn::pia::Result result = nn::pia::session::g_piaSession->CreateSessionAsync(&setting);
    if (result != 0)
    {
        SetLastNetworkError(result);
        SetState(eNQRNetworkManagerInternalState_HostCreateSessionFailed);
        m_saveTransferState = eSaveTransfer_CreateSessionFailed;
        return false;
    }
    SetNexAsyncProcess(eNEXAsyncProcess_CreateSession);
    SetState(eNQRNetworkManagerInternalState_HostCreatingSession);
    return true;
}

bool NQRNetworkManager_Lan_WiiU::canRunRateControlledEvent()
{
    unsigned long long now = System::processTimeInMilliSecs();
    if (m_lastRateControlledEvent != 0 && static_cast<int>(now) - static_cast<int>(m_lastRateControlledEvent) < 10000)
        return false;
    m_lastRateControlledEvent = now;
    return true;
}

bool NQRNetworkManager_Lan_WiiU::Pia_JoinSession()
{
    checkPiaCoreToUse();
    if (m_internalState == eNQRNetworkManagerInternalState_LoggingIn)
    {
        SetState(eNQRNetworkManagerInternalState_JoinWaitingForLogin);
        return true;
    }
    nn::pia::lan::LanJoinSessionSetting setting;
    OSFastMutex* mutex = &m_friendSearchMutex;
    EnterCriticalSection(mutex);
    for (unsigned int index = 0; index < m_friendSearchResults.size(); ++index)
    {
        if (m_friendSearchResults[index].sessionId == m_pendingSessionId)
            setting.SetSessionInfoPtr(&m_friendSearchResults[index].sessionInfo);
    }
    LeaveCriticalSection(mutex);
    nn::pia::Result result = nn::pia::session::g_piaSession->JoinSessionAsync(&setting);
    if (result != 0)
    {
        SetLastNetworkError(result);
        SetState(eNQRNetworkManagerInternalState_JoinSessionFailed);
        if (s_lastPiaResult == cPiaResultDeniedByParticipants)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_StorageError);
        else if (s_lastPiaResult == cPiaResultSessionVoid)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        return false;
    }
    SetState(eNQRNetworkManagerInternalState_JoinJoiningSession);
    SetNexAsyncProcess(eNEXAsyncProcess_JoinSession);
    return true;
}


bool NQRNetworkManager_Lan_WiiU::Pia_LeaveSession()
{
    checkPiaCoreToUse();
    nn::pia::Result result = nn::pia::session::g_piaSession->LeaveSessionAsync();
    if (result != 0)
    {
        if (result == cPiaResultSessionNotCreated)
        {
            SetState(eNQRNetworkManagerInternalState_Ending);
            return true;
        }
        AssertPiaError(result);
        SetState(eNQRNetworkManagerInternalState_LeaveRoomFailed);
        return true;
    }
    SetNexAsyncProcess(eNEXAsyncProcess_LeaveSession);
    return true;
}



void NQRNetworkManager_Lan_WiiU::UpdateExternalRoomDataFromThread()
{
    checkPiaCoreToUse();
    if (m_localGame || s_piaShutdownPending || !m_host)
        return;
    long long now = System::processTimeInMilliSecs();
    long long elapsed = now - m_lastRoomDataUpdateTime;
    if (m_nexAsyncProcess == eNEXAsyncProcess_None && elapsed > cRoomDataUpdateInterval)
    {
        if (nn::pia::session::g_piaSession != NULL)
        {
            nn::pia::lan::LanUpdateSessionSetting setting;
            setting.SetApplicationData(m_externalData, m_externalDataSize);
            nn::pia::Result result = nn::pia::session::g_piaSession->UpdateSessionSettingAsync(&setting);
            if (result == 0)
            {
                SetNexAsyncProcess(eNEXAsyncProcess_Update);
                m_updateRequested = false;
            }
            m_lastRoomDataUpdateTime = now;
        }
    }
    else
        m_updateRequested = true;
}


bool NQRNetworkManager_Lan_WiiU::Pia_StartupSession()
{
    checkPiaCoreToUse();
    char tokenData[32] = "Friend";
    nn::pia::Result result = 0;
    nn::pia::transport::Station::IdentificationToken token;
    std::memcpy(token.m_token, tokenData, sizeof(token.m_token));
    char* name = gProfile->GetNetworkAccountID();
    result = NQRNetworkManager_Lan_WiiU_Pia_StartupSession(&token, name);
    if (result != 0)
    {
        SetLastNetworkError(result);
        AssertPiaError(result);
        return false;
    }
    nn::pia::session::g_piaSession->SetSyncClockPulseInterval(1000);
    s_piaSessionStarted = true;
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
    gApp.setIgnoreMainMenuInputDuringInvite(false);
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
    OSFastMutex* playerMutex = &m_connectedPlayerMutex;
    EnterCriticalSection(playerMutex);
    OSFastMutex* stationMutex = &m_connectedStationMutex;
    EnterCriticalSection(stationMutex);
    std::vector<NQRNetworkPlayer*>::iterator it = m_players.begin();
    while (it != m_players.end())
    {
        NQRNetworkPlayer* player = *it;
        if (m_listener != NULL)
            m_listener->HandlePlayerLeaving(player);
        for (int index = 0; index < 8; ++index)
            if (m_roomSlotPlayers[index] == player)
                m_roomSlotPlayers[index] = NULL;
        RemoveConnectedPlayer(player);
        delete player;
    }
    while (m_connectedStations.size() != 0)
        RemoveConnectedStation(m_connectedStations[0].stationId);
    LeaveCriticalSection(stationMutex);
    LeaveCriticalSection(playerMutex);
}

void NQRNetworkManager_Lan_WiiU::ResetToIdle()
{
    if (!m_host)
        RemoveAllPlayersOnClient();
    m_unknown11F0 = false;
    m_host = false;
    m_hostStationIdCached = false;
    m_nextSmallId = 0;
    EnterCriticalSection(&m_playerMutex);
    for (int index = 0; index < m_roomPlayers[0].m_uiPlayerCountMinusOne + 1; ++index)
    {
        if (m_roomSlotPlayers[index] != NULL)
        {
            RemoveConnectedPlayer(m_roomSlotPlayers[index]);
            delete m_roomSlotPlayers[index];
        }
    }
    std::memset(m_roomSlotPlayers, 0, sizeof(m_roomSlotPlayers));
    std::memset(m_roomPlayers, 0, sizeof(m_roomPlayers));
    LeaveCriticalSection(&m_playerMutex);
    SetState(eNQRNetworkManagerInternalState_Idle);
    ClearSendQueue();
}

void NQRNetworkManager_Lan_WiiU::TickNexAsyncProcess()
{
    checkPiaCoreToUse();
    if (nn::pia::common::g_piaJobScheduler == NULL)
        return;
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
            m_browseRequested = false;
            return;
        }
        if (m_randomJoinRequested)
        {
            if (!canRunRateControlledEvent())
                return;
            m_randomJoinRequested = false;
            return;
        }
        if (m_updateRequested)
            UpdateExternalRoomDataFromThread();
        return;
    }
    if (m_nexCallContext != NULL && m_nexCallContext->GetState() == nn::nex::CallContext::CallInProgress)
        return;
    switch (m_nexAsyncProcess)
    {
    case eNEXAsyncProcess_Update:
        if (nn::pia::session::g_piaSession->IsUpdateSessionSettingCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::session::g_piaSession->GetUpdateSessionSettingResult();
            return;
        }
        break;
    case eNEXAsyncProcess_CreateSession:
        if (nn::pia::session::g_piaSession->IsCreateSessionCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetCreateSessionResult();
            if (result != 0)
            {
                SetLastNetworkError(result);
                SetState(eNQRNetworkManagerInternalState_HostCreateSessionFailed);
                Pia_Shutdown();
                m_saveTransferState = eSaveTransfer_CreateSessionFailed;
                return;
            }
            m_inPiaSession = true;
            SetState(eNQRNetworkManagerInternalState_HostSessionCreated);
            SetState(eNQRNetworkManagerInternalState_HostRoomReady);
            m_localStationId = nn::pia::session::g_piaSession->m_localStationId;
            m_saveTransferState = eSaveTransfer_WaitingForReceiver;
            return;
        }
        break;
    case eNEXAsyncProcess_JoinSession:
        if (nn::pia::session::g_piaSession->IsJoinSessionCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetJoinSessionResult();
            if (result == 0)
            {
                m_inPiaSession = true;
                m_saveTransferState = eSaveTransfer_Joined;
                SetState(eNQRNetworkManagerInternalState_JoinRoomReady);
                ClearIsHandlingBootInvite();
                return;
            }
            m_saveTransferState = eSaveTransfer_JoinFailed;
            SetLastNetworkError(result);
            SetState(m_internalState == eNQRNetworkManagerInternalState_JoinJoiningMatchmakeSession ?
                eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed :
                eNQRNetworkManagerInternalState_JoinSessionFailed);
            if (result == cPiaResultJoinSessionFull)
            {
                Minecraft* minecraft = Minecraft::GetInstance();
                int primaryPad = gProfile->GetPrimaryPad();
                minecraft->mConnectionFailed[primaryPad] = true;
                minecraft->mDisconnectReasons[primaryPad] = DisconnectPacket::eDisconnectReason_GameFull;
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_GameFull);
            }
            else if (result == cPiaResultJoinSessionRefused)
            {
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
                Minecraft* minecraft = Minecraft::GetInstance();
                int primaryPad = gProfile->GetPrimaryPad();
                minecraft->mConnectionFailed[primaryPad] = true;
                minecraft->mDisconnectReasons[primaryPad] = DisconnectPacket::eDisconnectReason_NoUserContentPrivilege;
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NoUserContentPrivilege);
            }
            Session_CleanupSession();
            if (!Pia_StartupSession())
                Pia_Shutdown();
            if (s_lastPiaResult == cPiaResultDeniedByParticipants)
            {
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_StorageError);
                return;
            }
            if (s_lastPiaResult == cPiaResultSessionVoid)
            {
                gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
                return;
            }
        }
        break;
    case eNEXAsyncProcess_LeaveSession:
        if (nn::pia::session::g_piaSession->IsLeaveSessionCompleted())
        {
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            if (m_internalState == eNQRNetworkManagerInternalState_LeavingRoom)
                SetState(eNQRNetworkManagerInternalState_Ending);
            nn::pia::Result result = nn::pia::session::g_piaSession->GetLeaveSessionResult();
            if (result != 0)
                SetLastNetworkError(result);
            Session_CleanupSession();
            if (!Pia_StartupSession())
                Pia_Shutdown();
            ResetToIdle();
        }
        break;
    default:
        break;
    }
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
    RemoteDataSend(0, cChannelSaveTransferJoin, displayName, sizeof(displayName));
}

void NQRNetworkManager_Lan_WiiU::NetworkPlayerConnectionComplete(NQRNetworkPlayer* player)
{
    EnterCriticalSection(&m_playerConnectionMutex);
    bool wasReady = player->IsReady();
    player->ConnectionComplete();
    bool isReady = player->IsReady();
    LeaveCriticalSection(&m_playerConnectionMutex);
    if (!wasReady && isReady)
        HandlePlayerJoined(player);
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
            syncData.m_uiStationId = stationId;
            syncData.m_uiLocalPlayerIndex = index;
            FindOrCreateNonNetworkPlayer(index, NQRNetworkPlayer::eNQRNetworkPlayerType_Remote, &syncData);
        }
    SyncRoomData();
    return true;
}

void NQRNetworkManager_Lan_WiiU::SendJoinFailedData(unsigned long long stationId, unsigned int playerMask, EJoinFailedReason reason)
{
    unsigned int packet[2];
    packet[0] = playerMask;
    packet[1] = static_cast<unsigned int>(reason);
    RemoteDataSend(stationId, cChannelJoinFailed, packet, sizeof(packet));
}

void NQRNetworkManager_Lan_WiiU::SendSaveTransferAckPacket()
{
    RemoteDataSend(m_hostStationId, cChannelSaveTransferAck, NULL, 0);
}

void NQRNetworkManager_Lan_WiiU::PollReceiveData()
{
    checkPiaCoreToUse();
    if (nn::pia::transport::g_piaTransport == NULL)
        return;
    nn::pia::transport::ReliableProtocol* protocol =
        nn::pia::transport::g_piaTransport->m_protocolManager.GetProtocol<nn::pia::transport::ReliableProtocol>(m_reliableProtocolHandle);
    if (!m_hostStationIdCached)
        return;

    unsigned char* packet = static_cast<unsigned char*>(std::malloc(cReceiveBufferSize));
    unsigned long long stationId;
    unsigned int size;
    nn::pia::Result result = protocol->Receive(&stationId, packet, &size, cReceiveBufferSize);
    while (result == 0)
    {
        const unsigned char channel = packet[0];
        switch (channel)
        {
        case 0:
        case 1:
        case 2:
        case 3:
            if (!m_host)
            {
                EnterCriticalSection(&m_connectedPlayerMutex);
                NQRNetworkPlayer* hostPlayer = m_roomSlotPlayers[0];
                NQRNetworkPlayer* localPlayer = GetPlayerFromStationIdAndLocalIdx(m_localStationId, channel);
                if (hostPlayer != NULL && localPlayer != NULL && size > 1)
                {
                    if (size > 2)
                    {
                        localPlayer->ReadAckRequest(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]), hostPlayer);
                        m_listener->HandleDataReceived(hostPlayer, localPlayer, packet + 2, size - 2);
                    }
                    else
                        localPlayer->ReadAckReturn(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]));
                }
                LeaveCriticalSection(&m_connectedPlayerMutex);
            }
            else
            {
                EnterCriticalSection(&m_connectedPlayerMutex);
                NQRNetworkPlayer* hostPlayer = m_roomSlotPlayers[0];
                NQRNetworkPlayer* remotePlayer = GetPlayerFromStationIdAndLocalIdx(stationId, channel);
                if (hostPlayer != NULL && remotePlayer != NULL && size > 1)
                {
                    if (size > 2)
                    {
                        hostPlayer->ReadAckRequest(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]), remotePlayer);
                        m_listener->HandleDataReceived(remotePlayer, hostPlayer, packet + 2, size - 2);
                    }
                    else
                        remotePlayer->ReadAckReturn(static_cast<NQRNetworkPlayer::AckFlags>(packet[1]));
                }
                LeaveCriticalSection(&m_connectedPlayerMutex);
            }
            break;
        case cChannelSaveTransferJoin:
        {
            wchar_t senderName[36];
            std::memcpy(senderName, packet + 1, sizeof(senderName));
            std::wcscpy(m_saveTransferSenderName, senderName);
            m_saveTransferPacketPending = false;
            m_saveTransferState = eSaveTransfer_Transferring;
            break;
        }
        case cChannelSaveTransferData:
        {
            const unsigned int dataSize = size - 1;
            std::memcpy(m_saveTransferReceiveData + m_saveTransferReceiveOffset, packet + 1, dataSize);
            m_saveTransferReceiveOffset += dataSize;
            SendSaveTransferAckPacket();
            if (m_saveTransferReceiveOffset >= m_saveTransferReceiveSize)
                m_saveTransferState = eSaveTransfer_ReceiveComplete;
            break;
        }
        case cChannelSaveTransferAck:
            m_saveTransferPacketPending = false;
            if (m_saveTransferOffset >= m_saveTransferSize)
                m_saveTransferState = eSaveTransfer_SendComplete;
            break;
        case cChannelJoinFailed:
        {
            unsigned int joinFailedData[2];
            std::memcpy(joinFailedData, packet + 1, sizeof(joinFailedData));
            const unsigned int playerMask = joinFailedData[0];
            const EJoinFailedReason reason = static_cast<EJoinFailedReason>(joinFailedData[1]);
            if ((playerMask & 1) == 0)
            {
                for (int userIndex = 1; userIndex < 4; ++userIndex)
                    if ((playerMask & (1 << userIndex)) != 0)
                    {
                        gApp.SetAction(userIndex, eXuiAction_RemoveLocalPlayer, NULL);
                        RemoveLocalPlayerByUserIndex(userIndex);
                    }
                int messageParameter = IDS_CONFIRM_OK;
                if (reason == eJoinFailedReason_Full)
                    gConsoleUIController.RequestErrorMessage(cGameFullTitleId, cGameFullTextId, &messageParameter, 1, 0xff, NULL, NULL, NULL, true);
                else if (reason == eJoinFailedReason_MiiInUse)
                    gConsoleUIController.RequestErrorMessage(cMiiInUseTitleId, cMiiInUseTextId, &messageParameter, 1, 0xff, NULL, NULL, NULL, true);
                else
                    gConsoleUIController.RequestErrorMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK, &messageParameter, 1, 0xff, NULL, NULL, NULL, true);
            }
            else
            {
                if (reason == eJoinFailedReason_Full)
                    gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_GameFull);
                else if (reason == eJoinFailedReason_MiiInUse)
                    gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_AgeRestricted);
                LeaveRoom(true);
            }
            break;
        }
        case cChannelLeave:
        {
            unsigned int playerMask;
            std::memcpy(&playerMask, packet + 1, sizeof(playerMask));
            RemoveRemotePlayersAndSync(stationId, playerMask);
            if (stationId == m_hostStationId && (playerMask & 1) != 0)
                SetState(eNQRNetworkManagerInternalState_Ending);
            break;
        }
        case cChannelJoin:
        {
            ClientJoinData joinData;
            std::memcpy(&joinData, packet + 1, sizeof(joinData));
            EJoinFailedReason reason;
            if (!AddRemotePlayersAndSync(stationId, joinData, &reason))
                SendJoinFailedData(stationId, joinData.playerMask, reason);
            break;
        }
        case cChannelRoomSync:
            std::memcpy(m_roomPlayers, packet + 1, sizeof(m_roomPlayers));
            MapRoomSlotPlayers(-1);
            break;
        }
        result = protocol->Receive(&stationId, packet, &size, cReceiveBufferSize);
    }
    std::free(packet);
}

void NQRNetworkManager_Lan_WiiU::SendSaveTransferDataPacket(void* data, int size)
{
    RemoteDataSend(0, cChannelSaveTransferData, data, static_cast<unsigned int>(size));
}

void NQRNetworkManager_Lan_WiiU::TickSaveTransferData()
{
    if (m_saveTransferState == eSaveTransfer_Transferring)
    {
        if (!m_saveTransferPacketPending)
        {
            int packetSize = m_saveTransferSize - m_saveTransferOffset;
            if (packetSize > cSaveTransferPacketSize)
                packetSize = cSaveTransferPacketSize;
            SendSaveTransferDataPacket(m_saveTransferData + m_saveTransferOffset, packetSize);
            m_saveTransferPacketPending = true;
            m_saveTransferOffset += packetSize;
        }
        if (m_internalState == eNQRNetworkManagerInternalState_Idle || m_connectedStations.size() == 0)
            m_saveTransferState = eSaveTransfer_ConnectionLost;
    }
    if (m_saveTransferState == eSaveTransfer_Joined)
    {
        if (m_internalState == eNQRNetworkManagerInternalState_Idle || m_connectedStations.size() == 0)
            m_saveTransferState = eSaveTransfer_ConnectionLost;
    }
}

void NQRNetworkManager_Lan_WiiU::ShowErrorViewer()
{
    if (s_lastNetworkError != 0)
    {
        g_pInput->RequestErrorViewer(0, s_lastNetworkError, NULL, NULL);
        if (s_lastNetworkError == cErrorCode118_0008)
            s_piaShutdownPending = true;
        s_lastNetworkError = 0;
    }
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
    switch (m_internalState)
    {
    case eNQRNetworkManagerInternalState_InitialiseFailed:
        ShowErrorViewer();
        if (m_receiving && m_localGame)
        {
            SetState(eNQRNetworkManagerInternalState_Playing);
            break;
        }
        m_unknownDD5 = true;
        SetState(eNQRNetworkManagerInternalState_Idle);
        break;
    case eNQRNetworkManagerInternalState_HostCreateSessionFailed:
        ShowErrorViewer();
        gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    case eNQRNetworkManagerInternalState_JoinNoMatchmakeSessionFound:
        m_nextIdleReasonIsFailure = true;
        SetState(eNQRNetworkManagerInternalState_Idle);
        break;
    case eNQRNetworkManagerInternalState_JoinSessionFailed:
    case eNQRNetworkManagerInternalState_JoinMatchmakeSessionFailed:
        if (s_lastPiaResult == cPiaResultDeniedByParticipants)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_StorageError);
        else if (s_lastPiaResult == cPiaResultSessionVoid)
            gApp.SetDisconnectReason(DisconnectPacket::eDisconnectReason_NetworkError);
        else
            ShowErrorViewer();
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    case eNQRNetworkManagerInternalState_LeaveRoomFailed:
        if (!m_host)
            RemoveLocalClientPlayers(cAllLocalPlayersMask);
        SetState(eNQRNetworkManagerInternalState_Ending);
        break;
    default:
        break;
    }
}


void NQRNetworkManager_Lan_WiiU::TickFromThread()
{
    checkPiaCoreToUse();
    TickSendQueue();
    OnlineCheck();
    FriendSearchTick();
    UpdateApplicationConnected();
    TickNexAsyncProcess();
    PollUnconnectedStations();
    PollReceiveData();
    TickSaveTransferData();
    ErrorHandlingTick();
    if (s_extDataRequested)
    {
        OSFastMutex* mutex = &m_friendSearchMutex;
        EnterCriticalSection(mutex);
        s_extDataRequested = false;
        bool found = false;
        for (unsigned int index = 0; index < m_friendSearchResults.size(); ++index)
        {
            if (m_friendSearchResults[index].sessionId == s_extDataSessionId)
            {
                std::memcpy(m_extDataBuffer, m_friendSearchResults[index].gameSessionData, sizeof(GameSessionData));
                m_extDataCallback(true, m_extDataCallbackData);
                found = true;
                break;
            }
        }
        if (!found)
            m_extDataCallback(false, m_extDataCallbackData);
        LeaveCriticalSection(mutex);
    }
    if (s_connectNetworkErrorPending)
    {
        s_connectNetworkErrorPending = false;
        int parameter = static_cast<int>(IDS_CONFIRM_OK);
        gConsoleUIController.RequestErrorMessage(IDS_ERROR_NETWORK_TITLE, IDS_ERROR_NETWORK,
            &parameter, 1, cAnyPad, NULL, NULL, NULL, true);
    }
    if (s_piaShutdownPending)
        Pia_Shutdown();
    if (s_shutdownTickStartTime > 0 && System::processTimeInMilliSecs() - s_shutdownTickStartTime > cShutdownTickDuration)
    {
        s_shutdownTickPending = false;
        s_shutdownTickStartTime = 0;
    }
    if (m_roomDataUpdateCountdown != 0)
    {
        if (m_internalState == eNQRNetworkManagerInternalState_Playing)
        {
            if (--m_roomDataUpdateCountdown == 0)
                UpdateExternalRoomData();
        }
        else
            m_roomDataUpdateCountdown = 0;
    }
    if (m_internalState == eNQRNetworkManagerInternalState_JoinRoomReady && m_localPlayersJoined == m_localPlayerCount)
    {
        UpdateRichPresenceCustomData(&s_presenceInfo);
        SetState(eNQRNetworkManagerInternalState_StartingGame);
        SetState(eNQRNetworkManagerInternalState_Playing);
    }
    if (m_internalState == eNQRNetworkManagerInternalState_Ending)
    {
        OSFastMutex* mutex = &m_connectedStationMutex;
        EnterCriticalSection(mutex);
        if (m_connectedStations.size() == 0)
            ResetToIdle();
        LeaveCriticalSection(mutex);
    }
    EnterCriticalSection(&m_stateChangeLock);
    while (m_stateChanges.size() > 0)
    {
        if (m_listener != NULL)
        {
            m_listener->HandleStateChange(m_stateChanges.front().eOldState, m_stateChanges.front().eNewState,
                                          m_stateChanges.front().bError);
            if (m_stateChanges.front().eNewState == eNQRNetworkManagerState_Idle)
                m_receiving = false;
        }
        m_state = m_stateChanges.front().eNewState;
        m_stateChanges.pop_front();
    }
    LeaveCriticalSection(&m_stateChangeLock);
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
    static_cast<NQRNetworkManager_Lan_WiiU*>(context)->RunTickThread();
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
    s_lastNetworkError = 0;
    m_tickEvent = new C4JEventImpl(C4JEvent::Mode0);
    m_tickThread = new C4JThreadImpl(TickThreadProc, this, "NQRNetworkManager_Lan_WiiU::TickThreadProc", 0);
    m_tickThread->SetProcessor(s_tickThreadProcessor);
    m_tickThread->Run();
    SetState(eNQRNetworkManagerInternalState_Idle);
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
    return m_internalState == eNQRNetworkManagerInternalState_HostRoomReady ||
        m_internalState == eNQRNetworkManagerInternalState_Playing ||
        m_internalState == eNQRNetworkManagerInternalState_Idle;
}

bool NQRNetworkManager_Lan_WiiU::IsInSession()
{
    return m_receiving;
}

bool NQRNetworkManager_Lan_WiiU::IsMatchmakingEnabled()
{
    return m_matchmaking;
}

void NQRNetworkManager_Lan_WiiU::CreateAndJoinRoom(int userIndexMask, int maxPlayers, void* extData, int extDataSize, int* attributes, int attributeCount, bool matchmaking, unsigned char gameMode)
{
    m_receiving = true;
    m_localPlayerCount = userIndexMask;
    m_localPlayersJoined = maxPlayers;
    m_roomAttributes = attributes;
    m_roomAttributeCount = attributeCount;
    m_matchmaking = matchmaking;
    m_gameMode = gameMode;
    if (extData != NULL && extDataSize >= static_cast<int>(sizeof(PresenceSyncInfo)))
        SetNQRPresenceInfoFromExtData(&s_presenceInfo, extData);
    UpdateRichPresenceCustomData(&s_presenceInfo);
    m_host = true;
    SetState(eNQRNetworkManagerInternalState_HostWaitingForLogin);
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
    OSFastMutex* mutex = &m_friendSearchMutex;
    EnterCriticalSection(mutex);
    int count = static_cast<int>(m_friendSearchResults.size());
    LeaveCriticalSection(mutex);
    return count;
}

void NQRNetworkManager_Lan_WiiU::FriendRoomManagerGetRoomInfo(int index, SessionSearchResult* result)
{
    OSFastMutex* mutex = &m_friendSearchMutex;
    EnterCriticalSection(mutex);
    result->m_uiOwnerPrincipalId = m_friendSearchResults[index].ownerPrincipalId;
    std::memcpy(result->m_szOwnerName, m_friendSearchResults[index].ownerName, sizeof(result->m_szOwnerName));
    result->m_uiOwnerLocalFriendIndex = m_friendSearchResults[index].ownerLocalFriendIndex;
    result->m_pExtData = m_friendSearchResults[index].extData;
    result->m_uiSessionId = m_friendSearchResults[index].sessionId;
    LeaveCriticalSection(mutex);
}

NQRNetworkManager::SessionSearchResult* NQRNetworkManager_Lan_WiiU::GetFriendRoomSearchResults(int* count)
{
    OSFastMutex* mutex = &m_friendSearchMutex;
    EnterCriticalSection(mutex);
    int resultCount = FriendRoomManagerGetCount();
    SessionSearchResult* results = new SessionSearchResult[resultCount];
    for (int index = 0; index < resultCount; ++index)
        FriendRoomManagerGetRoomInfo(index, &results[index]);
    *count = FriendRoomManagerGetCount();
    m_friendPresenceUpdated = false;
    LeaveCriticalSection(mutex);
    return results;
}

bool NQRNetworkManager_Lan_WiiU::JoinRoom(SessionSearchResult* result, int userIndexMask)
{
    SetNQRPresenceInfoFromExtData(&s_presenceInfo, result->m_pExtData);
    return JoinRoom(result->m_uiSessionId, userIndexMask, NULL);
}

bool NQRNetworkManager_Lan_WiiU::JoinRoom(unsigned int sessionId, int userIndexMask, const PresenceSyncInfo* presenceInfo)
{
    if (m_internalState == eNQRNetworkManagerInternalState_Idle)
        SetState(eNQRNetworkManagerInternalState_JoinRequested);
    if (presenceInfo != NULL)
        std::memcpy(&s_presenceInfo, presenceInfo, sizeof(PresenceSyncInfo));
    m_roomPlayerMask = userIndexMask;
    m_pendingSessionId = sessionId;
    m_localPlayersJoined = 0;
    m_host = false;
    m_receiving = true;
    m_hostStationIdCached = false;
    m_localGame = false;
    m_localPlayerCount = 0;
    for (int userIndex = 0; userIndex < 4; ++userIndex)
    {
        if (userIndexMask & (1 << userIndex))
            ++m_localPlayerCount;
    }
    Pia_SignalJoinSession();
    return true;
}

bool NQRNetworkManager_Lan_WiiU::JoinRoom(int, const nn::fp::FriendPresence*)
{
    return false;
}

bool NQRNetworkManager_Lan_WiiU::FriendPresenceUpdatedRoomList()
{
    return m_friendPresenceUpdated;
}

void NQRNetworkManager_Lan_WiiU::GetExtDataForRoom(unsigned int sessionId, void* extData, void (*callback)(bool, void*), void* callbackData)
{
    m_extDataCallback = callback;
    m_extDataCallbackData = callbackData;
    m_extDataBuffer = extData;
    s_extDataRequested = true;
    s_extDataSessionId = sessionId;
}

bool NQRNetworkManager_Lan_WiiU::FindAndJoinRoom(int, unsigned int, CMatchMakeSearchParam*, unsigned char)
{
    return true;
}



void NQRNetworkManager_Lan_WiiU::GetFriendsThreadProc(void* context)
{
    static_cast<NQRNetworkManager_Lan_WiiU*>(context)->GetFriendsFromSessionBrowse();
}

nn::pia::Result NQRNetworkManager_Lan_WiiU::Session_BrowseSession(nn::pia::session::SessionSearchCriteria* criteria)
{
    nn::pia::Result result = 0;
    if (nn::pia::session::g_piaSession == NULL)
        return cPiaResultSessionNotCreated;
    while (m_nexAsyncProcess != eNEXAsyncProcess_None)
        Sleep(cBrowseSessionPollInterval);
    SetNexAsyncProcess(eNEXAsyncProcess_BrowseSession);
    result = nn::pia::session::g_piaSession->BrowseSessionAsync(criteria);
    if (result != 0)
    {
        AssertPiaError(result);
        return result;
    }
    while (nn::pia::session::g_piaSession != NULL)
    {
        if (nn::pia::session::g_piaSession->IsBrowseSessionCompleted())
        {
            result = nn::pia::session::g_piaSession->GetBrowseSessionResult();
            if (result != 0)
                AssertPiaError(result);
            SetNexAsyncProcess(eNEXAsyncProcess_None);
            return result;
        }
        Sleep(cBrowseSessionPollInterval);
    }
    SetNexAsyncProcess(eNEXAsyncProcess_None);
    return cPiaResultSessionDestroyed;
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
    m_presenceGameMode.joinAvailabilityFlag = FALSE;
    m_presenceGameMode.matchmakeSystemType = cPresenceMatchmakeSystemType;
    m_presenceGameMode.joinGameId = cPresenceJoinGameId;
    m_presenceGameMode.joinGameMode = 0;
    m_presenceGameMode.ownerPrincipalId = 0;
    m_presenceGameMode.joinGroupId = 0;
    std::memset(m_presenceGameMode.applicationArg, 0, sizeof(PresenceSyncInfo));
    gProfile->SetRichPresenceGameModeData(m_presenceGameMode);
}

void NQRNetworkManager_Lan_WiiU::LeaveRoom(bool actuallyLeaveRoom)
{
    if (m_localGame)
    {
        if (m_internalState != eNQRNetworkManagerInternalState_Playing)
            return;
        SetState(eNQRNetworkManagerInternalState_LeavingRoom);
        SetState(eNQRNetworkManagerInternalState_Ending);
        ResetToIdle();
        return;
    }
    if (m_internalState == eNQRNetworkManagerInternalState_LeavingRoom)
        return;
    ClearRichPresenceCustomData();
    if (actuallyLeaveRoom)
    {
        switch (m_internalState)
        {
        case eNQRNetworkManagerInternalState_HostSessionCreated:
        case eNQRNetworkManagerInternalState_HostRoomReady:
        case eNQRNetworkManagerInternalState_JoinRoomReady:
        case eNQRNetworkManagerInternalState_Playing:
            SetState(eNQRNetworkManagerInternalState_LeavingRoom);
            if (m_inPiaSession)
                Pia_SignalLeaveSession();
            else
                SetState(eNQRNetworkManagerInternalState_Ending);
            break;
        default:
            if (m_internalState == eNQRNetworkManagerInternalState_HostCreatingSession)
                SetState(eNQRNetworkManagerInternalState_Ending);
            else
                SetState(eNQRNetworkManagerInternalState_Idle);
            break;
        }
        if (m_host)
        {
            OSFastMutex* mutex = &m_connectedStationMutex;
            EnterCriticalSection(mutex);
            while (m_connectedStations.size() != 0)
            {
                RemoveRemotePlayersAndSync(m_connectedStations[0].stationId, cAllLocalPlayersMask);
                RemoveConnectedStation(m_connectedStations[0].stationId);
            }
            LeaveCriticalSection(mutex);
        }
        else
            RemoveAllPlayersOnClient();
    }
    else
        SetState(eNQRNetworkManagerInternalState_Ending);
}

void NQRNetworkManager_Lan_WiiU::EndGame()
{
}

bool NQRNetworkManager_Lan_WiiU::SessionHasSpace(int extraPlayers)
{
    return m_roomPlayers[0].m_uiPlayerCountMinusOne + extraPlayers + 1 <= static_cast<int>(CGameNetworkManager::sInstance.GetMaxPlayers());
}

bool NQRNetworkManager_Lan_WiiU::SessionHasMii(int userIndex)
{
    NQRNetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
    return player != NULL && player->GetMiiName()[0] != 0;
}

void NQRNetworkManager_Lan_WiiU::UpdateRemotePlay()
{
    int localPlayerCount = 0;
    for (int userIndex = 0; userIndex < 4; ++userIndex)
    {
        if (GetLocalPlayerByUserIndex(userIndex) != NULL)
            ++localPlayerCount;
    }
    g_pInput->SetLocalMultiplayer(localPlayerCount > 1);
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
    if (!m_localGame)
        RemoteDataSend(m_hostStationId, 0xfd, &playerMask, sizeof(playerMask));
}

bool NQRNetworkManager_Lan_WiiU::RemoveLocalPlayerByUserIndex(int userIndex)
{
    NQRNetworkPlayer* player = GetLocalPlayerByUserIndex(userIndex);
    if (player == NULL)
        return false;
    SendLeaveData(1u << userIndex);
    if (m_listener != NULL)
        m_listener->HandlePlayerLeaving(player);
    RemoveConnectedPlayer(player);
    delete player;
    MapRoomSlotPlayers(-1);
    SyncRoomData();
    UpdateRemotePlay();
    return true;
}


int NQRNetworkManager_Lan_WiiU::GetPlayerCount()
{
    return m_roomPlayers[0].m_uiPlayerCountMinusOne + 1;
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
    if (index < 8)
        return GetPlayerIfReady(m_roomSlotPlayers[index]);
    return NULL;
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

std::wstring NQRNetworkManager_Lan_WiiU::GetDisplayNameByGamertag(std::wstring gamertag)
{
    if (m_miiNames.find(gamertag) != m_miiNames.end())
        return m_miiNames[gamertag];
    return gamertag;
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
    if (m_listener != NULL)
        m_listener->HandleDataReceived(from, to, static_cast<unsigned char*>(const_cast<void*>(data)), size);
}

void NQRNetworkManager_Lan_WiiU::RemoteDataSend(NQRNetworkPlayer* from, NQRNetworkPlayer* to, const void* data, unsigned int size)
{
    unsigned char channel = from->GetLocalPlayerIndex();
    RemoteDataSend(to->GetStationId(), channel, data, size);
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
    if (m_localGame)
    {
        ClearRichPresenceCustomData();
        return;
    }
    PresenceSyncInfo presenceInfo;
    SetNQRPresenceInfoFromExtData(&presenceInfo, m_externalData);
    UpdateRichPresenceCustomData(&presenceInfo);
}

int NQRNetworkManager_Lan_WiiU::GetJoiningReadyPercentage()
{
    if (m_internalState == eNQRNetworkManagerInternalState_HostWaitingForLogin ||
        m_internalState == eNQRNetworkManagerInternalState_JoinWaitingForLogin)
        return 50;
    return 100;
}

void NQRNetworkManager_Lan_WiiU::SignalInitialiseAfterOnline()
{
    s_initialiseAfterOnline = true;
}

unsigned int NQRNetworkManager_Lan_WiiU::GetSaveTransferConnectionFromSessionBrowse()
{
    EnterCriticalSection(&m_friendSearchMutex);
    nn::pia::Result result = 0;
    m_friendSearchResults.clear();
    m_friendSearchStatus = 0;
    if (IsInitialisedAfterOnline())
    {
        nn::pia::lan::LanSessionSearchCriteria criteria;
        criteria.SetMaxParticipantNum(8);
        criteria.SetVacantOnly(true);
        criteria.SetOpenedOnly(true);
        criteria.SetResultRange(0, 1);
        result = Session_BrowseSession(&criteria);
        if (result != 0)
            AssertPiaError(result);
        else
        {
            nn::pia::session::ISessionInfoList* sessionInfoList = nn::pia::session::g_piaSession->GetBrowsedSessionInfoList();
            if (sessionInfoList != NULL)
            {
                m_saveTransferReceiveSize = 0;
                bool found = false;
                int sessionCount = sessionInfoList->GetSize();
                nn::pia::session::ISessionInfo** sessionInfos = sessionInfoList->Begin();
                for (int index = 0; index < sessionCount; ++index)
                {
                    nn::pia::session::ISessionInfo* sessionInfo = *sessionInfos;
                    result = sessionInfo->GetApplicationData(m_saveTransferDescriptor, sizeof(SaveTransferDescriptor));
                    if (result != 0)
                        AssertPiaError(result);
                    else
                    {
                        unsigned int sessionId = sessionInfo->GetSessionId();
                        m_saveTransferSessionInfo.Copy(*static_cast<nn::pia::lan::LanSessionInfo*>(sessionInfo));
                        if (m_saveTransferDescriptor->version == cSaveTransferDescriptorVersion)
                        {
                            m_saveTransferSessionId = sessionId;
                            m_saveTransferSessionFound = true;
                            m_saveTransferReceiveSize = m_saveTransferDescriptor->dataSize;
                            found = true;
                        }
                        else
                            m_saveTransferSessionFound = false;
                    }
                }
                m_friendPresenceUpdated = false;
                m_saveTransferState = found ? eSaveTransfer_SessionFound : eSaveTransfer_NoSessionFound;
            }
        }
    }
    LeaveCriticalSection(&m_friendSearchMutex);
    return 0;
}

void NQRNetworkManager_Lan_WiiU::GetSaveTransferConnectionThreadProc(void* context)
{
    static_cast<NQRNetworkManager_Lan_WiiU*>(context)->GetSaveTransferConnectionFromSessionBrowse();
}

void NQRNetworkManager_Lan_WiiU::Pia_ShutdownForOnline()
{
    s_shutdownForOnline = true;
    if (s_piaInitialised)
    {
        s_shutdownTickPending = true;
        s_piaShutdownPending = true;
    }
}

void NQRNetworkManager_Lan_WiiU::CreateSessionForSaveTransfer(int dataSize, void* data, int metadataSize, void* metadata, long long seed, unsigned int hostOptions, unsigned int texturePack, wchar_t* name)
{
    m_saveTransferSize = dataSize + metadataSize;
    m_saveTransferData = static_cast<unsigned char*>(std::malloc(m_saveTransferSize));
    std::memcpy(m_saveTransferData, data, dataSize);
    std::memcpy(m_saveTransferData + dataSize, metadata, metadataSize);
    m_saveTransferOffset = 0;

    std::memset(m_saveTransferDescriptor, 0, sizeof(SaveTransferDescriptor));
    m_saveTransferDescriptor->kind = cSaveTransferDescriptorKind;
    m_saveTransferDescriptor->version = cSaveTransferDescriptorVersion;
    m_saveTransferDescriptor->dataSize = dataSize;
    m_saveTransferDescriptor->metadataSize = metadataSize;
    m_saveTransferDescriptor->seed = seed;
    m_saveTransferDescriptor->hostOptions = hostOptions;
    m_saveTransferDescriptor->texturePack = texturePack;
    for (int index = 0; index < cSaveTransferNameLength && name[index] != 0; ++index)
        m_saveTransferDescriptor->name[index] = name[index];

    m_receiving = true;
    m_hostStationIdCached = false;
    m_host = true;
    m_externalData = m_saveTransferDescriptor;
    m_externalDataSize = sizeof(SaveTransferDescriptor);
    m_gameMode = cSaveTransferGameMode;
    Pia_SignalCreateSession();
    m_saveTransferState = eSaveTransfer_CreatingSession;
}

unsigned int NQRNetworkManager_Lan_WiiU::FindSessionForSaveTransfer()
{
    SetFriendSearchState(eNQRNetworkManagerFriendSearchState_Searching);
    m_saveTransferState = eSaveTransfer_Searching;
    return GetFriendsFromSessionBrowse();
}

void NQRNetworkManager_Lan_WiiU::JoinSaveTransferConnection()
{
    m_saveTransferState = eSaveTransfer_Joining;
    JoinRoom(m_saveTransferSessionId, 0, NULL);
    m_saveTransferReceiveData = static_cast<unsigned char*>(std::malloc(m_saveTransferReceiveSize));
    m_saveTransferReceiveOffset = 0;
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
    if (!IsInitialisedAfterOnline() && !s_piaShutdownPending)
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
