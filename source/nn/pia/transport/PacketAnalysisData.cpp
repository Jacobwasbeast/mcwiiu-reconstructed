#include "nn/pia/transport/PacketAnalysisData.hpp"

namespace nn { namespace pia { namespace transport {

PacketAnalysisData::Entry::Entry()
    : m_protocolId(g_piaProtocolDefaultType)
{
}

void PacketAnalysisData::ClearExceptName()
{
    for (int i = 0; i < cEntryNumMax; ++i)
    {
        Entry& entry = m_entries[i];
        entry.m_protocolId = ProtocolId(g_piaProtocolDefaultType);
        entry.m_packetNum = 0;
        entry.m_packetSize = 0;
        entry.m_totalPacketNum = 0;
        entry.m_totalPacketSize = 0;
    }
    m_entryNum = 0;
    m_passedMilliSecond = 0;
    m_packetNum = 0;
    m_packetSize = 0;
    m_totalPacketNum = 0;
    m_totalPacketSize = 0;
}

void PacketAnalysisData::ClearCounters()
{
    for (int i = 0; i < m_entryNum; ++i)
    {
        Entry& entry = m_entries[i];
        entry.m_packetNum = 0;
        entry.m_packetSize = 0;
    }
    m_passedMilliSecond = 0;
    m_packetNum = 0;
    m_packetSize = 0;
}

int PacketAnalysisData::GetIndex(ProtocolId protocolId) const
{
    const unsigned short value = static_cast<unsigned short>(protocolId);
    for (int i = 0; i < m_entryNum; ++i)
    {
        if (static_cast<unsigned short>(m_entries[i].m_protocolId) == value)
            return i;
    }
    return -1;
}

int PacketAnalysisData::CreateIndex(ProtocolId protocolId)
{
    if (m_entryNum >= cEntryNumMax)
        return -1;
    const int index = m_entryNum;
    m_entries[index].m_protocolId = protocolId;
    m_entryNum = index + 1;
    return index;
}

}}}
