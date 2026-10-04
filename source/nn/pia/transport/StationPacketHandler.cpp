#include "nn/pia/transport/StationPacketHandler.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/Station.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"
#include "nn/pia/common/Packet.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace transport {

StationPacketHandler::StationPacketHandler()
{
}

StationPacketHandler::~StationPacketHandler()
{
}

Result StationPacketHandler::Initialize(unsigned int stationNum, bool isStarConnection,
                                       unsigned int maxProtocolNum)
{
    if (stationNum == 0)
        return 0x10c07;
    const Result result = PacketHandler::Initialize(maxProtocolNum);
    if (result != 0)
        return result;

    m_isStarConnection = isStarConnection;
    m_localStationIndex = 0xfd;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    m_stationNum = stationNum;
    return 0;
}

void StationPacketHandler::Finalize()
{
    m_localStationIndex = 0xfd;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    PacketHandler::Finalize();
}

void StationPacketHandler::Cleanup()
{
    m_localStationIndex = 0xfd;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    PacketHandler::Cleanup();
}

unsigned int StationPacketHandler::GenerateConnectionId() const
{
    common::Time time;
    time.SetNow();
    return static_cast<unsigned int>(time.m_tick);
}

void StationPacketHandler::Trace(unsigned long long value) const
{
}

unsigned int StationPacketHandler::IsBroadcastSupported() const
{
    return m_isStarConnection;
}

unsigned int StationPacketHandler::CheckPacket(const common::Packet& packet)
{
    StationManager* manager = GetStationManager();
    Station* station = manager->GetStation(packet.m_field608);
    if (station == 0)
        return packet.m_size;

    const unsigned char passId = packet.m_packetData.m_header.m_field09;
    const unsigned char receivingPassId = station->m_field55;
    if (passId != 0 && passId != 1 && receivingPassId != 0 && passId != receivingPassId)
        return 0;
    if (m_localStationIndex < 32 &&
        !station->m_sequenceIdController.CheckReceivedSequenceId(packet.m_packetData.m_header.m_field0a))
        return 0;
    if (m_localStationIndex != static_cast<unsigned int>(manager->m_invalidStationIndex) &&
        station->m_field20 == 5 && station->m_field10 == 0xfd)
        return 0;
    return packet.m_size;
}

unsigned int StationPacketHandler::CheckMessage(const ProtocolMessageReader& reader)
{
    if ((reader.m_header.m_flags & 2) != 0)
        return 0;
    if ((reader.m_header.m_flags & 1) == 0)
        return reader.GetDestination() == m_localStationId;
    if ((reader.GetDestination() >> 32) != 0)
        return 0;
    if (IsBroadcastSupported() != 0 && m_localStationBitmap != 0 &&
        reader.GetDestination() != 0 &&
        (static_cast<unsigned int>(reader.GetDestination()) & m_localStationBitmap) == 0)
        return 0;
    return 1;
}

unsigned int StationPacketHandler::CheckReceive(const ProtocolMessageReader& reader)
{
    if ((reader.m_header.m_flags & 2) != 0)
        return 0;
    if ((reader.m_header.m_flags & 1) == 0)
        return reader.GetDestination() == m_localStationId;
    const unsigned int stationBitmap = static_cast<unsigned int>(reader.GetDestination());
    return stationBitmap == 0 || (stationBitmap & m_localStationBitmap) != 0;
}

StationManager* StationPacketHandler::GetStationManager()
{
    return g_piaStationManager;
}

ProtocolMessageWriter* StationPacketHandler::AssignByStationBitmap(
    const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
    bool isReliable, bool isPriority, bool isBroadcast)
{
    const unsigned int participating = GetParticipatingStationBitmap(GetStationManager());
    return AssignMulti(protocolId, stationBitmap & participating, size,
                       isReliable, isPriority, isBroadcast);
}

void StationPacketHandler::EndDispatch(const common::Time& time)
{
    const unsigned int stationBitmap = m_keepAliveSender.Update(m_keepAliveStationBitmap, time);
    m_keepAliveStationBitmap = 0;
    if (stationBitmap != 0)
    {
        const ProtocolId protocolId(0x0800);
        if (AssignByStationBitmap(protocolId, stationBitmap, 0, false, false, true) != 0)
        {
            Commit();
            if (m_keepAliveStationBitmap != 0)
            {
                m_keepAliveSender.Update(m_keepAliveStationBitmap, time);
                m_keepAliveStationBitmap = 0;
            }
        }
    }
    PacketHandler::EndDispatch(time);
}

