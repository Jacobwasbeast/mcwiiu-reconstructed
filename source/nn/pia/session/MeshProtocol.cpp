#include "nn/pia/session/MeshProtocol.hpp"

#include <cstring>
#include <new>

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Memory.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/common/WatermarkManager.hpp"
#include "nn/pia/session/DestroyMeshJob.hpp"
#include "nn/pia/session/KickoutManageJob.hpp"
#include "nn/pia/session/Mesh.hpp"
#include "nn/pia/session/MeshEventListener.hpp"
#include "nn/pia/session/ProcessDestroyMeshJob.hpp"
#include "nn/pia/session/ProcessHostMigrationJob.hpp"
#include "nn/pia/session/ProcessJoinRequestJob.hpp"
#include "nn/pia/session/Session.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/RelayRouteManager.hpp"
#include "nn/pia/transport/ReliableSlidingWindow.hpp"
#include "nn/pia/transport/ResendingMessageManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationProtocol.hpp"
#include "nn/pia/transport/ThreadStreamManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace session {

namespace
{
enum MeshProtocolResult
{
    cResultInvalidArgument = 0x10c07,
    cResultInvalidState = 0x10c08
};

enum
{
    cStationIndexNone = 0xfd,
    cStationNumMax = transport::StationManager::cStationNumMax,
    cStationIndexMax = cStationNumMax - 1,
    cWatermarkSendBuffer = 7,
    cWatermarkReceiveBuffer = 8,
    cUpdateMeshHeaderSize = 12,
    cSystemMessageHeaderSize = 4,
    cJoinResponseHeaderSize = 16,
    cJoinResponsePartHeaderSize = 8,
    cMultiMigrationMessageSize = 44,
    cRankDecisionMessageBufferSize = 16,
    cRankDecisionMessageSize = 13,
    cAckIdSize = 4,
    cRankDecisionReceivedSize = cRankDecisionMessageSize + cAckIdSize,
    cStationEntryAlignMask = 3,
    cDefaultUpdateInterval = 5000,
    cBroadcastUpdateInterval = 2000,
    cDefaultUnknown24 = 10000,
    cTraceStationLevel = 0x20000000,
    cTraceStationJoinLevel = 0x10000000,
    cTraceStationUpdateLevel = 0x8000000,
    cTraceStationMigrationLevel = 0x40000000,
    cTraceRelayRouteLevel = 0x1000000,
    cTraceTableLevel = 0x4000000,
    cTraceMessageLevel = 0x4000000,
    cStationIndexBroadcast = 0xff,
    cSlidingWindowSendCapacity = 4,
    cSlidingWindowReceiveCapacity = 4,
    cStationReconnectState = 6
};

inline transport::StationPacketHandler* handlerOf(unsigned int value)
{
    return reinterpret_cast<transport::StationPacketHandler*>(static_cast<uintptr_t>(value));
}

inline void updateSendWatermark(const transport::ReliableSlidingWindow& window)
{
    if (transport::g_piaWatermarkManager != 0)
    {
        unsigned int sendCount = window.m_sendBuffer.m_count;
        transport::g_piaWatermarkManager->GetWatermark(cWatermarkSendBuffer)->Update(sendCount);
    }
}
}

MeshProtocol::MeshProtocol()
    : transport::Protocol(), m_waitingJoinRequestJob(0), m_waitingJoinResponseJob(0),
      m_waitingLeaveRequestJob(0), m_waitingLeaveResponseJob(0),
      m_unknown24(cDefaultUnknown24), m_unknown28(cDefaultUpdateInterval), m_unknown30(0),
      m_windowNum(0), m_windows(0), m_unknown40(0), m_bufferSize(0), m_buffer(0),
      m_updateMeshBufferSize(0), m_updateMeshBuffer(0), m_updateMeshBufferValid(0),
      m_nextTimeNum(0), m_nextTimes(0), m_broadcastSupported(0)
{
    if (transport::g_piaWatermarkManager != 0)
    {
        transport::g_piaWatermarkManager->GetWatermark(cWatermarkSendBuffer)
            ->SetName("MeshProtocolReliable send buffer num");
        if (transport::g_piaWatermarkManager != 0)
        {
            transport::g_piaWatermarkManager->GetWatermark(cWatermarkReceiveBuffer)
                ->SetName("MeshProtocolReliable receive buffer num");
        }
    }
}

MeshProtocol::~MeshProtocol()
{
}

void MeshProtocol::Trace(unsigned long long value) const
{
}

unsigned short MeshProtocol::GetProtocolType() const
{
    return cProtocolType;
}

Result MeshProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    return transport::Protocol::StartupWithStationAddress(address);
}

Result MeshProtocol::StartupNative()
{
    m_waitingLeaveResponseJob = 0;
    m_waitingJoinResponseJob = 0;
    m_waitingJoinRequestJob = 0;
    m_waitingLeaveRequestJob = 0;
    m_unknown38 = 0;
    unsigned int broadcastSupported =
        transport::g_piaThreadStreamManager->m_packetOutput->IsBroadcastSupported();
    m_unknown28 = broadcastSupported != 0 ? cBroadcastUpdateInterval : cDefaultUpdateInterval;
    m_broadcastSupported = broadcastSupported;
    return 0;
}

common::StepSequenceJob* MeshProtocol::GetWaitingJoinRequestJob()
{
    return m_waitingJoinRequestJob;
}

common::StepSequenceJob* MeshProtocol::GetWaitingJoinResponseJob()
{
    return m_waitingJoinResponseJob;
}

common::StepSequenceJob* MeshProtocol::GetWaitingLeaveResponseJob()
{
    return m_waitingLeaveResponseJob;
}

void MeshProtocol::SetWaitingJoinRequestJob(common::StepSequenceJob* job)
{
    m_waitingJoinRequestJob = job;
}

void MeshProtocol::SetWaitingJoinResponseJob(common::StepSequenceJob* job)
{
    m_waitingJoinResponseJob = job;
}

void MeshProtocol::SetWaitingLeaveResponseJob(common::StepSequenceJob* job)
{
    m_waitingLeaveResponseJob = job;
}

unsigned int MeshProtocol::GetJoinRequestDataSize() const
{
    return transport::g_piaStationManager->m_localStation->m_stationAddress.GetSerializedSize() +
           cSystemMessageHeaderSize;
}

void MeshProtocol::MakeJoinRequestData(unsigned char* buffer)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    buffer[0] = cMessageTypeJoinRequest;
    buffer[1] = static_cast<unsigned char>(localStation->m_field10);
    buffer[2] = 0;
    buffer[3] = 0;
    unsigned int size = 0;
    localStation->m_stationAddress.Serialize(buffer + cSystemMessageHeaderSize, &size,
                                             GetJoinRequestDataSize() - cSystemMessageHeaderSize);
}

