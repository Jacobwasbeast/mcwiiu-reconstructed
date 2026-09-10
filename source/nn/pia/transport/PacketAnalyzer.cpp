#include "nn/pia/transport/PacketAnalyzer.hpp"

#include <string.h>

#include "nn/pia/common/TimeSpan.hpp"
#include "vendor/__N_28_transport_PacketAnalyzer_cpp_ff43370f.hpp"

namespace nn { namespace pia { namespace transport {

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

    m_startTime = common::Time::Create(
        __N_28_transport_PacketAnalyzer_cpp_ff43370f::getTime());
    m_startedUp = true;
    return 0;
}

void PacketAnalyzer::Cleanup()
{
    if (!m_startedUp)
        return;
    m_startedUp = false;
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

    const common::Time now = common::Time::Create(
        __N_28_transport_PacketAnalyzer_cpp_ff43370f::getTime());
    packetAnalysisData->m_passedMilliSecond = static_cast<unsigned int>(
        (now.m_tick - m_startTime.m_tick) /
        static_cast<unsigned long long>(common::TimeSpan::GetTicksPerMSec().m_tick));
    return 0;
}

}}}