void StationPacketHandler::UpdateLocalStation()
{
    if (m_localStationIndex < 32)
        return;
    const Station* station = GetStationManager()->m_localStation;
    if (station == 0)
        return;
    m_localStationIndex = station->m_field10;
    if (m_localStationIndex < 32)
    {
        m_localStationId = (static_cast<unsigned long long>(station->m_field18) << 32) |
                           station->m_field1c;
        m_localStationBitmap = 1u << m_localStationIndex;
        m_protocolMessageWriter.SetSource(m_localStationId);
    }
}

void StationPacketHandler::BeginDispatch(const common::Time& time)
{
    UpdateLocalStation();
    PacketHandler::BeginDispatch(time);
    StationManager* manager = GetStationManager();
    unsigned int stationBitmap = 0;
    unsigned int index = static_cast<unsigned int>(m_reader->m_field04);
    while (index != static_cast<unsigned int>(m_reader->m_field08))
    {
        PacketStream* stream = m_reader->m_packetStream;
        const common::Packet* packet = index < stream->m_packetNum ? &stream->m_packets[index] : 0;
        if (packet->m_field614 != 0)
        {
            if (packet->m_packetData.m_header.m_field09 != 1)
            {
                const Station* station = manager->GetStation(packet->m_field608);
                if (station != 0 && station->m_field10 < 32)
                    stationBitmap |= 1u << station->m_field10;
            }
            unsigned int offset = 0;
            while (offset < packet->m_size - common::Packet::cHeaderSize)
            {
                m_protocolMessageReader.Attach(*packet, offset);
                if (m_protocolMessageReader.m_message == 0)
                    break;
                if (m_protocolMessageReader.m_header.m_source != 0)
                {
                    const Station* station = manager->GetStation(m_protocolMessageReader.m_header.m_source);
                    if (station != 0)
                    {
                        const unsigned int shift = station->m_field10 & 63;
                        if (shift < 32)
                            stationBitmap |= 1u << shift;
                    }
                }
                offset += ((m_protocolMessageReader.m_messageSize + 23u) & ~3u) + 4u;
            }
        }
        ++index;
        if (static_cast<int>(index) >= static_cast<int>(m_reader->m_packetStream->m_packetNum))
            index = 0;
    }
    m_keepAliveReceiver.Update(stationBitmap, time);
}

unsigned int StationPacketHandler::GetParticipatingStationBitmap(const StationManager* manager)
{
    return manager->GetParticipatingStationBitmap(false);
}

ProtocolMessageWriter* StationPacketHandler::AssignByStationIndex(
    const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
    unsigned int size, bool isReliable, bool isPriority, bool isBroadcast)
{
    if (stationIndex == 0xff)
    {
        if (IsBroadcastSupported() != 0)
            return AssignAll(protocolId, size, isReliable, isPriority, isBroadcast);
        const unsigned int stationBitmap = GetParticipatingStationBitmap(GetStationManager());
        return AssignMulti(protocolId, stationBitmap, size, isReliable, isPriority, isBroadcast);
    }
    return AssignSingle(protocolId, stationIndex, size, isReliable, isPriority, isBroadcast);
}

StationConnectionInfoTable* StationPacketHandler::GetStationConnectionInfoTable()
{
    return g_stationConnectionInfoTable;
}

Result StationPacketHandler::Startup(PacketStream::Writer* writer,
                                     PacketStream::Reader* reader,
                                     common::IPacketInput* packetInput,
                                     common::IPacketOutput* packetOutput,
                                     unsigned long long* stationId,
                                     const common::CryptoSetting* cryptoSetting)
{
    const Result result = PacketHandler::Startup(writer, reader, packetInput, packetOutput,
                                                 stationId, cryptoSetting);
    if (result != 0)
        return result;

    m_localStationIndex = 0xfd;
    m_localStationId = 0;
    m_localStationBitmap = 0;
    m_protocolMessageWriter.SetSource(0);
    m_keepAliveStationBitmap = 0;
    return 0;
}