void MeshProtocol::Finalize()
{
    if (m_buffer != 0)
    {
        common::DeleteArray<unsigned char>(m_buffer);
        m_buffer = 0;
    }
    m_bufferSize = 0;
    if (m_updateMeshBuffer != 0)
    {
        common::DeleteArray<unsigned char>(m_updateMeshBuffer);
        m_updateMeshBuffer = 0;
    }
    transport::ReliableSlidingWindow* windows = m_windows;
    m_updateMeshBufferSize = 0;
    if (windows != 0)
    {
        unsigned int count = pead::getArraySize(windows) / sizeof(transport::ReliableSlidingWindow);
        transport::ReliableSlidingWindow* window = windows;
        while (count != 0)
        {
            window->~ReliableSlidingWindow();
            ++window;
            --count;
        }
        pead::freeToSeadHeap(windows);
        m_windows = 0;
    }
    m_windowNum = 0;
    if (m_nextTimes != 0)
    {
        pead::getArraySize(m_nextTimes);
        pead::freeToSeadHeap(m_nextTimes);
        m_nextTimes = 0;
    }
    m_nextTimeNum = 0;
}

Result MeshProtocol::Initialize()
{
    unsigned int windowNum =
        static_cast<unsigned short>(transport::g_piaTransport->m_stationNum - 1);
    m_windowNum = windowNum;
    pead::Heap* heap = common::HeapManager::GetHeap();
    transport::ReliableSlidingWindow* windows = static_cast<transport::ReliableSlidingWindow*>(
        pead::allocateFromSeadHeap(windowNum * sizeof(transport::ReliableSlidingWindow), heap));
    if (windows == 0)
    {
        m_windows = 0;
    }
    else
    {
        transport::ReliableSlidingWindow* window = windows;
        for (; windowNum != 0; --windowNum)
        {
            new (window) transport::ReliableSlidingWindow();
            ++window;
        }
        m_windows = windows;
    }

    for (unsigned short index = 0; index < m_windowNum; ++index)
    {
        Result result = m_windows[index].Initialize(cSlidingWindowSendCapacity,
                                                   cSlidingWindowReceiveCapacity);
        if (result != 0)
        {
            Finalize();
            return result;
        }
    }

    unsigned int timeNum =
        static_cast<unsigned short>(transport::g_piaTransport->m_stationNum - 1);
    m_nextTimeNum = timeNum;
    heap = common::HeapManager::GetHeap();
    common::Time* times = static_cast<common::Time*>(
        pead::allocateFromSeadHeap(timeNum * sizeof(common::Time), heap));
    if (times == 0)
    {
        m_nextTimes = 0;
    }
    else
    {
        common::Time* time = times;
        for (; timeNum != 0; --timeNum)
        {
            new (time) common::Time();
            ++time;
        }
        m_nextTimes = times;
    }

    for (unsigned short index = 0; index < m_nextTimeNum; ++index)
    {
        common::Time now;
        now.SetNow();
        m_nextTimes[index] = now;
    }

    unsigned int infoSize = transport::StationConnectionInfo().GetSerializedSize();
    unsigned int bufferSize = (infoSize + (~infoSize & cStationEntryAlignMask) + 1) *
                                  transport::g_piaTransport->m_stationNum +
                              cJoinResponseHeaderSize;
    m_bufferSize = bufferSize;
    m_updateMeshBufferSize = bufferSize;
    m_buffer = common::NewArray<unsigned char>(bufferSize);
    m_updateMeshBuffer = common::NewArray<unsigned char>(m_updateMeshBufferSize);
    return 0;
}

Result MeshProtocol::UpdateProtocolEvent(const transport::ProtocolEvent& event)
{
    transport::Station* eventStation =
        transport::g_piaStationManager->GetStation(event.m_stationAddress);
    int eventIndex = static_cast<int>(eventStation->m_field10);
    StationIndex localIndex = g_piaMesh->GetLocalStationIndex();
    transport::Station* localStation = transport::g_piaStationManager->GetStation(localIndex);
    unsigned long long localStationId =
        (static_cast<unsigned long long>(localStation->m_field18) << 32) | localStation->m_field1c;
    if (static_cast<int>(localIndex) > cStationIndexMax)
        return cResultInvalidState;
    if (eventIndex < 0 || eventIndex >= static_cast<int>(m_windowNum) + 1)
        return cResultInvalidArgument;

    transport::ReliableSlidingWindow* window;
    if (eventIndex < static_cast<int>(localIndex))
        window = m_windows + eventIndex;
    else if (static_cast<int>(localIndex) < eventIndex)
        window = m_windows + (eventIndex - 1);
    else
        return cResultInvalidArgument;

    if (event.m_type == 0)
    {
        unsigned int protocolType = GetProtocolType();
        Result result = window->Startup(
            handlerOf(GetStationPacketHandler()), ((protocolType & 0xff) << 8) | 1,
            localStationId,
            (static_cast<unsigned long long>(eventStation->m_field18) << 32) |
                eventStation->m_field1c);
        return result;
    }
    if (event.m_type == 1)
    {
        window->Cleanup();
        return 0;
    }
    return cResultInvalidArgument;
}

unsigned char MeshProtocol::GetSystemMessagePartitionNum(unsigned char messageType,
                                                         unsigned char stationCount)
{
    unsigned int infoSize = transport::StationConnectionInfo().GetSerializedSize();
    unsigned int entrySize = infoSize + (~infoSize & cStationEntryAlignMask) + 1;
    unsigned int payloadLimit = handlerOf(mStationPacketHandler)->GetPayloadSizeLimit();
    unsigned int messageSize = 0;
    if (stationCount > 0 && stationCount <= cStationNumMax)
    {
        for (int perMessage = stationCount; perMessage > 0; --perMessage)
        {
            if (messageType == cMessageTypeJoinResponse)
                messageSize = entrySize * perMessage + cJoinResponseHeaderSize;
            else if (messageType == cMessageTypeUpdateMesh)
                messageSize = entrySize * perMessage + cUpdateMeshHeaderSize;
            if (messageSize <= payloadLimit)
            {
                return stationCount / perMessage + (stationCount % perMessage != 0);
            }
        }
    }
    return 0;
}

void MeshProtocol::SendLeaveRequest()
{
    transport::Station* hostStation =
        transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
    if (hostStation == 0)
        return;
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation == 0)
        return;

    int hostIndex = static_cast<int>(hostStation->m_field10);
    int localIndex = static_cast<int>(localStation->m_field10);
    if (hostIndex <= cStationIndexMax && localIndex <= cStationIndexMax)
    {
        transport::ReliableSlidingWindow* window;
        if (hostIndex < localIndex)
            window = m_windows + hostIndex;
        else if (localIndex < hostIndex)
            window = m_windows + (hostIndex - 1);
        else
            return;

        if (window->IsInCommunication())
        {
            m_buffer[0] = cMessageTypeLeaveRequest;
            m_buffer[1] = static_cast<unsigned char>(localIndex);
            m_buffer[3] = 0;
            m_buffer[2] = 0;
            unsigned int size = 0;
            localStation->m_stationAddress.Serialize(m_buffer + cSystemMessageHeaderSize, &size,
                                                     m_bufferSize - cSystemMessageHeaderSize);
            Result result = window->PushData(m_buffer, size + cSystemMessageHeaderSize);
            if (result == 0)
                updateSendWatermark(*window);
        }
    }
}

