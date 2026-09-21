#include "nn/pia/transport/BandwidthCheckProtocol.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

namespace
{
const unsigned int cBulkBufferSize = 0x584;

long long elapsedMilliseconds(const common::Time& start)
{
    common::Time now;
    now.SetNow();
    const long long elapsedTicks = static_cast<long long>(now.m_tick) -
                                   static_cast<long long>(start.m_tick);
    const long long ticksPerMillisecond = common::TimeSpan::GetTicksPerMSec().m_tick;
    return elapsedTicks / ticksPerMillisecond;
}

unsigned int stationBit(StationIndex index)
{
    if (index < 0 || index >= 32)
        return 0;
    return 1u << static_cast<unsigned int>(index);
}
}

Result BandwidthCheckProtocol::Header::Serialize(unsigned char* buffer,
                                                 unsigned int* size) const
{
    if (buffer == 0 || size == 0)
        return 0x10c07;
    const unsigned int values[4] = { m_type, m_sequence, m_offset, m_flags };
    for (unsigned int field = 0; field < 4; ++field)
    {
        const unsigned int value = values[field];
        buffer[field * 4 + 0] = static_cast<unsigned char>(value >> 24);
        buffer[field * 4 + 1] = static_cast<unsigned char>(value >> 16);
        buffer[field * 4 + 2] = static_cast<unsigned char>(value >> 8);
        buffer[field * 4 + 3] = static_cast<unsigned char>(value);
    }
    *size = GetSerializedSize();
    return 0;
}

Result BandwidthCheckProtocol::Header::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    unsigned int* values[4] = { &m_type, &m_sequence, &m_offset, &m_flags };
    for (unsigned int field = 0; field < 4; ++field)
    {
        const unsigned int offset = field * 4;
        *values[field] = (static_cast<unsigned int>(buffer[offset + 0]) << 24) |
                         (static_cast<unsigned int>(buffer[offset + 1]) << 16) |
                         (static_cast<unsigned int>(buffer[offset + 2]) << 8) |
                         static_cast<unsigned int>(buffer[offset + 3]);
    }
    return 0;
}

unsigned int BandwidthCheckProtocol::Header::GetSerializedSize() const
{
    return 16;
}

BandwidthCheckProtocol::Header::~Header()
{
}

BandwidthCheckProtocol::BandwidthCheckProtocol()
    : Protocol(),
      m_initialized(false), m_stationNum(0), m_activated(false), m_bulkBuffer(0),
      m_activeStationBitmap(0), m_bandwidthCheckedBitmap(0),
      m_measurementTime(static_cast<unsigned int>(-1)),
      m_bulkPacketSize(0), m_completionTimeout(0), m_measurementCount(0),
      m_senderState(SenderState_None), m_senderStartTime(), m_ackReceived(false),
      m_bulkStartTime(), m_uplinkBitRate(0), m_senderStation(0xfd),
      m_bulkPacketSequence(0), m_cancelRequested(false), m_receiverState(ReceiverState_None),
      m_receiverStartTime(), m_bulkArrivalTime(), m_responseSentTime(), m_ackSentTime(),
      m_receivedBytes(0),
      m_receiverStation(0xfd), m_stationAddress(), m_receivedBulkPacketCount(0),
      m_firstBulkPacketSequence(-1), m_lastBulkPacketSequence(-1), m_receiverActive(false),
      m_isServer(false), m_measurementResult(0)
{
}

BandwidthCheckProtocol::~BandwidthCheckProtocol()
{
}

void BandwidthCheckProtocol::initSender()
{
    m_senderStartTime.m_tick = 0;
    m_uplinkBitRate = -1;
    m_senderStation = 0xfd;
    m_bulkPacketSequence = 0;
    m_senderState = SenderState_None;
    m_ackReceived = false;
    m_bulkStartTime.m_tick = 0;
    m_cancelRequested = false;
    m_measurementResult = -1;
}

void BandwidthCheckProtocol::initReceiver()
{
    m_receiverState = ReceiverState_None;
    m_receiverStation = 0xfd;
    m_receivedBytes = 0;
    m_bulkArrivalTime.m_tick = 0;
    m_receiverStartTime.m_tick = 0;
    m_responseSentTime.m_tick = 0;
    m_ackSentTime.m_tick = 0;
    m_receivedBulkPacketCount = 0;
    m_firstBulkPacketSequence = -1;
    m_lastBulkPacketSequence = -1;
    m_receiverActive = false;
    m_stationAddress.Clear();
}

