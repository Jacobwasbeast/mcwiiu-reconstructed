#include "nn/pia/transport/SendThreadStream.hpp"
#include "nn/pia/common/SessionStateMonitoringContent.hpp"

namespace nn { namespace pia { namespace transport {

void SendThreadStream::SetMonitoringData()
{
    common::g_sessionStateMonitoringContent.m_sendThreadSleepMilliSecond = 0xff;
    if (static_cast<unsigned int>(m_sleepMilliSecond) - 1u < 0xfe)
        common::g_sessionStateMonitoringContent.m_sendThreadSleepMilliSecond = static_cast<unsigned char>(m_sleepMilliSecond);
}

}}}