void MeshProtocol::SendLeaveResponse(transport::Station* station)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation == 0)
        return;

    unsigned int addressSize = localStation->m_stationAddress.GetSerializedSize();
    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer =
        handlerOf(mStationPacketHandler)->AssignByStationAddress(
            protocolId, station->m_stationAddress, addressSize + cSystemMessageHeaderSize, false,
            false);
    if (writer == 0)
    {
        station->Trace(cTraceStationJoinLevel);
        return;
    }

    m_buffer[0] = cMessageTypeLeaveResponse;
    m_buffer[1] = static_cast<unsigned char>(localStation->m_field10);
    m_buffer[3] = 0;
    m_buffer[2] = 0;
    unsigned int size;
    localStation->m_stationAddress.Serialize(m_buffer + cSystemMessageHeaderSize, &size,
                                             m_bufferSize - cSystemMessageHeaderSize);
    writer->SetPayload(m_buffer);
    Result result = handlerOf(mStationPacketHandler)->Commit();
    if (result == 0)
    {
        transport::ProtocolId reliableProtocolId(mProtocolId);
        writer = handlerOf(mStationPacketHandler)->AssignByStationAddress(
            reliableProtocolId, station->m_stationAddress, addressSize + cSystemMessageHeaderSize,
            true, false);
        if (writer != 0)
        {
            writer->SetPayload(m_buffer);
            handlerOf(mStationPacketHandler)->Commit();
        }
    }
}

void MeshProtocol::SendDenyingJoinResponse(const common::StationAddress& address,
                                           unsigned char reason)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    transport::ProtocolId protocolId(mProtocolId);
    transport::ProtocolMessageWriter* writer =
        handlerOf(mStationPacketHandler)->AssignByStationAddress(protocolId, address, 5, false,
                                                                 false);
    if (writer == 0)
    {
        address.Trace(cTraceStationUpdateLevel);
        return;
    }

    m_buffer[0] = cMessageTypeJoinResponse;
    m_buffer[1] = 0;
    m_buffer[2] = 0xff;
    m_buffer[3] = 0xff;
    m_buffer[4] = reason;
    writer->SetPayload(m_buffer);
    Result result = handlerOf(mStationPacketHandler)->Commit();
    if (result == 0)
    {
        transport::ProtocolId reliableProtocolId(mProtocolId);
        writer = handlerOf(mStationPacketHandler)->AssignByStationAddress(
            reliableProtocolId, address, 5, true, false);
        if (writer != 0)
        {
            writer->SetPayload(m_buffer);
            handlerOf(mStationPacketHandler)->Commit();
        }
    }
}