Result BandwidthCheckProtocol::Initialize()
{
    if (m_initialized)
        return 0x10c02;
    pead::Heap* heap = common::HeapManager::GetHeap();
    unsigned char* buffer = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(cBulkBufferSize, heap));
    if (buffer == 0)
        return 0x10c01;
    for (unsigned int index = 0; index < cBulkBufferSize; ++index)
        buffer[index] = 0;
    m_bulkBuffer = buffer;
    m_stationNum = g_piaTransport != 0 ? g_piaTransport->m_stationNum :
                   (g_piaStationManager != 0 ? g_piaStationManager->m_stationNum : 0);
    m_activeStationBitmap = 0;
    m_bandwidthCheckedBitmap = 0;
    m_measurementTime = static_cast<unsigned int>(-1);
    m_bulkPacketSize = 0;
    m_completionTimeout = 0;
    m_measurementCount = 0;
    initSender();
    initReceiver();
    m_initialized = true;
    m_activated = false;
    return 0;
}

void BandwidthCheckProtocol::Finalize()
{
    if (!m_initialized)
        return;
    m_initialized = false;
    m_activated = false;
    if (m_bulkBuffer != 0)
    {
        pead::getArraySize(m_bulkBuffer);
        pead::freeToSeadHeap(m_bulkBuffer);
        m_bulkBuffer = 0;
    }
}

Result BandwidthCheckProtocol::Startup()
{
    m_activeStationBitmap = 0;
    m_bandwidthCheckedBitmap = 0;
    return 0;
}

void BandwidthCheckProtocol::Cleanup()
{
    m_activeStationBitmap = 0;
    m_bandwidthCheckedBitmap = 0;
}

int BandwidthCheckProtocol::GetMaxIpPacketSize() const
{
    return common::g_payloadSizeManager == 0 ? 0 :
        static_cast<int>(common::g_payloadSizeManager->m_mtuSize + 0x1c);
}

int BandwidthCheckProtocol::GetMinIpPacketSize() const
{
    return common::g_payloadSizeManager == 0 ? 0 :
        static_cast<int>(common::g_payloadSizeManager->m_minMtuSize + 0x68);
}

unsigned int BandwidthCheckProtocol::IsSetupParamValid(int measurementTime,
                                                       unsigned int ipPacketSize,
                                                       int measurementCount) const
{
    return static_cast<unsigned int>(measurementTime - 1) < 1000000u &&
           (ipPacketSize & 3u) == 0 &&
           ipPacketSize <= static_cast<unsigned int>(GetMaxIpPacketSize()) &&
           static_cast<unsigned int>(GetMinIpPacketSize()) <= ipPacketSize &&
           measurementCount > 0;
}

Result BandwidthCheckProtocol::Activate(int measurementTime, unsigned int ipPacketSize,
                                        bool isServer, int measurementCount)
{
    if (!IsSetupParamValid(measurementTime, ipPacketSize, measurementCount))
        return 0x10c07;
    if (!m_initialized)
        return 0x10c0c;
    m_measurementTime = static_cast<unsigned int>(measurementTime);
    m_bulkPacketSize = ipPacketSize;
    m_activated = true;
    m_measurementCount = static_cast<unsigned int>(measurementCount);
    m_completionTimeout = static_cast<unsigned int>(measurementCount + 0xfa);
    m_isServer = isServer;
    return 0;
}

void BandwidthCheckProtocol::Inactivate()
{
    m_completionTimeout = 0;
    m_measurementTime = static_cast<unsigned int>(-1);
    m_activated = false;
    m_isServer = false;
    m_measurementCount = 0;
    initSender();
    initReceiver();
}

unsigned int BandwidthCheckProtocol::IsReady() const
{
    return m_initialized && m_activated &&
           (m_senderState == SenderState_None ||
            m_senderState == SenderState_Completed ||
            m_senderState == SenderState_Canceled);
}

Result BandwidthCheckProtocol::StartSend(StationIndex stationIndex)
{
    if (stationIndex < 0 || stationIndex >= 32)
        return 0x10c07;
    if (!m_initialized)
        return 0x10c0c;
    if (!IsReady())
        return 0x10c08;
    initSender();
    m_senderStation = stationIndex;
    return 0;
}

void BandwidthCheckProtocol::CancelSend()
{
    if (m_senderState >= SenderState_WaitingForAck &&
        m_senderState <= SenderState_Measuring)
        m_cancelRequested = true;
}

unsigned int BandwidthCheckProtocol::IsCompleted() const
{
    return m_initialized &&
           (m_senderState == SenderState_Completed ||
            m_senderState == SenderState_Canceled);
}

