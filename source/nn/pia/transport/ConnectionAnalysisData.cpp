#include "nn/pia/transport/ConnectionAnalysisData.hpp"

namespace nn { namespace pia { namespace transport {

void ConnectionAnalysisData::Clear()
{
    for (unsigned int i = 0; i < cStationNumMax; ++i)
    {
        Entry& entry = m_entries[i];
        entry.m_rtt = 0;
        entry.m_rttMin = 0;
        entry.m_rttMax = 0;
        entry.m_prevSendPacketNum = 0;
        entry.m_sendPacketNum = 0;
        entry.m_prevLostPacketNum = 0;
        entry.m_lostPacketNum = 0;
        entry.m_isValid = false;
    }
    m_passedMilliSecond = 0;
}

}}}