bool MeshProtocol::SendDestroyMesh(StationIndex stationIndex)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10) &&
            transport::g_piaStationManager->GetStation(stationIndex) != 0)
        {
            int windowIndex = static_cast<int>(stationIndex);
            if (localIndex <= windowIndex)
                --windowIndex;
            transport::ReliableSlidingWindow* window = m_windows + windowIndex;
            if (window->IsInCommunication())
            {
                localStation->m_stationAddress.GetSerializedSize();
                m_buffer[0] = cMessageTypeDestroyMesh;
                m_buffer[1] = static_cast<unsigned char>(localIndex);
                m_buffer[3] = 0;
                m_buffer[2] = 0;
                unsigned int size;
                localStation->m_stationAddress.Serialize(m_buffer + cSystemMessageHeaderSize,
                                                         &size,
                                                         m_bufferSize - cSystemMessageHeaderSize);
                Result result = window->PushData(m_buffer, size + cSystemMessageHeaderSize);
                if (result == 0)
                    updateSendWatermark(*window);
                return result == 0;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendDestroyResponse(StationIndex stationIndex)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax)
        {
            transport::Station* station = 0;
            if (localIndex == static_cast<int>(localStation->m_field10) &&
                (station = transport::g_piaStationManager->GetStation(stationIndex)) != 0)
            {
                transport::ProtocolId protocolId(mProtocolId);
                transport::ProtocolMessageWriter* writer =
                    handlerOf(mStationPacketHandler)->AssignByStationAddress(
                        protocolId, station->m_stationAddress, 2, false, false);
                if (writer == 0)
                {
                    station->Trace(cTraceStationUpdateLevel);
                    return false;
                }

                m_buffer[0] = cMessageTypeDestroyResponse;
                m_buffer[1] = static_cast<unsigned char>(localIndex);
                writer->SetPayload(m_buffer);
                Result result = handlerOf(mStationPacketHandler)->Commit();
                if (result == 0)
                {
                    transport::ProtocolId reliableProtocolId(mProtocolId);
                    writer = handlerOf(mStationPacketHandler)->AssignByStationAddress(
                        reliableProtocolId, station->m_stationAddress, 2, true, false);
                    if (writer != 0)
                    {
                        writer->SetPayload(m_buffer);
                        handlerOf(mStationPacketHandler)->Commit();
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendGreeting(StationIndex stationIndex)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        unsigned int localIndex = g_piaMesh->GetLocalStationIndex();
        if (static_cast<int>(localIndex) <= cStationIndexMax &&
            localIndex == localStation->m_field10)
        {
            if (transport::g_piaStationManager->GetStation(stationIndex) == 0)
            {
                transport::g_piaStationManager->Trace(cTraceStationUpdateLevel);
                return false;
            }

            transport::ProtocolId protocolId(mProtocolId);
            transport::ProtocolMessageWriter* writer =
                handlerOf(mStationPacketHandler)->AssignByStationIndex(protocolId, stationIndex, 2,
                                                                       false, false, true);
            if (writer != 0)
            {
                writer->WriteU8(cMessageTypeGreeting, 0);
                writer->WriteU8(localIndex & 0xff, 1);
                Result result = handlerOf(mStationPacketHandler)->Commit();
                return result == 0;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendMigrationFinish(bool succeeded)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10))
        {
            m_buffer[0] = cMessageTypeMigrationFinish;
            m_buffer[1] = static_cast<unsigned char>(localIndex);
            m_buffer[2] = succeeded;
            for (unsigned short index = 0; index < m_windowNum; ++index)
            {
                if (m_windows[index].IsInCommunication())
                {
                    Result result = m_windows[index].PushData(m_buffer, 3);
                    if (result == 0)
                        updateSendWatermark(m_windows[index]);
                }
            }
            return true;
        }
    }
    return false;
}

bool MeshProtocol::SendGreetingResponse(StationIndex stationIndex)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10) &&
            transport::g_piaStationManager->GetStation(stationIndex) != 0)
        {
            int windowIndex = static_cast<int>(stationIndex);
            if (localIndex <= windowIndex)
                --windowIndex;
            transport::ReliableSlidingWindow* window = m_windows + windowIndex;
            if (window->IsInCommunication())
            {
                m_buffer[0] = cMessageTypeGreetingResponse;
                m_buffer[1] = static_cast<unsigned char>(localIndex);
                Result result = window->PushData(m_buffer, 2);
                if (result == 0)
                    updateSendWatermark(*window);
                return result == 0;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendMigrationStart(StationIndex stationIndex, StationIndex newHostIndex)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10) &&
            transport::g_piaStationManager->GetStation(stationIndex) != 0)
        {
            int windowIndex = static_cast<int>(stationIndex);
            if (localIndex <= windowIndex)
                --windowIndex;
            transport::ReliableSlidingWindow* window = m_windows + windowIndex;
            if (window->IsInCommunication())
            {
                m_buffer[0] = cMessageTypeMigrationStart;
                m_buffer[1] = static_cast<unsigned char>(localIndex);
                m_buffer[2] = static_cast<unsigned char>(newHostIndex);
                Result result = window->PushData(m_buffer, 3);
                if (result == 0)
                    updateSendWatermark(*window);
                return result == 0;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendMigrationResponse(StationIndex stationIndex)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax)
        {
            transport::Station* station = 0;
            if (localIndex == static_cast<int>(localStation->m_field10) &&
                (station = transport::g_piaStationManager->GetStation(stationIndex)) != 0)
            {
                transport::ProtocolId protocolId(mProtocolId);
                transport::ProtocolMessageWriter* writer =
                    handlerOf(mStationPacketHandler)->AssignByStationAddress(
                        protocolId, station->m_stationAddress, 2, false, false);
                if (writer == 0)
                {
                    station->Trace(cTraceStationMigrationLevel);
                    return false;
                }

                m_buffer[0] = cMessageTypeMigrationResponse;
                m_buffer[1] = static_cast<unsigned char>(localIndex);
                writer->SetPayload(m_buffer);
                Result result = handlerOf(mStationPacketHandler)->Commit();
                if (result == 0)
                {
                    transport::ProtocolId reliableProtocolId(mProtocolId);
                    writer = handlerOf(mStationPacketHandler)->AssignByStationAddress(
                        reliableProtocolId, station->m_stationAddress, 2, true, false);
                    if (writer != 0)
                    {
                        writer->SetPayload(m_buffer);
                        handlerOf(mStationPacketHandler)->Commit();
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendRelayRouteDirections()
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        transport::RelayRouteManager* relayRouteManager = 0;
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10) &&
            (relayRouteManager = transport::g_piaTransport->m_relayRouteManager) != 0)
        {
            unsigned int directionsSize = relayRouteManager->GetRelayRouteDirectionsSize();
            unsigned char* buffer = m_buffer;
            buffer[0] = cMessageTypeRelayRouteDirections;
            buffer[1] = static_cast<unsigned char>(localIndex);
            buffer[2] = g_piaMesh->GetStationNum();
            buffer[3] = 0;
            common::serializeU32(buffer + 4, relayRouteManager->m_unknown20);
            common::serializeU32(buffer + 8, relayRouteManager->m_unknown1c);
            unsigned int writtenSize = 0;
            Result result = relayRouteManager->WriteRelayRouteDirectionsToBuffer(
                buffer + cUpdateMeshHeaderSize, m_bufferSize - cUpdateMeshHeaderSize,
                &writtenSize);
            if (result == 0)
            {
                for (unsigned short index = 0; index < m_windowNum; ++index)
                {
                    if (m_windows[index].IsInCommunication())
                    {
                        result = m_windows[index].PushData(m_buffer,
                                                           directionsSize + cUpdateMeshHeaderSize);
                        if (result == 0)
                            updateSendWatermark(m_windows[index]);
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendKickoutNotice(StationIndex stationIndex, unsigned char reason)
{
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex <= cStationIndexMax)
    {
        int windowIndex = static_cast<int>(stationIndex);
        if (localIndex <= windowIndex)
            --windowIndex;
        transport::ReliableSlidingWindow* window = m_windows + windowIndex;
        if (window->IsInCommunication())
        {
            m_buffer[0] = cMessageTypeKickoutNotice;
            m_buffer[1] = static_cast<unsigned char>(localIndex);
            m_buffer[2] = reason;
            Result result = window->PushData(m_buffer, 3);
            if (result == 0)
                updateSendWatermark(*window);
            return result == 0;
        }
    }
    return false;
}

bool MeshProtocol::SendDummyMessage(StationIndex stationIndex)
{
    unsigned int localIndex = g_piaMesh->GetLocalStationIndex();
    if (static_cast<int>(localIndex) <= cStationIndexMax)
    {
        transport::ProtocolId protocolId(mProtocolId);
        transport::ProtocolMessageWriter* writer =
            handlerOf(mStationPacketHandler)->AssignByStationIndex(protocolId, stationIndex, 2,
                                                                   false, false, true);
        if (writer != 0)
        {
            writer->WriteU8(cMessageTypeDummyMessage, 0);
            writer->WriteU8(localIndex & 0xff, 1);
            Result result = handlerOf(mStationPacketHandler)->Commit();
            return result == 0;
        }
    }
    return false;
}

bool MeshProtocol::SendDummyAck(StationIndex stationIndex)
{
    unsigned int localIndex = g_piaMesh->GetLocalStationIndex();
    if (static_cast<int>(localIndex) <= cStationIndexMax)
    {
        transport::ProtocolId protocolId(mProtocolId);
        transport::ProtocolMessageWriter* writer =
            handlerOf(mStationPacketHandler)->AssignByStationIndex(protocolId, stationIndex, 2,
                                                                   false, false, true);
        if (writer != 0)
        {
            writer->WriteU8(cMessageTypeDummyAck, 0);
            writer->WriteU8(localIndex & 0xff, 1);
            Result result = handlerOf(mStationPacketHandler)->Commit();
            return result == 0;
        }
    }
    return false;
}

void MeshProtocol::SendConnectionFailureNotice(StationIndex destination, StationIndex reporter,
                                               StationIndex failed, unsigned char reason,
                                               unsigned int version)
{
    int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
    if (localIndex <= cStationIndexMax)
    {
        transport::ProtocolId protocolId(mProtocolId);
        transport::ProtocolMessageWriter* writer =
            handlerOf(mStationPacketHandler)->AssignByStationIndex(protocolId, destination, 9,
                                                                   false, false, true);
        if (writer != 0)
        {
            unsigned char payload[9];
            payload[0] = cMessageTypeConnectionFailureNotice;
            payload[1] = static_cast<unsigned char>(localIndex);
            payload[2] = static_cast<unsigned char>(reporter);
            payload[3] = static_cast<unsigned char>(failed);
            payload[4] = reason;
            common::serializeU32(payload + 5, version);
            writer->SetPayload(payload, 0, 9);
            handlerOf(mStationPacketHandler)->Commit();
        }
    }
}

void MeshProtocol::SendUpdateMesh(bool update, unsigned int versionIncrement)
{
    bool leavingMesh = g_piaMesh->IsProcessingLeaveMesh();
    transport::StationConnectionInfo info;
    unsigned int infoSize = info.GetSerializedSize();
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    unsigned int hostIndex = g_piaMesh->GetHostStationIndex();
    unsigned int bufferSize = (infoSize + (~infoSize & cStationEntryAlignMask) + 1) *
                                  g_piaMesh->m_unknown54 +
                              cUpdateMeshHeaderSize;
    if (!update)
    {
        if (!m_updateMeshBufferValid)
            return;
    }
    else
    {
        g_piaMesh->m_unknown80 += versionIncrement;
        std::memset(m_updateMeshBuffer, 0, m_updateMeshBufferSize);
        transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
        unsigned char stationCount = 0;
        unsigned char hostSlot = 0xff;
        int offset = cUpdateMeshHeaderSize;
        unsigned char* buffer = m_updateMeshBuffer;
        for (transport::Station** it = transport::g_piaStationManager->m_stations.data;
             it != transport::g_piaStationManager->m_stations.data +
                       transport::g_piaStationManager->m_stations.size;
             ++it)
        {
            transport::Station* station = *it;
            unsigned char stationIndex = station->m_field10;
            if (stationIndex >= cStationNumMax)
            {
                station->Trace(cTraceStationLevel);
                continue;
            }
            if (!g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(stationIndex)))
            {
                station->Trace(cTraceStationLevel);
                continue;
            }
            if (stationIndex == hostIndex)
                hostSlot = stationCount;
            unsigned int size = 0;
            table->GetStationConnectionInfo(station, &info);
            info.Serialize(buffer + offset, &size, infoSize);
            buffer[offset + size] = stationIndex;
            stationCount = (stationCount + 1) & 0xff;
            offset += size + 1 + (~infoSize & cStationEntryAlignMask);
        }
        if (g_piaMesh->m_unknown54 < stationCount)
        {
            transport::g_piaStationManager->Trace(cTraceTableLevel);
            table->Trace(cTraceTableLevel);
        }
        buffer[2] = hostSlot;
        buffer[1] = stationCount;
        buffer[0] = cMessageTypeUpdateMesh;
        common::serializeU32(buffer + 4, g_piaMesh->m_unknown80);
        buffer[10] = stationCount;
        buffer[8] = 1;
        buffer[11] = 0;
        buffer[9] = 0;
        m_updateMeshBufferValid = 1;
    }

    if (!leavingMesh)
    {
        if (!m_broadcastSupported)
        {
            int sentCount = 0;
            if (update)
            {
                for (unsigned short index = 0; index < m_windowNum; ++index)
                {
                    if (m_windows[index].IsInCommunication())
                    {
                        transport::Station* station =
                            transport::g_piaStationManager->GetStationByStationIdWithoutConvert(
                                m_windows[index].m_remoteStationId);
                        if (station != 0 &&
                            g_piaMesh->IsValidStationIndex(
                                static_cast<StationIndex>(station->m_field10)))
                        {
                            int delay = (m_unknown28 / transport::g_piaTransport->m_stationNum) *
                                        sentCount;
                            Result result =
                                m_windows[index].PushData(m_updateMeshBuffer, bufferSize);
                            if (result == 0)
                            {
                                updateSendWatermark(m_windows[index]);
                                delay += m_unknown28;
                            }
                            m_nextTimes[index].m_tick =
                                transport::g_piaTransport->m_time.m_tick +
                                common::TimeSpan::GetTicksPerMSec().m_tick * delay;
                            ++sentCount;
                        }
                    }
                }
            }
            else
            {
                for (unsigned short index = 0; index < m_windowNum; ++index)
                {
                    if (m_windows[index].IsInCommunication())
                    {
                        transport::Station* station =
                            transport::g_piaStationManager->GetStationByStationIdWithoutConvert(
                                m_windows[index].m_remoteStationId);
                        if (station != 0 &&
                            g_piaMesh->IsValidStationIndex(
                                static_cast<StationIndex>(station->m_field10)))
                        {
                            int delay = (m_unknown28 / transport::g_piaTransport->m_stationNum) *
                                        sentCount;
                            m_nextTimes[index].m_tick =
                                transport::g_piaTransport->m_time.m_tick +
                                common::TimeSpan::GetTicksPerMSec().m_tick * delay;
                            ++sentCount;
                        }
                    }
                }
            }
        }
        else if (mStationPacketHandler != 0)
        {
            transport::ProtocolId protocolId(mProtocolId);
            transport::ProtocolMessageWriter* writer =
                handlerOf(mStationPacketHandler)->AssignByStationIndex(
                    protocolId, static_cast<StationIndex>(cStationIndexBroadcast), bufferSize,
                    false, false, true);
            if (writer != 0)
            {
                writer->SetPayload(m_updateMeshBuffer);
                Result result = handlerOf(mStationPacketHandler)->Commit();
                if (result == 0)
                    m_unknown30 = transport::g_piaTransport->m_time.m_tick;
            }
        }
    }
}

void MeshProtocol::ParseJoinResponse(const transport::ReceivedMessageAccessor& message)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned int ackId = transport::g_piaResendingMessageManager->ExtractAckIdFromMessage(
        data, message.m_messageSize);
    JoinMeshJob* job = static_cast<JoinMeshJob*>(GetWaitingJoinResponseJob());
    if (job == 0)
    {
        static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
            ->SendAck(ackId, message.m_stationAddress);
    }
    else if (job->m_waitingJoinResponse)
    {
        transport::Station* hostStation =
            transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
        if (hostStation != 0)
        {
            if (hostStation->m_stationAddress == message.m_stationAddress &&
                job->SetStationDataList(data, message.m_messageSize) != 0)
            {
                static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
                    ->SendAck(ackId, hostStation->m_stationAddress);
            }
        }
    }
}

void MeshProtocol::ParseLeaveRequest(const transport::ReceivedMessageAccessor& message)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    common::StationAddress address;
    address.Deserialize(data + cSystemMessageHeaderSize);
    int stationIndex = data[1];
    transport::Station* station =
        transport::g_piaStationManager->GetStation(static_cast<StationIndex>(stationIndex));
    if (station != 0)
    {
        SendLeaveResponse(station);
        if (g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(stationIndex)))
            g_piaMesh->UnfixDisconnectedId(static_cast<StationIndex>(stationIndex));
        transport::RelayRouteManager* relayRouteManager =
            transport::g_piaTransport->m_relayRouteManager;
        if (relayRouteManager != 0 && stationIndex <= cStationIndexMax)
            relayRouteManager->SearchRefugeRelayRoute(static_cast<StationIndex>(stationIndex));
        station->m_field20 = cStationReconnectState;
        station->SetStationIdAndIndex(0, static_cast<StationIndex>(cStationIndexNone));
        SendUpdateMesh(true, 1);
    }
}

void MeshProtocol::ParseLeaveResponse(const transport::ReceivedMessageAccessor& message)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    LeaveMeshJob* job = static_cast<LeaveMeshJob*>(GetWaitingLeaveResponseJob());
    if (job != 0 && job->m_disconnectStarted)
    {
        common::StationAddress address;
        address.Deserialize(data + cSystemMessageHeaderSize);
        unsigned char stationIndex = data[1];
        transport::Station* hostStation =
            transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
        if (hostStation != 0 && hostStation->m_field10 == stationIndex)
            job->m_disconnectStarted = false;
    }
}

void MeshProtocol::ParseDestroyResponse(const transport::ReceivedMessageAccessor& message)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;
    if (!g_piaMesh->m_destroyMeshJob->m_started)
        return;
    g_piaMesh->m_destroyMeshJob->ReceiveDestroyResponse(
        static_cast<StationIndex>(message.m_message[1]));
}

void MeshProtocol::ParseKickoutNotice(const transport::ReceivedMessageAccessor& message)
{
    if (!g_piaMesh->IsHost() && message.m_messageSize == 3)
    {
        const unsigned char* data = message.m_message;
        int stationIndex = data[1];
        if (stationIndex <= cStationIndexMax && stationIndex == g_piaMesh->GetHostStationIndex())
        {
            KickoutManageJob* job = g_piaMesh->m_kickoutManageJob;
            if (job->StartupClient(data[2]))
                job->Ready(false);
        }
    }
}

void MeshProtocol::ParseDummyMessage(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_sourceStationId == 0)
        return;
    if (message.m_messageSize != 2)
        return;
    SendDummyAck(static_cast<StationIndex>(message.m_message[1]));
}