Result BandwidthCheckProtocol::GetUplinkBitRate(int* result) const
{
    if (result == 0)
        return 0x10c07;
    if (!m_initialized)
        return 0x10c0c;
    if (!IsCompleted())
        return 0x10c08;
    *result = m_senderState == SenderState_Completed ? m_uplinkBitRate : -1;
    return 0;
}

void BandwidthCheckProtocol::SetBandwidthChecked(StationIndex stationIndex)
{
    const unsigned int bit = stationBit(stationIndex);
    if (bit != 0)
        m_bandwidthCheckedBitmap |= bit;
}

unsigned int BandwidthCheckProtocol::getBulkPacketSize() const
{
    return m_bulkPacketSize;
}

bool BandwidthCheckProtocol::IsBandwidthChecked(StationIndex stationIndex) const
{
    const unsigned int bit = stationBit(stationIndex);
    return bit != 0 && (m_bandwidthCheckedBitmap & bit) != 0;
}

bool BandwidthCheckProtocol::isStartSendCalled() const
{
    return m_senderStation != 0xfd;
}

void BandwidthCheckProtocol::processCancel()
{
    m_senderState = SenderState_Canceled;
    m_cancelRequested = false;
}

bool BandwidthCheckProtocol::isBulkDataArrived() const
{
    return m_firstBulkPacketSequence != -1;
}

bool BandwidthCheckProtocol::isExist(StationIndex stationIndex) const
{
    return g_piaStationManager != 0 &&
           g_piaStationManager->GetStation(stationIndex) != 0;
}

Result BandwidthCheckProtocol::send(StationIndex stationIndex,
                                    const unsigned char* payload,
                                    unsigned int payloadSize)
{
    if (!isExist(stationIndex))
        return 0x2c0a;
    StationPacketHandler* handler = reinterpret_cast<StationPacketHandler*>(
        static_cast<uintptr_t>(GetStationPacketHandler()));
    if (handler == 0)
        return 0;
    ProtocolMessageWriter* writer = handler->AssignByStationIndex(
        mProtocolId, stationIndex, payloadSize, false, false, true);
    if (writer == 0)
        return 0;
    writer->SetPayload(payload);
    return handler->Commit();
}

Result BandwidthCheckProtocol::sendHeaderOnlyPacket(StationIndex stationIndex,
                                                    unsigned int sequence,
                                                    int packetKind, int packetValue,
                                                    int packetExtra)
{
    Header header;
    header.m_type = static_cast<unsigned int>(packetKind);
    header.m_sequence = static_cast<unsigned int>(packetValue);
    header.m_offset = static_cast<unsigned int>(packetExtra);
    header.m_flags = sequence;
    unsigned char bytes[16];
    unsigned int size = 0;
    header.Serialize(bytes, &size);
    return send(stationIndex, bytes, size);
}

Result BandwidthCheckProtocol::sendSynPacket()
{
    return sendHeaderOnlyPacket(m_senderStation, 0xffffffffu, 3, -1, -1);
}

int BandwidthCheckProtocol::getPayloadSize(unsigned int ipPacketSize) const
{
    return static_cast<int>(ipPacketSize) -
           static_cast<int>(common::g_payloadSizeManager->m_minMtuSize + 0x58);
}

Result BandwidthCheckProtocol::sendBulkPacket()
{
    if (m_bulkBuffer == 0 || common::g_payloadSizeManager == 0)
        return 0x10c01;
    Header header;
    header.m_type = 1;
    header.m_sequence = 0xffffffffu;
    header.m_offset = m_bulkPacketSequence;
    header.m_flags = 0xffffffffu;
    unsigned int serializedSize = 0;
    header.Serialize(m_bulkBuffer, &serializedSize);
    PacketHandler* handler = reinterpret_cast<PacketHandler*>(
        static_cast<uintptr_t>(GetStationPacketHandler()));
    if (handler == 0)
        return 0x10c08;
    int payloadSize = getPayloadSize(m_bulkPacketSize);
    const int payloadLimit = handler->GetPayloadSizeLimit();
    if (payloadSize > payloadLimit)
        payloadSize = payloadLimit;
    if (payloadSize < static_cast<int>(serializedSize) ||
        payloadSize > static_cast<int>(cBulkBufferSize))
        return 0x10c07;
    const Result result = send(m_senderStation, m_bulkBuffer,
                               static_cast<unsigned int>(payloadSize));
    if (result == 0)
        ++m_bulkPacketSequence;
    return result;
}

Result BandwidthCheckProtocol::sendAckPacket()
{
    return sendHeaderOnlyPacket(m_receiverStation, 0xffffffffu, 4, -1, -1);
}

