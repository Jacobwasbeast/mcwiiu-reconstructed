#include "nn/pia/transport/PacketAnalyzer.hpp"

#include <string.h>

#include "nn/pia/common/TimeSpan.hpp"
#include "nn/pia/Transport.hpp"
#include "nn/pia/common/Packet.hpp"
#include "nn/pia/transport/ProtocolMessageReader.hpp"

namespace nn { namespace pia { namespace transport {

static common::Time getTime()
{
    static common::Time cachedTime;
    cachedTime = g_piaTransport->m_time;
    return cachedTime;
}

PacketAnalyzer::PacketAnalyzer(const char* name, unsigned int headerSize)
    : PacketAnalysisData(), m_criticalSection(-1), m_startedUp(false),
      m_headerSize(headerSize)
{
    unsigned int length = 0;
    while (name[length] != '\0')
        ++length;
    if (length > cNameLengthMax - 1)
        length = cNameLengthMax - 1;
    memcpy(m_name, name, length);
    m_name[length] = '\0';
}

PacketAnalyzer::~PacketAnalyzer()
{
}

Result PacketAnalyzer::Startup()
{
    if (m_startedUp)
        return static_cast<Result>(0x10c08);

    m_criticalSection.Lock();
    ClearExceptName();
    m_criticalSection.Unlock();

    m_startTime = getTime();
    m_startedUp = true;
    return 0;
}

void PacketAnalyzer::Cleanup()
{
    if (!m_startedUp)
        return;
    m_startedUp = false;
}

Result PacketAnalyzer::UpdatePacketAnalysisData(
    const common::Packet& packet, bool isRelay, bool isSingleCount)
{
    if (!m_startedUp)
        return static_cast<Result>(0x10c08);
    if (packet.IsValid() == 0)
        return static_cast<Result>(0x10c07);

    m_criticalSection.Lock();
    const unsigned int count = (!isRelay || isSingleCount) ? 1 :
        static_cast<unsigned int>(packet.GetPacketNumInNetwork());
    unsigned int offset = 0;
    const unsigned int payloadSize = packet.m_size - common::Packet::cHeaderSize;
    while (offset < payloadSize - 0x24U)
    {
        ProtocolMessageReader reader;
        reader.Attach(packet, offset);
        if (reader.m_message == 0)
            break;

        const unsigned int messageSize = (reader.m_messageSize + 0x17U) & ~3U;
        const ProtocolId protocolId(reader.m_header.m_protocolId);
        int index = GetIndex(protocolId);
        if (index < 0)
            index = CreateIndex(protocolId);
        if (index >= 0)
        {
            Entry& entry = m_entries[index];
            const unsigned int bytes = count * (messageSize + 4U);
            entry.m_packetNum += count;
            entry.m_packetSize += bytes;
            entry.m_totalPacketNum += count;
            entry.m_totalPacketSize += bytes;
        }
        offset += messageSize + 4U;
    }

    const unsigned int packetBytes = count * (packet.m_size + m_headerSize);
    m_packetNum += count;
    m_packetSize += packetBytes;
    m_totalPacketNum += count;
    m_totalPacketSize += packetBytes;
    m_criticalSection.Unlock();
    return 0;
}

void PacketAnalyzer::ClearPacketAnalysisData()
{
    m_criticalSection.Lock();
    ClearCounters();
    m_criticalSection.Unlock();
}

Result PacketAnalyzer::GetPacketAnalysisData(
    PacketAnalysisData* packetAnalysisData) const
{
    if (packetAnalysisData == 0)
        return static_cast<Result>(0x10c07);

    m_criticalSection.Lock();
    *packetAnalysisData = *this;
    m_criticalSection.Unlock();

    const common::Time now = getTime();
    packetAnalysisData->m_passedMilliSecond = static_cast<unsigned int>(
        (now.m_tick - m_startTime.m_tick) /
        static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick));
    return 0;
}

}}}