void MeshProtocol::ParseRelayRouteDirections(const transport::ReceivedMessageAccessor& message)
{
    transport::RelayRouteManager* relayRouteManager = 0;
    if (transport::g_piaStationManager->m_localStation != 0 && !g_piaMesh->IsHost() &&
        (relayRouteManager = transport::g_piaTransport->m_relayRouteManager) != 0)
    {
        const unsigned char* data = message.m_message;
        unsigned int messageSize = message.m_messageSize;
        unsigned char senderIndex = data[1];
        if (senderIndex == g_piaMesh->GetHostStationIndex() &&
            static_cast<unsigned short>(data[2]) <= g_piaMesh->m_unknown54 && data[3] == 0)
        {
            unsigned int versionHigh = common::deserializeU32(data + 4);
            unsigned int versionLow = common::deserializeU32(data + 8);
            if (relayRouteManager->m_unknown20 <= versionHigh &&
                (relayRouteManager->m_unknown20 < versionHigh ||
                 relayRouteManager->m_unknown1c < versionLow))
            {
                relayRouteManager->Trace(cTraceRelayRouteLevel);
                relayRouteManager->ReadRelayRouteDirectionsFromBuffer(
                    data + cUpdateMeshHeaderSize, messageSize - cUpdateMeshHeaderSize);
                relayRouteManager->m_unknown1c = versionLow;
                relayRouteManager->m_unknown20 = versionHigh;
            }
        }
    }
}