Result BandwidthCheckProtocol::sendResponse()
{
    const float measuredBitRateFloat =
        (static_cast<float>(static_cast<int>(m_receivedBytes)) * 8.0f * 1000.0f) /
        static_cast<float>(static_cast<int>(m_measurementCount));
    const int measuredBitRate = static_cast<int>(measuredBitRateFloat);
    return sendHeaderOnlyPacket(m_receiverStation, m_receivedBulkPacketCount, 2,
                                measuredBitRate, -1);
}

void BandwidthCheckProtocol::processSend()
{
    if (m_senderState == SenderState_WaitingForAck &&
        elapsedMilliseconds(m_senderStartTime) > 150)
    {
        m_senderStartTime.SetNow();
        if (sendSynPacket() == 0x2c0a)
            m_cancelRequested = true;
    }
    else if (m_senderState == SenderState_Measuring)
    {
        const float target = static_cast<float>(m_measurementTime) * 1.30f *
                             static_cast<float>(m_bulkPacketSize) / 1000.0f;
        for (unsigned int packet = 0; packet < 4; ++packet)
        {
            const float sent = static_cast<float>(m_bulkPacketSequence) *
                               static_cast<float>(m_bulkPacketSize) * 8.0f;
            if (!(sent < target))
                break;
            if (sendBulkPacket() == 0x2c0a)
                m_cancelRequested = true;
        }
    }
    if (m_receiverState == ReceiverState_WaitingForAck &&
        elapsedMilliseconds(m_ackSentTime) > 100)
    {
        sendAckPacket();
        m_ackSentTime.SetNow();
    }
    else if (m_receiverState == ReceiverState_WaitingForResponse &&
             elapsedMilliseconds(m_responseSentTime) > 100)
    {
        sendResponse();
        m_responseSentTime.SetNow();
    }
}

int BandwidthCheckProtocol::getIpPacketSize(unsigned int payloadSize) const
{
    return static_cast<int>(payloadSize) +
           static_cast<int>(common::g_payloadSizeManager->m_minMtuSize + 0x58);
}

int BandwidthCheckProtocol::getSendPacketNumInMeasuringTime() const
{
    const float denominator = static_cast<float>(m_bulkPacketSize);
    if (denominator == 0.0f)
        return 1;
    return static_cast<int>(1.2999999523162842f *
        (((static_cast<float>(m_measurementTime) - 176.0f) * 0.125f) /
         denominator) * ((static_cast<float>(m_measurementCount) - 176.0f) /
                         1000.0f)) + 1;
}

void BandwidthCheckProtocol::senderStateTransition()
{
    switch (m_senderState)
    {
    case SenderState_None:
        if (isStartSendCalled())
            m_senderState = SenderState_WaitingForAck;
        break;
    case SenderState_WaitingForAck:
        if (m_cancelRequested)
            processCancel();
        else if (m_ackReceived)
        {
            m_senderState = SenderState_Measuring;
            m_bulkStartTime.SetNow();
        }
        break;
    case SenderState_Measuring:
        if (m_cancelRequested)
            processCancel();
        else if (m_uplinkBitRate != -1)
            m_senderState = SenderState_Completed;
        else if (elapsedMilliseconds(m_bulkStartTime) >
                 static_cast<long long>(m_completionTimeout))
            m_senderState = SenderState_WaitingForResponse;
        break;
    case SenderState_WaitingForResponse:
        if (m_cancelRequested)
            processCancel();
        else if (m_uplinkBitRate != -1)
            m_senderState = SenderState_Completed;
        else if (elapsedMilliseconds(m_bulkStartTime) >
                 static_cast<long long>(m_completionTimeout) + 2000)
            m_senderState = SenderState_Canceled;
        break;
    default:
        break;
    }
}

void BandwidthCheckProtocol::receiverStateTransition()
{
    switch (m_receiverState)
    {
    case ReceiverState_None:
        if (m_receiverActive)
        {
            m_receiverState = ReceiverState_WaitingForBulk;
            m_receiverStartTime.SetNow();
        }
        break;
    case ReceiverState_WaitingForBulk:
        if (isBulkDataArrived())
        {
            m_receiverState = ReceiverState_Measuring;
            m_bulkArrivalTime.SetNow();
        }
        else if (elapsedMilliseconds(m_receiverStartTime) > 2000)
            initReceiver();
        break;
    case ReceiverState_Measuring:
        if (elapsedMilliseconds(m_bulkArrivalTime) >
            static_cast<long long>(m_measurementCount))
            m_receiverState = ReceiverState_WaitingForAck;
        break;
    case ReceiverState_WaitingForAck:
        if (elapsedMilliseconds(m_bulkArrivalTime) >
            static_cast<long long>(m_measurementCount) + 1000)
        {
            if (m_uplinkBitRate == -1 && !isStartSendCalled() && !m_isServer)
                StartSend(m_receiverStation);
            SetBandwidthChecked(m_receiverStation);
            initReceiver();
        }
        break;
    }
}

