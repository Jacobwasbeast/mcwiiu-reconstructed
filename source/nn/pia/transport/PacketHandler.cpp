#include "nn/pia/transport/PacketHandler.hpp"

#include "nn/pia/PayloadSizeManager.hpp"
#include "nn/pia/common/Crypto.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/common/Time.hpp"
#include "nn/pia/common/IPacketInput.hpp"
#include "nn/pia/common/IPacketOutput.hpp"
#include "nn/pia/transport/PacketAnalyzer.hpp"
#include "nn/pia/HeapManager.hpp"
#include "nn/pia/transport/StationManager.hpp"
#include "nn/pia/transport/StationConnectionInfoTable.hpp"

#include <cstring>
#include <new>

namespace nn { namespace pia { namespace transport {

namespace
{
unsigned int nextPacketIndex(const PacketStream::Accessor& accessor, unsigned int index)
{
    ++index;
    return static_cast<int>(index) < static_cast<int>(accessor.m_packetStream->m_packetNum) ? index : 0;
}

common::Packet* packetAt(const PacketStream::Accessor& accessor, unsigned int index)
{
    return index < accessor.m_packetStream->m_packetNum ? accessor.m_packetStream->m_packets + index : 0;
}
}

PacketHandler::PacketHandler()
    : m_writer(0), m_reader(0), m_pendingWriter(0), m_iterator(this),
      m_sendPacketAnalyzer(0), m_sendUnicastPacketAnalyzer(0),
      m_sendBroadcastPacketAnalyzer(0), m_receivePacketAnalyzer(0),
      m_stationId(0), m_packetInput(0), m_packetOutput(0)
{
}

PacketHandler::~PacketHandler()
{
}

ProtocolMessageReader* PacketHandler::Iterator::GetMessageReader() const
{
    return m_handler->m_protocolMessageReader.m_message != 0
        ? &m_handler->m_protocolMessageReader : 0;
}

PacketHandler::Iterator& PacketHandler::GetIterator(const ProtocolId& protocolId)
{
    m_field35c = protocolId.value();
    m_field35e = 0xffff;
    BeginIteration();
    return m_iterator;
}

PacketHandler::Iterator& PacketHandler::GetIterator(unsigned char protocolType)
{
    m_field35e = 0xff00;
    m_field35c = static_cast<unsigned short>(protocolType) << 8;
    BeginIteration();
    return m_iterator;
}

Result PacketHandler::Initialize(unsigned int maxProtocolNum)
{
    m_payloadSize = 0;
    m_packetSize = 0;
    m_reader = 0;
    m_writer = 0;
    PacketAnalyzer** analyzers[] = { &m_sendPacketAnalyzer, &m_sendUnicastPacketAnalyzer,
                                    &m_sendBroadcastPacketAnalyzer, &m_receivePacketAnalyzer };
    const char* names[] = { "Pia Send", "Pia Send Unicast", "Pia Send Broadcast", "Pia Receive" };
    for (unsigned int index = 0; index < 4; ++index)
    {
        pead::Heap* heap = common::HeapManager::GetHeap();
        void* storage = pead::allocateFromSeadHeap(sizeof(PacketAnalyzer), heap);
        *analyzers[index] = storage != 0 ? new (storage) PacketAnalyzer(names[index], maxProtocolNum) : 0;
        if (*analyzers[index] == 0)
            return static_cast<Result>(0x10c01);
    }
    m_cryptoSetting.m_mode = 0;
    return 0;
}

void PacketHandler::Finalize()
{
    PacketAnalyzer** analyzers[] = { &m_receivePacketAnalyzer, &m_sendBroadcastPacketAnalyzer,
                                    &m_sendUnicastPacketAnalyzer, &m_sendPacketAnalyzer };
    for (unsigned int index = 0; index < 4; ++index)
    {
        PacketAnalyzer* analyzer = *analyzers[index];
        if (analyzer != 0)
        {
            analyzer->~PacketAnalyzer();
            pead::freeToSeadHeap(analyzer);
            *analyzers[index] = 0;
        }
    }
    m_packetSize = 0;
    m_reader = 0;
    m_payloadSize = 0;
    m_writer = 0;
    m_cryptoSetting = common::CryptoSetting();
}

void PacketHandler::Cleanup()
{
    PacketAnalyzer* analyzers[] = { m_receivePacketAnalyzer, m_sendBroadcastPacketAnalyzer,
                                  m_sendUnicastPacketAnalyzer, m_sendPacketAnalyzer };
    for (unsigned int index = 0; index < 4; ++index)
    {
        if (analyzers[index] != 0)
            analyzers[index]->Cleanup();
    }
    m_packetSize = 0;
    m_payloadSize = 0;
    m_reader = 0;
    m_writer = 0;
    m_cryptoSetting = common::CryptoSetting();
}

Result PacketHandler::Startup(PacketStream::Writer* writer, PacketStream::Reader* reader,
                              common::IPacketInput* packetInput,
                              common::IPacketOutput* packetOutput,
                              unsigned long long* stationId,
                              const common::CryptoSetting* cryptoSetting)
{
    const unsigned int packetSize =
        (common::g_payloadSizeManager->m_mtuSize & ~3u) -
        common::g_payloadSizeManager->m_signatureSize;

    if (writer == 0 || reader == 0 || packetInput == 0 || packetOutput == 0 || stationId == 0)
        return static_cast<Result>(0x10c07);
    if (cryptoSetting != 0 && cryptoSetting->m_mode > 1)
        return static_cast<Result>(0x10c07);
    if (m_writer != 0 || m_reader != 0)
        return static_cast<Result>(0x10c08);

    m_packetInput = packetInput;
    m_writer = writer;
    m_reader = reader;
    m_packetOutput = packetOutput;
    m_protocolMessageWriter.SetSource(0);
    m_pendingWriter = 0;

    if (packetSize - cPacketHeaderSize > cPayloadSizeMax)
        return static_cast<Result>(0x10c07);

    if (cryptoSetting == 0)
        m_cryptoSetting = common::CryptoSetting();
    else
        m_cryptoSetting = *cryptoSetting;

    if (m_cryptoSetting.m_mode == 0)
    {
        m_packetSize = packetSize;
    }
    else if (m_cryptoSetting.m_mode == 1)
    {
        const unsigned int blockSize = common::Crypto::GetBlockSize(common::Crypto::cModeAesGcm);
        m_packetSize =
            ((packetSize - cPacketHeaderSize) / blockSize) * blockSize + cPacketHeaderSize;
    }
    m_payloadSize = m_packetSize - cPacketHeaderSize;

    m_sendPacketAnalyzer->Startup();
    m_sendPacketAnalyzer->ClearPacketAnalysisData();
    m_sendUnicastPacketAnalyzer->Startup();
    m_sendUnicastPacketAnalyzer->ClearPacketAnalysisData();
    m_sendBroadcastPacketAnalyzer->Startup();
    m_sendBroadcastPacketAnalyzer->ClearPacketAnalysisData();
    m_receivePacketAnalyzer->Startup();
    m_receivePacketAnalyzer->ClearPacketAnalysisData();

    m_stationId = stationId;
    return 0;
}

int PacketHandler::GetPayloadSizeLimit() const
{
    return static_cast<int>(m_payloadSize) - 24;
}

Result PacketHandler::GetPacketAnalysisData(PacketAnalysisData* send,
                                             PacketAnalysisData* unicast,
                                             PacketAnalysisData* broadcast,
                                             PacketAnalysisData* receive) const
{
    if (send == 0 && receive == 0)
        return static_cast<Result>(0x10c07);
    PacketAnalysisData* outputs[] = { send, unicast, broadcast, receive };
    PacketAnalyzer* analyzers[] = { m_sendPacketAnalyzer, m_sendUnicastPacketAnalyzer,
                                   m_sendBroadcastPacketAnalyzer, m_receivePacketAnalyzer };
    for (unsigned int index = 0; index < 4; ++index)
    {
        if (outputs[index] == 0)
            continue;
        if (analyzers[index] == 0)
            return static_cast<Result>(0x10c08);
        const Result result = analyzers[index]->GetPacketAnalysisData(outputs[index]);
        if (result != 0)
            return result;
    }
    return 0;
}

void PacketHandler::ClearPacketAnalysisData()
{
    if (m_sendPacketAnalyzer != 0)
        m_sendPacketAnalyzer->ClearPacketAnalysisData();
    if (m_sendUnicastPacketAnalyzer != 0)
        m_sendUnicastPacketAnalyzer->ClearPacketAnalysisData();
    if (m_sendBroadcastPacketAnalyzer != 0)
        m_sendBroadcastPacketAnalyzer->ClearPacketAnalysisData();
    if (m_receivePacketAnalyzer != 0)
        m_receivePacketAnalyzer->ClearPacketAnalysisData();
}

unsigned char* PacketHandler::AssignPacketPayload(common::Packet* packet, unsigned int size)
{
    unsigned char* payload = packet->AssignPayload(size);
    std::memset(payload + (size & ~3U) - 4, 0, 4);
    return payload;
}

bool PacketHandler::IsBundleCandidate(const common::Packet* packet, bool field602,
                                       unsigned int sizeLimit, bool field604)
{
    return packet->m_field601 == 0 &&
        (m_cryptoSetting.m_mode == 0 || packet->m_field602 == field602) &&
        packet->m_size <= sizeLimit && packet->m_field604 == field604;
}

common::Packet* PacketHandler::AssignPacket(const common::StationAddress& address,
                                            bool isPriority, bool field602)
{
    common::Packet* packet = m_writer->Assign();
    if (packet == 0)
    {
        m_writer->m_packetStream->Trace(0x80000);
        return 0;
    }
    packet->Reset();
    packet->m_field5f4 = address;
    packet->m_field601 = isPriority;
    packet->m_field602 = field602;
    return packet;
}

ProtocolMessageWriter* PacketHandler::AssignByStationAddress(const ProtocolId& protocolId,
                                                             const common::StationAddress& address,
                                                             unsigned int size, bool isPriority,
                                                             bool field602)
{
    if (!address.IsValid() || m_writer == 0 || m_pendingWriter != 0)
        return 0;
    const unsigned int alignedSize = ((size + 23U) & ~3U) + 4U;
    if (alignedSize > m_payloadSize)
        return 0;

    common::Packet* packet = 0;
    if (!isPriority)
    {
        unsigned int index = static_cast<unsigned int>(m_writer->m_field04);
        while (index != static_cast<unsigned int>(m_writer->m_field08))
        {
            PacketStream* stream = m_writer->m_packetStream;
            common::Packet* candidate = index < stream->m_packetNum ? stream->m_packets + index : 0;
            if (IsBundleCandidate(candidate, field602, m_packetSize - alignedSize, true) &&
                candidate->m_field5f4 == address)
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
        packet = AssignPacket(address, isPriority, field602);
    if (packet == 0)
        return 0;

    unsigned char* payload = AssignPacketPayload(packet, alignedSize);
    m_protocolMessageWriter.Reset(protocolId, size, false, isPriority);
    m_protocolMessageWriter.AddMessageBuffer(packet, payload, 0, true, false);
    m_pendingWriter = &m_protocolMessageWriter;
    return m_pendingWriter;
}

Result PacketHandler::Commit()
{
    if (m_pendingWriter == 0)
        return static_cast<Result>(0x10c08);
    m_pendingWriter->Commit();
    m_pendingWriter = 0;
    return 0;
}

ProtocolMessageWriter* PacketHandler::ReserveMessageWriter()
{
    if (m_protocolMessageWriter.m_messageBufferNum != 0)
        m_pendingWriter = &m_protocolMessageWriter;
    return m_pendingWriter;
}

void PacketHandler::Trace(unsigned long long) const
{
}

void PacketHandler::RelayMessage(const ProtocolMessageReader&)
{
}

unsigned int PacketHandler::CheckPacket(const common::Packet& packet)
{
    return packet.m_size;
}

unsigned int PacketHandler::CheckMessage(const ProtocolMessageReader&)
{
    return 1;
}

unsigned int PacketHandler::CheckReceive(const ProtocolMessageReader&)
{
    return 1;
}

unsigned int PacketHandler::IsBroadcastSupported() const
{
    return 0;
}

bool PacketHandler::CheckFiltering(const common::StationAddress& address) const
{
    Station* station = g_piaStationManager->GetStation(address);
    if (station == 0)
        return false;
    StationConnectionInfo info;
    return g_stationConnectionInfoTable->GetStationConnectionInfo(station, &info) == 0;
}

void PacketHandler::BeginIteration()
{
    m_field360 = static_cast<unsigned int>(-1);
    m_field364 = 0;
    m_protocolMessageReader.Clear();
    NextIteration();
}

unsigned int PacketHandler::GenerateConnectionId() const
{
    common::Time now;
    now.SetNow();
    return static_cast<unsigned int>(now.m_tick);
}

bool PacketHandler::IsEndIteration() const
{
    return static_cast<int>(m_reader->m_field0c) <= static_cast<int>(m_field360);
}

void PacketHandler::NextIteration()
{
    if (IsEndIteration())
        return;
    for (;;)
    {
        if (m_protocolMessageReader.m_message == 0)
        {
            ++m_field360;
            m_field364 = 0;
            if (static_cast<int>(m_reader->m_field0c) <= static_cast<int>(m_field360))
                return;
            common::Packet* packet = m_reader->Get(static_cast<int>(m_field360));
            if (packet->m_field614 == 0)
                continue;
            m_protocolMessageReader.Attach(*packet, m_field364);
        }
        else
        {
            common::Packet* packet = m_reader->Get(static_cast<int>(m_field360));
            m_field364 += ((m_protocolMessageReader.m_messageSize + 23U) & ~3U) + 4U;
            if (m_field364 < packet->m_size - cPacketHeaderSize)
                m_protocolMessageReader.Attach(*packet, m_field364);
            else
                m_protocolMessageReader.Clear();
        }
        if (m_protocolMessageReader.m_message == 0)
            continue;
        const unsigned short protocolId = m_protocolMessageReader.m_header.m_protocolId;
        if ((protocolId & m_field35e) != m_field35c)
            continue;
        if ((m_protocolMessageReader.m_header.m_flags & 4) == 0 &&
            g_piaProtocolMessageFilteringManager != 0 &&
            g_piaProtocolMessageFilteringManager->IsFilteringEnabled(
                static_cast<unsigned char>(protocolId >> 8)) != 0 &&
            !CheckFiltering(m_protocolMessageReader.m_stationAddress))
        {
            m_protocolMessageReader.m_stationAddress.Trace(0x800000);
            continue;
        }
        if (CheckReceive(m_protocolMessageReader) != 0)
            return;
    }
}

void PacketHandler::BeginDispatch(const common::Time&)
{
    m_reader->PullAll();
    common::Crypto::Setting setting;
    setting.m_mode = common::Crypto::cModeNone;
    setting.m_key = 0;
    setting.m_keyLength = 0;
    if (m_cryptoSetting.m_mode == 1)
    {
        setting.m_mode = common::Crypto::cModeAesGcm;
        setting.m_key = m_cryptoSetting.m_key;
        setting.m_keyLength = sizeof(m_cryptoSetting.m_key);
    }
    for (unsigned int index = m_reader->m_field04;
         index != static_cast<unsigned int>(m_reader->m_field08);
         index = nextPacketIndex(*m_reader, index))
    {
        common::Packet* packet = packetAt(*m_reader, index);
        if (packet->IsValid() == 0)
        {
            packet->m_field614 = 0;
            continue;
        }
        if (packet->m_packetData.m_header.m_version != 1)
        {
            unsigned char parameter[12] = {};
            if (m_cryptoSetting.m_mode == 1)
            {
                if (m_packetInput == 0 ||
                    m_packetInput->CalculateCryptoParameter(parameter, sizeof(parameter), packet) != 0)
                {
                    packet->m_field614 = 0;
                    continue;
                }
                setting.m_iv = parameter;
                setting.m_ivLength = sizeof(parameter);
            }
            if (packet->Decrypt(setting) != 0 || packet->m_packetData.m_header.m_version != 1)
            {
                packet->m_field614 = 0;
                continue;
            }
        }
        const unsigned int size = CheckPacket(*packet);
        if (size < cPacketHeaderSize)
        {
            packet->m_field614 = 0;
            continue;
        }
        packet->m_size = size;
        unsigned char accepted = 1;
        unsigned char anyAccepted = 0;
        unsigned char hasRelay = 0;
        unsigned int offset = 0;
        while (offset < packet->m_size - cPacketHeaderSize)
        {
            m_protocolMessageReader.Attach(*packet, offset);
            if (m_protocolMessageReader.m_message == 0)
            {
                if (!m_protocolMessageReader.m_isTerminator)
                    accepted = 0;
                break;
            }
            offset += ((m_protocolMessageReader.m_messageSize + 23U) & ~3U) + 4U;
            const bool broadcast = IsBroadcastSupported() != 0;
            const unsigned char result = static_cast<unsigned char>(CheckMessage(m_protocolMessageReader));
            if (broadcast)
                anyAccepted |= result;
            else
            {
                accepted &= result;
                if (accepted == 0)
                    break;
            }
            hasRelay |= (m_protocolMessageReader.m_header.m_flags >> 1) & 1;
        }
        if (IsBroadcastSupported() != 0)
            accepted &= anyAccepted;
        packet->m_field614 = accepted;
        if (accepted != 0 && hasRelay != 0)
        {
            for (offset = 0; offset < packet->m_size - cPacketHeaderSize;
                 offset += ((m_protocolMessageReader.m_messageSize + 23U) & ~3U) + 4U)
            {
                m_protocolMessageReader.Attach(*packet, offset);
                if (m_protocolMessageReader.m_message == 0)
                    break;
                if ((m_protocolMessageReader.m_header.m_flags & 2) != 0)
                    RelayMessage(m_protocolMessageReader);
            }
        }
    }
    if (m_receivePacketAnalyzer != 0)
    {
        for (unsigned int index = m_reader->m_field04;
             index != static_cast<unsigned int>(m_reader->m_field08);
             index = nextPacketIndex(*m_reader, index))
            m_receivePacketAnalyzer->UpdatePacketAnalysisData(*packetAt(*m_reader, index), false, false);
    }
}

void PacketHandler::EndDispatch(const common::Time&)
{
    if (m_pendingWriter != 0)
        return;
    if (m_sendPacketAnalyzer != 0)
    {
        for (unsigned int index = m_writer->m_field04;
             index != static_cast<unsigned int>(m_writer->m_field08);
             index = nextPacketIndex(*m_writer, index))
        {
            common::Packet* packet = packetAt(*m_writer, index);
            const bool broadcast = packet->m_stationIndex == 0xff;
            m_sendPacketAnalyzer->UpdatePacketAnalysisData(*packet, true, broadcast);
            PacketAnalyzer* analyzer = broadcast ? m_sendBroadcastPacketAnalyzer : m_sendUnicastPacketAnalyzer;
            if (analyzer != 0)
                analyzer->UpdatePacketAnalysisData(*packet, true, broadcast);
        }
    }
    if (m_cryptoSetting.m_mode != 0)
    {
        common::Crypto::Setting setting;
        if (m_cryptoSetting.m_mode == 1)
        {
            setting.m_mode = common::Crypto::cModeAesGcm;
            setting.m_key = m_cryptoSetting.m_key;
            setting.m_keyLength = sizeof(m_cryptoSetting.m_key);
        }
        for (unsigned int index = m_writer->m_field04;
             index != static_cast<unsigned int>(m_writer->m_field08);
             index = nextPacketIndex(*m_writer, index))
        {
            common::Packet* packet = packetAt(*m_writer, index);
            if (packet->m_field602 != 0)
                continue;
            common::Time now;
            now.SetNow();
            unsigned char parameter[12];
            for (unsigned int byte = 0; byte < 8; ++byte)
                parameter[byte] = static_cast<unsigned char>(now.m_tick >> (56 - byte * 8));
            if (m_cryptoSetting.m_mode == 1 && m_packetOutput != 0)
            {
                ++*m_stationId;
                for (unsigned int byte = 0; byte < 8; ++byte)
                    packet->m_packetData.m_header.m_field10[byte] =
                        static_cast<unsigned char>(*m_stationId >> (56 - byte * 8));
                m_packetOutput->CalculateCryptoParameter(parameter, sizeof(parameter), packet);
            }
            setting.m_iv = parameter;
            setting.m_ivLength = sizeof(parameter);
            packet->Encrypt(setting);
        }
    }
    m_reader->Release();
    m_writer->Push();
}

}}}