void MeshProtocol::ParseJoinRequest(const transport::ReceivedMessageAccessor& message)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    unsigned int ackId = transport::g_piaResendingMessageManager->ExtractAckIdFromMessage(
        data, message.m_messageSize);
    ProcessJoinRequestJob* job = static_cast<ProcessJoinRequestJob*>(GetWaitingJoinRequestJob());
    if (job == 0)
        return;

    if (job->m_unknown78 != 1)
    {
        if (job->GetState() == common::Job::cStateSuspended)
        {
            common::StationAddress address(message.m_stationAddress);
            transport::Station* station = transport::g_piaStationManager->GetStation(address);
            if (station != 0 && station->m_field10 == cStationIndexNone &&
                data[1] == cStationIndexNone)
            {
                unsigned long long now = transport::g_piaTransport->m_time.m_tick;
                station->m_field60 = now >> 32;
                station->m_field64 = now;
                job->SetJoiningStationData(address);
                job->m_unknown4c = ackId;
                static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
                    ->SendAck(ackId, address);
                job->Resume(false);
            }
            return;
        }
        if (job->m_unknown78 != 1)
            return;
    }

    if (job->m_unknown4c == ackId)
    {
        static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
            ->SendAck(ackId, message.m_stationAddress);
        transport::Station* station =
            transport::g_piaStationManager->GetStation(message.m_stationAddress);
        if (station != 0)
        {
            unsigned long long now = transport::g_piaTransport->m_time.m_tick;
            station->m_field60 = now >> 32;
            station->m_field64 = now;
        }
    }
}

void MeshProtocol::ParseDestroyMesh(const transport::ReceivedMessageAccessor& message)
{
    if (transport::g_piaStationManager->m_localStation == 0)
        return;

    const unsigned char* data = message.m_message;
    common::StationAddress address;
    address.Deserialize(data + cSystemMessageHeaderSize);
    unsigned char senderIndex = data[1];
    transport::Station* hostStation =
        transport::g_piaStationManager->GetStation(g_piaMesh->GetHostStationIndex());
    if (hostStation == 0)
    {
        if (g_piaMesh->GetProcessHostMigrationJobPtr()->m_started)
        {
            ProcessDestroyMeshJob* job = g_piaMesh->m_processDestroyMeshJob;
            if (job->Startup())
                job->Ready(false);
        }
    }
    else if (hostStation->m_field10 == senderIndex)
    {
        ProcessDestroyMeshJob* job = g_piaMesh->m_processDestroyMeshJob;
        if (job->Startup())
            job->Ready(false);
    }
}

void MeshProtocol::ParseGreeting(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::g_piaStationManager->m_localStation != 0)
    {
        MeshEventListener::MeshEvent event;
        event.stationIndex = static_cast<StationIndex>(message.m_message[1]);
        ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
        if (!job->m_unknownA5)
        {
            if (!job->m_started && g_piaMesh->m_meshEventListener != 0)
            {
                event.stationId = 0;
                event.type = Session::cEventTypeUnknown15;
                g_piaMesh->m_meshEventListener->NoticeEvent(event);
            }
        }
        else
        {
            job->ReceiveGreeting(event.stationIndex);
        }
    }
}