common::Packet* StationPacketHandler::AssignPacket(
    ::nn::pia::StationIndex stationIndex, unsigned int stationBitmap,
    const common::StationAddress& address, bool isPriority, bool field602, bool field604)
{
    Station* station = 0;
    unsigned char sendingPassId = 0;
    if (m_localStationIndex < 32)
    {
        if (static_cast<unsigned int>(stationIndex) < 32)
        {
            station = GetStationManager()->GetStation(stationIndex);
            if (station != 0 && field604)
            {
                sendingPassId = station->m_field54;
                if (sendingPassId == 0)
                {
                    station->Trace(0x8000);
                    return 0;
                }
            }
        }
    }
    else
        sendingPassId = 1;

    common::Packet* packet = PacketHandler::AssignPacket(address, isPriority, field602);
    if (packet == 0)
        return 0;
    const unsigned short sequenceId = station != 0 ?
        station->m_sequenceIdController.GetNextSendSequenceId() : 0;
    packet->m_stationBitmap = stationBitmap;
    packet->m_packetData.m_header.m_field09 = sendingPassId;
    packet->m_packetData.m_header.m_field0a = sequenceId;
    packet->m_stationIndex = stationIndex;
    m_keepAliveStationBitmap |= stationBitmap;
    return packet;
}

bool StationPacketHandler::AssignHostMessage(unsigned int size, unsigned int sizeLimit,
                                              bool isPriority, bool field602, bool field604)
{
    common::Packet* packet = 0;
    if (!isPriority)
    {
        unsigned int index = static_cast<unsigned int>(m_writer->m_field04);
        while (index != static_cast<unsigned int>(m_writer->m_field08))
        {
            PacketStream* stream = m_writer->m_packetStream;
            common::Packet* candidate = index < stream->m_packetNum ? &stream->m_packets[index] : 0;
            if (IsBundleCandidate(candidate, field602, sizeLimit, field604) &&
                candidate->m_stationIndex == 0xfe)
            {
                packet = candidate;
                break;
            }
            ++index;
            if (static_cast<int>(index) >= static_cast<int>(stream->m_packetNum))
                index = 0;
        }
    }
    if (packet == 0)
    {
        const common::StationAddress address(GetStationConnectionInfoTable()->m_hostStationAddress);
        packet = AssignPacket(static_cast< ::nn::pia::StationIndex>(0xfe), 0, address, isPriority, field602, field604);
        if (packet == 0)
            return false;
    }
    void* payload = AssignPacketPayload(packet, size);
    m_protocolMessageWriter.AddMessageBuffer(packet, payload, 0, true, false);
    return true;
}

bool StationPacketHandler::AssignDirectMessage(unsigned int stationBitmap, unsigned int size,
                                                unsigned int sizeLimit, bool isPriority,
                                                bool field602, bool field604)
{
    if (stationBitmap == 0)
        return true;
    StationManager* manager = GetStationManager();
    unsigned int index = static_cast<unsigned int>(isPriority ? m_writer->m_field08 : m_writer->m_field04);
    while (stationBitmap != 0)
    {
        if (index != static_cast<unsigned int>(m_writer->m_field08))
        {
            PacketStream* stream = m_writer->m_packetStream;
            common::Packet* packet = index < stream->m_packetNum ? &stream->m_packets[index] : 0;
            ++index;
            if (static_cast<int>(index) >= static_cast<int>(stream->m_packetNum))
                index = 0;
            if (!IsBundleCandidate(packet, field602, sizeLimit, field604))
                continue;
            const unsigned int packetBitmap = packet->m_stationBitmap;
            if (packetBitmap == 0 || (packetBitmap & stationBitmap) != packetBitmap)
                continue;
            stationBitmap &= ~packetBitmap;
            void* payload = AssignPacketPayload(packet, size);
            m_protocolMessageWriter.AddMessageBuffer(packet, payload, packetBitmap, true, false);
            continue;
        }

        const int bit = pead::BitFlagUtil::findOnBitFromRight(stationBitmap, m_stationNum);
        const unsigned int selected = bit < 0 || bit == 31 ? stationBitmap :
            stationBitmap & ((1u << (bit + 1)) - 1u);
        stationBitmap &= ~selected;
        common::Packet* packet;
        {
            common::StationAddress address;
            ::nn::pia::StationIndex stationIndex = static_cast< ::nn::pia::StationIndex>(0xfd);
            if (pead::BitFlagUtil::countOnBit(selected) == 1)
            {
                stationIndex = static_cast< ::nn::pia::StationIndex>(
                    pead::BitFlagUtil::countOnBit((selected & (0u - selected)) - 1u));
                if (manager->GetStationAddress(&address, stationIndex) != 0)
                    continue;
            }
            else
                address.Clear();
            packet = AssignPacket(stationIndex, selected, address,
                                                  isPriority, field602, field604);
            if (packet == 0)
                return false;
            ++index;
            if (static_cast<int>(index) >= static_cast<int>(m_writer->m_packetStream->m_packetNum))
                index = 0;
        }
        void* payload = AssignPacketPayload(packet, size);
        m_protocolMessageWriter.AddMessageBuffer(packet, payload, selected, true, false);
    }
    return true;
}

