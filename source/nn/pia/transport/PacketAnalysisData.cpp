#include "nn/pia/transport/PacketAnalysisData.hpp"
#include "nn/pia/transport/AnalysisPrinter.hpp"
#include "vendor/__N_32_transport_PacketAnalysisData_cpp_3279541b.hpp"

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

void PacketAnalysisData::Print(bool isDetail) const
{
    const int elapsed = static_cast<int>(m_passedMilliSecond);
    AnalysisPrinter::Write("[Analysis] ------ BEGIN(%s, %d.%03d sec. passed) ------",
                           m_name, elapsed / 1000, elapsed % 1000);
    AnalysisPrinter::Write("[Analysis] ProtocolId, TotalNum, TotalSize, AverageSize, Protocol(port)");
    const unsigned int average = m_packetNum == 0 ? 0 : m_packetSize / m_packetNum;
    AnalysisPrinter::Write("[Analysis]     0xffff, %8d,  %8d,    %8d, Packet",
                           static_cast<int>(m_packetNum), static_cast<int>(m_packetSize),
                           static_cast<int>(average));
    for (int i = 0; i < m_entryNum; ++i)
    {
        const Entry& entry = m_entries[i];
        if (!isDetail && __N_32_transport_PacketAnalysisData_cpp_3279541b::isInternalProtocol(entry.m_protocolId))
            continue;
        const unsigned int entryAverage = entry.m_packetNum == 0 ? ~0u : entry.m_packetSize / entry.m_packetNum;
        AnalysisPrinter::Write("[Analysis]     0x%04x, %8d,  %8d,    %8d, %s(%d)",
                               static_cast<unsigned int>(entry.m_protocolId),
                               static_cast<int>(entry.m_packetNum), static_cast<int>(entry.m_packetSize),
                               static_cast<int>(entryAverage),
                               __N_32_transport_PacketAnalysisData_cpp_3279541b::getProtocolName(entry.m_protocolId),
                               static_cast<int>(static_cast<unsigned short>(entry.m_protocolId) & 0xff));
    }
    AnalysisPrinter::Write("[Analysis] ---------------------------- END ----------------------------");
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
