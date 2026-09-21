#include "nn/pia/transport/RttProtocol.hpp"

#include "nn/pia/HeapManager.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/NetworkFactory.hpp"
#include "nn/pia/transport/PacketHandler.hpp"
#include "nn/pia/transport/ProtocolEvent.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/transport/ProtocolMessageWriter.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationPacketHandler.hpp"
#include "platform/wiiu/pead.hpp"

#include <new>

namespace nn { namespace pia { namespace transport {

namespace {

inline unsigned int readU32(const unsigned char* buffer)
{
    return (static_cast<unsigned int>(buffer[0]) << 24) |
           (static_cast<unsigned int>(buffer[1]) << 16) |
           (static_cast<unsigned int>(buffer[2]) << 8) |
           static_cast<unsigned int>(buffer[3]);
}

inline void writeU32(unsigned char* buffer, unsigned int value)
{
    buffer[0] = static_cast<unsigned char>(value >> 24);
    buffer[1] = static_cast<unsigned char>(value >> 16);
    buffer[2] = static_cast<unsigned char>(value >> 8);
    buffer[3] = static_cast<unsigned char>(value);
}

}

RttProtocol::Data::~Data()
{
}

Result RttProtocol::Data::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    m_type = readU32(buffer);
    m_sequence = readU32(buffer + 4);
    m_timestamp = (static_cast<unsigned long long>(readU32(buffer + 8)) << 32) |
        readU32(buffer + 12);
    return 0;
}

Result RttProtocol::Data::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0 || size == 0)
        return 0x10c07;
    writeU32(buffer, m_type);
    writeU32(buffer + 4, m_sequence);
    writeU32(buffer + 8, static_cast<unsigned int>(m_timestamp >> 32));
    writeU32(buffer + 12, static_cast<unsigned int>(m_timestamp));
    *size = 16;
    return 0;
}

unsigned int RttProtocol::Data::GetSerializedSize() const
{
    return 16;
}

RttProtocol::RttProtocol()
    : Protocol(), m_stationIndex(0xfd), m_stationIdHigh(0), m_stationIdLow(0),
      m_rttCalculators(0), m_stationCount(0), m_isStarConnection(0)
{
}

RttProtocol::~RttProtocol()
{
}

Result RttProtocol::Initialize(unsigned short stationCount, int initialPulseInterval)
{
    if (m_rttCalculators != 0)
        return 0x10c02;
    if (stationCount == 0 || stationCount > 32)
        return 0x10c07;

    RttCalculator::SetInitialPulseInterval(initialPulseInterval);
    RttCalculator* calculators = static_cast<RttCalculator*>(pead::allocateFromSeadHeap(
        static_cast<unsigned int>(stationCount) * sizeof(RttCalculator),
        common::HeapManager::GetHeap()));
    if (calculators == 0)
        return 0x10c01;

    for (unsigned int index = 0; index < stationCount; ++index)
        new (calculators + index) RttCalculator();
    m_rttCalculators = calculators;
    m_stationCount = stationCount;
    m_isStarConnection = 0;
    return 0;
}

void RttProtocol::Finalize()
{
    if (m_rttCalculators == 0)
        return;
    m_isStarConnection = 0;
    m_stationCount = 0;
    RttCalculator* calculators = m_rttCalculators;
    const unsigned int count = pead::getArraySize(calculators) / sizeof(RttCalculator);
    for (unsigned int index = 0; index < count; ++index)
        calculators[index].~RttCalculator();
    pead::freeToSeadHeap(calculators);
    m_rttCalculators = 0;
}

Result RttProtocol::StartupWithStationAddress(const common::StationAddress& address)
{
    if (m_rttCalculators == 0)
        return 0x10c08;
    Station* station = g_piaStationManager->GetStation(address);
    if (station == 0 || station->m_field10 >= 0x20)
        return 0x10c07;
    if (m_stationIndex != 0xfd)
        return 0x10c08;

    m_stationIndex = static_cast<StationIndex>(station->m_field10);
    if (station->m_field18 == 0 && station->m_field1c == 0)
        return 0x10c07;
    if (m_stationIdHigh != 0 || m_stationIdLow != 0)
        return 0x10c08;

    m_stationIdHigh = station->m_field18;
    m_stationIdLow = station->m_field1c;
    m_isStarConnection = g_piaTransport->m_networkFactory->IsStarConnection();
    return 0;
}

void RttProtocol::CleanupWithStationAddress()
{
    RttCalculator* calculators = m_rttCalculators;
    m_stationIndex = 0xfd;
    m_stationIdLow = 0;
    m_stationIdHigh = 0;
    m_isStarConnection = 0;
    if (calculators == 0 || m_stationCount == 0)
        return;
    for (unsigned int index = 0; index < m_stationCount; ++index)
        calculators[index].Cleanup();
}

Result RttProtocol::send(StationIndex stationIndex, const Data& data)
{
    const uintptr_t handlerValue = static_cast<uintptr_t>(GetStationPacketHandler());
    if (handlerValue == 0)
        return 0x10c08;
    if (stationIndex == m_stationIndex)
        return 0x10c08;

    unsigned int size = 0;
    data.Serialize(m_data, &size);
    StationPacketHandler* handler = reinterpret_cast<StationPacketHandler*>(handlerValue);
    ProtocolMessageWriter* writer = handler->AssignByStationIndex(
        mProtocolId, stationIndex, size, false, false, true);
    if (writer == 0)
        return 0;
    writer->SetPayload(m_data);
    return handler->Commit();
}