void MeshProtocol::ParseMigrationFinish(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::g_piaStationManager->m_localStation != 0)
    {
        unsigned char succeeded = message.m_message[2];
        unsigned char senderIndex = message.m_message[1];
        if (senderIndex == g_piaMesh->GetHostStationIndex())
        {
            if (g_piaMesh->m_processHostMigrationJob->m_unknownB8)
                g_piaMesh->m_processHostMigrationJob->ReceiveFinish(succeeded != 0);
        }
    }
}

void MeshProtocol::ParseGreetingResponse(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() && transport::g_piaStationManager->m_localStation != 0)
    {
        if (g_piaMesh->m_processHostMigrationJob->m_greetingSent)
        {
            g_piaMesh->m_processHostMigrationJob->ReceiveGreetingResponse(
                static_cast<StationIndex>(message.m_message[1]));
        }
    }
}

void MeshProtocol::ParseMigrationStart(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() &&
        transport::g_piaStationManager->m_localStation != 0 &&
        !g_piaMesh->m_leaveMeshJob->IsRunning())
    {
        const unsigned char* data = message.m_message;
        unsigned char senderIndex = data[1];
        if (senderIndex == g_piaMesh->GetHostStationIndex())
        {
            int newHostIndex = data[2];
            if (newHostIndex <= cStationIndexMax &&
                newHostIndex != g_piaMesh->GetHostStationIndex())
            {
                ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
                if (job->Startup(true, static_cast<StationIndex>(newHostIndex)))
                    job->Ready(false);
            }
        }
    }
}

void MeshProtocol::ParseMultiMigrationStart(const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() &&
        transport::g_piaStationManager->m_localStation != 0 && !g_piaMesh->IsHost() &&
        !g_piaMesh->m_leaveMeshJob->IsRunning())
    {
        const unsigned char* data = message.m_message;
        unsigned char senderIndex = data[1];
        if (message.m_messageSize == g_piaMesh->m_unknown54 + cUpdateMeshHeaderSize &&
            senderIndex == g_piaMesh->GetHostStationIndex())
        {
            unsigned int versionHigh = common::deserializeU32(data + 4);
            unsigned int versionLow = common::deserializeU32(data + 8);
            ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
            if (job->StartupMulti(true, data + cUpdateMeshHeaderSize, versionHigh, versionLow))
                job->Ready(false);
        }
    }
}

void MeshProtocol::ParseMultiMigrationRankDecision(
    const transport::ReceivedMessageAccessor& message)
{
    if (g_piaMesh->IsHostMigrationEnabled() &&
        transport::g_piaStationManager->m_localStation != 0 &&
        message.m_messageSize == cRankDecisionReceivedSize)
    {
        const unsigned char* data = message.m_message;
        unsigned int ackId = transport::g_piaResendingMessageManager->ExtractAckIdFromMessage(
            data, cRankDecisionReceivedSize);
        unsigned char rawHigher = data[4];
        StationIndex senderIndex = static_cast<StationIndex>(data[1]);
        if (rawHigher == 0 || rawHigher == 1)
        {
            bool higher = rawHigher != 0;
            unsigned int meshValue = common::deserializeU32(data + 5);
            unsigned int relayValue = common::deserializeU32(data + 9);
            ProcessHostMigrationJob* job = g_piaMesh->m_processHostMigrationJob;
            bool firstIsHigher;
            unsigned int candidateMeshValue;
            unsigned int candidateRelayValue;
            Result result = job->CheckHostCandidateRanking(
                g_piaMesh->GetLocalStationIndex(), senderIndex, &firstIsHigher,
                &candidateMeshValue, &candidateRelayValue);
            if (result == 0)
            {
                if (firstIsHigher != higher &&
                    (meshValue < candidateMeshValue ||
                     (meshValue <= candidateMeshValue && relayValue < candidateRelayValue)))
                {
                    higher = firstIsHigher;
                }
                static_cast<transport::StationProtocol*>(g_piaMesh->GetStationProtocol())
                    ->SendAck(ackId, senderIndex);
                job->SetRankDecision(senderIndex, higher);
            }
        }
    }
}

bool MeshProtocol::SendMultiMigrationStart(bool* sent)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10))
        {
            unsigned char message[cMultiMigrationMessageSize] = {0};
            unsigned short stationNum = g_piaMesh->m_unknown54;
            unsigned int meshValue;
            unsigned int relayValue;
            Result result = g_piaMesh->m_processHostMigrationJob->MakeHostCandidateRanking(
                static_cast<StationIndex>(localIndex), message + cUpdateMeshHeaderSize,
                &meshValue, &relayValue, true);
            if (result == 0)
            {
                message[1] = static_cast<unsigned char>(localIndex);
                message[0] = cMessageTypeMultiMigrationStart;
                message[2] = g_piaMesh->GetStationNum();
                common::serializeU32(message + 4, meshValue);
                common::serializeU32(message + 8, relayValue);
                for (int index = 0; index < cStationNumMax; ++index)
                {
                    sent[index] = false;
                    if (index != localIndex &&
                        g_piaMesh->IsValidStationIndex(static_cast<StationIndex>(index)))
                    {
                        transport::ReliableSlidingWindow* window;
                        if (index < localIndex)
                            window = m_windows + index;
                        else
                            window = m_windows + (index - 1);
                        if (window->IsInCommunication())
                        {
                            result = window->PushData(message, stationNum + cUpdateMeshHeaderSize);
                            if (result == 0)
                            {
                                updateSendWatermark(*window);
                                sent[index] = true;
                            }
                        }
                    }
                }
                return true;
            }
        }
    }
    return false;
}

bool MeshProtocol::SendMultiMigrationRankDecision(StationIndex stationIndex, long long timeout,
                                                  unsigned int* ackId)
{
    transport::Station* localStation = transport::g_piaStationManager->m_localStation;
    if (localStation != 0)
    {
        int localIndex = static_cast<int>(g_piaMesh->GetLocalStationIndex());
        if (localIndex <= cStationIndexMax &&
            localIndex == static_cast<int>(localStation->m_field10) &&
            transport::g_piaStationManager->GetStation(stationIndex) != 0)
        {
            bool firstIsHigher;
            unsigned int meshValue;
            unsigned int relayValue;
            Result result = g_piaMesh->m_processHostMigrationJob->CheckHostCandidateRanking(
                static_cast<StationIndex>(localIndex), stationIndex, &firstIsHigher, &meshValue,
                &relayValue);
            if (result == 0)
            {
                unsigned char message[cRankDecisionMessageBufferSize] = {0};
                message[0] = cMessageTypeMultiMigrationRankDecision;
                message[1] = static_cast<unsigned char>(localIndex);
                message[2] = g_piaMesh->GetHostStationIndex();
                message[3] = g_piaMesh->GetStationNum();
                message[4] = !firstIsHigher;
                common::serializeU32(message + 5, meshValue);
                common::serializeU32(message + 9, relayValue);
                transport::ProtocolId protocolId(mProtocolId);
                result = transport::g_piaResendingMessageManager->SetSendMessageByStationIndex(
                    ackId, message, cRankDecisionMessageSize, stationIndex, protocolId, true,
                    timeout);
                return result == 0;
            }
        }
    }
    return false;
}