void BandwidthCheckProtocol::processReceive()
{
    PacketHandler* handler = reinterpret_cast<PacketHandler*>(
        static_cast<uintptr_t>(GetStationPacketHandler()));
    PacketHandler::Iterator& iterator = handler->GetIterator(mProtocolId);
    while (!handler->IsEndIteration())
    {
        ProtocolMessageReader& reader = *iterator.GetMessageReader();
        {
            const StationIndex stationIndex =
                g_piaTransport->ConvertToStationIndexUnsafe(reader.m_header.m_source);
            if ((m_activeStationBitmap & stationBit(stationIndex)) != 0)
            {
                Header header;
                header.Deserialize(reader.m_message + ProtocolMessageReader::cMessageHeaderSize);
                if (header.m_type == 3)
                {
                    if (m_receiverState == ReceiverState_None &&
                        !IsBandwidthChecked(stationIndex))
                    {
                        m_receiverStation = stationIndex;
                        m_stationAddress = reader.m_stationAddress;
                        m_receiverActive = true;
                        Trace(0x8000);
                    }
                }
                else if (header.m_type == 1)
                {
                    if (m_receiverState == ReceiverState_WaitingForBulk &&
                        stationIndex == m_receiverStation &&
                        m_stationAddress == reader.m_stationAddress)
                    {
                        m_firstBulkPacketSequence = static_cast<int>(header.m_offset);
                        m_lastBulkPacketSequence = static_cast<int>(header.m_offset);
                    }
                    else if (m_receiverState == ReceiverState_Measuring &&
                             stationIndex == m_receiverStation &&
                             m_stationAddress == reader.m_stationAddress)
                    {
                        m_receivedBytes += static_cast<unsigned int>(
                            getIpPacketSize(reader.m_messageSize));
                        ++m_receivedBulkPacketCount;
                    }
                    else
                    {
                        Station* station = g_piaStationManager->GetStation(reader.m_stationAddress);
                        station->Trace(0x8000);
                    }
                }
                else if (header.m_type == 2)
                {
                    if (m_senderState == SenderState_Measuring ||
                        m_senderState == SenderState_WaitingForResponse)
                    {
                        m_uplinkBitRate = static_cast<int>(header.m_sequence);
                        const int packetCount = getSendPacketNumInMeasuringTime();
                        const int value = packetCount > 0
                            ? 10000 - static_cast<int>(header.m_flags) * 10000 / packetCount
                            : 0;
                        m_measurementResult = value > 0 ? value : 0;
                    }
                }
                else if (header.m_type == 4)
                {
                    if (m_senderState == SenderState_WaitingForAck &&
                        m_senderStation == stationIndex)
                    {
                        m_ackReceived = true;
                        Trace(0x8000);
                    }
                    else
                    {
                        Station* station = g_piaStationManager->GetStation(reader.m_stationAddress);
                        station->Trace(0x8000);
                    }
                }
            }
        }
        senderStateTransition();
        receiverStateTransition();
        handler->NextIteration();
    }
    senderStateTransition();
    receiverStateTransition();
}

Result BandwidthCheckProtocol::Dispatch()
{
    if (!m_initialized)
        return 0x10c0c;
    if (!m_activated)
        return 0;
    processSend();
    processReceive();
    return 0;
}

Result BandwidthCheckProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    if (g_piaStationManager == 0)
        return 0x10c07;
    const Station* station = g_piaStationManager->GetStation(event.m_stationAddress);
    if (station == 0)
        return 0x10c07;
    const StationIndex stationIndex = static_cast<StationIndex>(station->m_field10);
    if (stationIndex < 0 || static_cast<unsigned int>(stationIndex) >= m_stationNum)
        return 0x10c07;
    const unsigned int bit = stationBit(stationIndex);
    if (event.m_type == 0)
        m_activeStationBitmap |= bit;
    else if (event.m_type == 1)
    {
        m_activeStationBitmap &= ~bit;
        m_bandwidthCheckedBitmap &= ~bit;
        if (stationIndex == m_receiverStation)
            initReceiver();
        if (stationIndex == m_senderStation && !IsCompleted())
            m_senderState = SenderState_Canceled;
    }
    else
        return 0x10c07;
    return 0;
}

void BandwidthCheckProtocol::Trace(unsigned long long) const
{
}

unsigned short BandwidthCheckProtocol::GetProtocolType() const
{
    return 0x54;
}

}}}