Result RttProtocol::request(StationIndex stationIndex)
{
    if ((stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount)) &&
        stationIndex != 0xfe)
        return 0x10c07;
    if (m_stationIndex == 0xfd)
        return 0x10c08;

    Data data;
    data.m_type = 0;
    data.m_sequence = 0;
    common::Time now;
    now.SetNow();
    data.m_timestamp = now.m_tick;
    return send(m_isStarConnection != 0 ? static_cast<StationIndex>(0xff) : stationIndex, data);
}

void RttProtocol::UpdateLastRequestedTimeAll()
{
    for (unsigned int index = 0; index < m_stationCount; ++index)
    {
        RttCalculator* calculator = m_rttCalculators + index;
        if (*reinterpret_cast<const unsigned char*>(calculator) != 0)
            calculator->SetLastRequestedTime(g_piaTransport->m_time);
    }
}

void RttProtocol::process(const ProtocolMessageReader* reader)
{
    const unsigned char* payload = reader->m_message + ProtocolMessageReader::cMessageHeaderSize;
    const unsigned int type = readU32(payload);
    const unsigned long long timestamp =
        (static_cast<unsigned long long>(readU32(payload + 8)) << 32) | readU32(payload + 12);
    Station* station = g_piaStationManager->GetStationByStationIdWithoutConvert(
        reader->m_header.m_source);
    if (station == 0 || station->m_field10 == 0xfd)
        return;

    const StationIndex stationIndex = static_cast<StationIndex>(station->m_field10);
    if (type == 0)
    {
        Data response;
        response.m_type = 1;
        response.m_sequence = 0;
        response.m_timestamp = timestamp;
        send(stationIndex, response);
        return;
    }
    if (type != 1)
        return;

    common::Time now;
    now.SetNow();
    const long long elapsedTicks = static_cast<long long>(now.m_tick - timestamp);
    const long long elapsedMilliseconds =
        elapsedTicks / common::TimeSpan::GetTicksPerMSec().m_tick;
    if (stationIndex >= 0 && static_cast<int>(stationIndex) < static_cast<int>(m_stationCount))
        m_rttCalculators[stationIndex].Update(static_cast<int>(elapsedMilliseconds));
}

Result RttProtocol::Dispatch()
{
    if (m_stationIndex == 0xfd)
        return 0;
    const uintptr_t handlerValue = static_cast<uintptr_t>(GetStationPacketHandler());
    if (handlerValue == 0)
        return 0x10c08;
    for (unsigned int index = 0; index < m_stationCount; ++index)
    {
        RttCalculator* calculator = m_rttCalculators + index;
        if (*reinterpret_cast<const unsigned char*>(calculator) != 0 && calculator->IsTimeOut())
        {
            request(static_cast<StationIndex>(index));
            if (m_isStarConnection != 0)
            {
                UpdateLastRequestedTimeAll();
                break;
            }
            else
                calculator->SetLastRequestedTime(g_piaTransport->m_time);
        }
    }

    StationPacketHandler* handler = reinterpret_cast<StationPacketHandler*>(handlerValue);
    PacketHandler::Iterator& iterator = handler->GetIterator(mProtocolId);
    while (!handler->IsEndIteration())
    {
        ProtocolMessageReader* reader = iterator.GetMessageReader();
        if (reader != 0)
        {
            if (reader->m_header.m_source ==
                ((static_cast<unsigned long long>(m_stationIdHigh) << 32) | m_stationIdLow))
                reader->Trace(0x40000);
            else
                process(reader);
        }
        handler->NextIteration();
    }
    return 0;
}

Result RttProtocol::UpdateProtocolEvent(const ProtocolEvent& event)
{
    if (m_stationIndex == 0xfd)
        return 0x10c08;
    Station* station = g_piaStationManager->GetStation(event.m_stationAddress);
    const StationIndex stationIndex = static_cast<StationIndex>(station->m_field10);
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return 0x10c07;
    if (stationIndex == m_stationIndex)
        return 0x10c07;

    RttCalculator& calculator = m_rttCalculators[stationIndex];
    if (event.m_type == 0)
    {
        calculator.Startup();
        request(stationIndex);
        if (m_isStarConnection == 0)
            calculator.SetLastRequestedTime(g_piaTransport->m_time);
        else
            UpdateLastRequestedTimeAll();
        return 0;
    }
    if (event.m_type == 1)
    {
        calculator.Cleanup();
        return 0;
    }
    return 0x10c07;
}

int RttProtocol::GetRtt(StationIndex stationIndex) const
{
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return -1;
    return m_rttCalculators[stationIndex].GetRtt();
}

int RttProtocol::GetRtt(StationIndex stationIndex, unsigned int count) const
{
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return -1;
    return m_rttCalculators[stationIndex].GetRtt(count);
}

void RttProtocol::Trace(unsigned long long) const
{
}

void RttProtocol::ResetRttMinMax(StationIndex stationIndex)
{
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return;
    m_rttCalculators[stationIndex].ResetRttMinMax();
}

int RttProtocol::GetRttMin(StationIndex stationIndex) const
{
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return -1;
    return m_rttCalculators[stationIndex].GetRttMin();
}

int RttProtocol::GetRttMax(StationIndex stationIndex) const
{
    if (stationIndex < 0 || static_cast<int>(stationIndex) >= static_cast<int>(m_stationCount))
        return -1;
    return m_rttCalculators[stationIndex].GetRttMax();
}

unsigned short RttProtocol::GetProtocolType() const
{
    return 0x58;
}

}}}