unsigned char MeshProtocol::MakeJoinResponseData(StationIndex stationIndex, unsigned char** buffers,
                                                 unsigned int* sizes)
{
    transport::StationConnectionInfo info;
    unsigned int infoSize = info.GetSerializedSize();
    unsigned int hostIndex = g_piaMesh->GetHostStationIndex();
    std::memset(m_buffer, 0, m_bufferSize);
    unsigned int hostSlot = 0xff;
    unsigned int count = 0;
    unsigned int stationSlot = 0xff;
    unsigned char* buffer = m_buffer;
    if (g_piaSession != 0)
    {
        unsigned int current = g_piaSession->m_currentSessionIndex;
        if (g_piaSession->m_matchmakeSession[current] != 0)
            buffer[8] = g_piaSession->m_fieldc2[current];
        unsigned int other = current == 0;
        if (g_piaSession->m_matchmakeSession[other] != 0)
            buffer[9] = g_piaSession->m_fieldc2[other];
    }
    buffer[10] = g_piaMesh->m_unknown54;
    common::serializeU32(buffer + 12, g_piaMesh->m_unknown80);
    transport::StationConnectionInfoTable* table = transport::g_stationConnectionInfoTable;
    unsigned int offset = cJoinResponseHeaderSize;
    for (transport::Station** it = transport::g_piaStationManager->m_stations.data;
         it != transport::g_piaStationManager->m_stations.data +
                   transport::g_piaStationManager->m_stations.size;
         ++it)
    {
        transport::Station* station = *it;
        unsigned char index = station->m_field10;
        if (index >= cStationNumMax)
        {
            station->Trace(cTraceStationUpdateLevel);
            continue;
        }
        if (index == hostIndex)
            hostSlot = count;
        if (index == stationIndex)
            stationSlot = count;
        unsigned int size = 0;
        table->GetStationConnectionInfo(station, &info);
        info.Serialize(buffer + offset, &size, infoSize);
        buffer[offset + size] = index;
        count = (count + 1) & 0xff;
        offset += size + 1 + (~infoSize & cStationEntryAlignMask);
    }

    unsigned int partitionNum = GetSystemMessagePartitionNum(cMessageTypeJoinResponse, count);
    char stationCount = count;
    if (partitionNum == 1)
    {
        if (handlerOf(mStationPacketHandler)->GetPayloadSizeLimit() < offset)
            return 0;
        unsigned char* output = buffers[0];
        output[7] = 0;
        output[1] = stationCount;
        output[5] = 0;
        output[3] = stationSlot;
        output[6] = stationCount;
        output[2] = hostSlot;
        output[0] = cMessageTypeJoinResponse;
        output[4] = 1;
        std::memcpy(output + cJoinResponsePartHeaderSize, buffer + cJoinResponsePartHeaderSize,
                    offset - cJoinResponsePartHeaderSize);
        sizes[0] = offset;
        return 1;
    }

    if (partitionNum - 2 > 1)
        return 0;

    int consumed = 0;
    char partIndex = 0;
    unsigned int remaining = count;
    unsigned int perPart = (remaining / partitionNum + (remaining % partitionNum != 0)) & 0xff;
    unsigned char* header = buffer;
    for (unsigned int part = 0; part < partitionNum; ++part)
    {
        header[0] = cMessageTypeJoinResponse;
        header[1] = stationCount;
        int headerSize = cJoinResponsePartHeaderSize;
        header[7] = stationCount - static_cast<char>(remaining);
        header[2] = hostSlot;
        header[3] = stationSlot;
        header[5] = partIndex;
        header[6] = static_cast<char>(perPart);
        header[4] = static_cast<char>(partitionNum);
        if (partIndex == 0)
            headerSize = cJoinResponseHeaderSize;
        int partSize = (infoSize + (~infoSize & cStationEntryAlignMask) + 1) * perPart + headerSize;
        std::memcpy(buffers[part], header, partSize);
        remaining = (remaining - perPart) & 0xff;
        consumed += partSize - cJoinResponsePartHeaderSize;
        header = buffer + consumed;
        sizes[part] = partSize;
        if (remaining < perPart)
            perPart = remaining;
        ++partIndex;
    }
    return partitionNum;
}

void MeshProtocol::ParseHelper(const transport::ReceivedMessageAccessor& message)
{
    if (message.m_messageSize == 0)
        return;

    switch (message.m_message[0])
    {
    case cMessageTypeJoinRequest:
        ParseJoinRequest(message);
        break;
    case cMessageTypeJoinResponse:
        ParseJoinResponse(message);
        break;
    case cMessageTypeLeaveRequest:
        ParseLeaveRequest(message);
        break;
    case cMessageTypeLeaveResponse:
        ParseLeaveResponse(message);
        break;
    case cMessageTypeDestroyMesh:
        ParseDestroyMesh(message);
        break;
    case cMessageTypeDestroyResponse:
        ParseDestroyResponse(message);
        break;
    case cMessageTypeUpdateMesh:
        ParseUpdateMesh(message);
        break;
    case cMessageTypeKickoutNotice:
        ParseKickoutNotice(message);
        break;
    case cMessageTypeDummyMessage:
        ParseDummyMessage(message);
        break;
    case cMessageTypeDummyAck:
        ParseDummyAck(message);
        break;
    case cMessageTypeConnectionFailureNotice:
        ParseConnectionFailureNotice(message);
        break;
    case cMessageTypeGreeting:
        ParseGreeting(message);
        break;
    case cMessageTypeMigrationFinish:
        ParseMigrationFinish(message);
        break;
    case cMessageTypeGreetingResponse:
        ParseGreetingResponse(message);
        break;
    case cMessageTypeMigrationStart:
        ParseMigrationStart(message);
        break;
    case cMessageTypeMigrationResponse:
        ParseMigrationResponse(message);
        break;
    case cMessageTypeMultiMigrationStart:
        ParseMultiMigrationStart(message);
        break;
    case cMessageTypeMultiMigrationRankDecision:
        ParseMultiMigrationRankDecision(message);
        break;
    case cMessageTypeConnectionReport:
        ParseConnectionReport(message);
        break;
    case cMessageTypeRelayRouteDirections:
        ParseRelayRouteDirections(message);
        break;
    default:
    {
        const common::StationAddress* address = &message.m_stationAddress;
        address->Trace(cTraceMessageLevel);
        break;
    }
    }
}

}}}