bool StationPacketHandler::AssignAllMessage(unsigned int stationBitmap, unsigned int size,
                                             unsigned int sizeLimit, bool isPriority,
                                             bool field602, bool field604)
{
    if (stationBitmap == 0)
        return true;
    common::Packet* packet = 0;
    if (!isPriority)
    {
        unsigned int index = static_cast<unsigned int>(m_writer->m_field04);
        while (index != static_cast<unsigned int>(m_writer->m_field08))
        {
            PacketStream* stream = m_writer->m_packetStream;
            common::Packet* candidate = index < stream->m_packetNum ? &stream->m_packets[index] : 0;
            if (IsBundleCandidate(candidate, field602, sizeLimit, field604) &&
                candidate->m_stationIndex == 0xff)
            {
                packet = candidate;
                break;
            }
            ++index;
            if (static_cast<int>(index) >= static_cast<int>(stream->m_packetNum))
                index = 0;
        }
    }
    if (packet == 0)
    {
        common::StationAddress address;
        packet = AssignPacket(static_cast< ::nn::pia::StationIndex>(0xff), stationBitmap, address, isPriority, field602, field604);
        if (packet == 0)
            return false;
    }
    void* payload = AssignPacketPayload(packet, size);
    m_protocolMessageWriter.AddMessageBuffer(packet, payload, stationBitmap, true, false);
    return true;
}

ProtocolMessageWriter* StationPacketHandler::AssignSingle(
    const ProtocolId& protocolId, ::nn::pia::StationIndex stationIndex,
    unsigned int size, bool isPriority, bool field602, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    m_protocolMessageWriter.SetSource(m_localStationId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    if (stationIndex == 0xfe)
        AssignHostMessage(alignedSize, sizeLimit, isPriority, field602, field604);
    else if (static_cast<unsigned int>(stationIndex) < 32 && m_localStationIndex != 0xfd)
        AssignDirectMessage(1u << stationIndex, alignedSize, sizeLimit, isPriority, field602, field604);
    else
        return 0;
    return ReserveMessageWriter();
}

ProtocolMessageWriter* StationPacketHandler::AssignAll(
    const ProtocolId& protocolId, unsigned int size, bool isPriority, bool field602, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    if (m_localStationIndex == 0xfd)
        return 0;
    const unsigned int stationBitmap = GetParticipatingStationBitmap(GetStationManager());
    if (stationBitmap == 0)
        return 0;
    m_protocolMessageWriter.SetSource(m_localStationId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    AssignAllMessage(stationBitmap, alignedSize, sizeLimit, isPriority, field602, field604);
    return ReserveMessageWriter();
}

ProtocolMessageWriter* StationPacketHandler::AssignMulti(
    const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
    bool isPriority, bool field602, unsigned long long sourceId, bool field604)
{
    if (m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23u) & ~3u) + 4u;
    if (alignedSize > m_payloadSize)
        return 0;
    const unsigned int sizeLimit = m_packetSize - alignedSize;
    UpdateLocalStation();
    if (m_localStationIndex == 0xfd)
        return 0;
    m_protocolMessageWriter.SetSource(sourceId);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    AssignDirectMessage(stationBitmap, alignedSize, sizeLimit, isPriority, field602, field604);
    return ReserveMessageWriter();
}

ProtocolMessageWriter* StationPacketHandler::AssignMulti(
    const ProtocolId& protocolId, unsigned int stationBitmap, unsigned int size,
    bool isPriority, bool field602, bool field604)
{
    return AssignMulti(protocolId, stationBitmap, size, isPriority, field602,
                       m_localStationId, field604);
}

}}}
