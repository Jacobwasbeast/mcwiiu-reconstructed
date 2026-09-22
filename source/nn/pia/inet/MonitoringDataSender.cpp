#include "nn/pia/inet/MonitoringDataSender.hpp"

namespace nn { namespace pia { namespace inet {

MonitoringDataSender::MonitoringDataSender()
    : m_sendSessionBeginReportRequested(false)
{
}

MonitoringDataSender::~MonitoringDataSender()
{
}

void MonitoringDataSender::SetSendSessionBeginReportRequested(bool requested)
{
    m_sendSessionBeginReportRequested = requested;
}

unsigned char MonitoringDataSender::IsSendSessionBeginReportRequested() const
{
    return m_sendSessionBeginReportRequested ? 1 : 0;
}

void MonitoringDataSender::GetIv(unsigned char* destination, unsigned char* source) const
{
    for (unsigned int index = 0; index < 8; ++index)
        destination[index] = source[index];
    for (unsigned int index = 8; index < 12; ++index)
        destination[index] = 0;
}

void MonitoringDataSender::Trace(unsigned long long value) const
{
    (void)value;
}

unsigned int MonitoringDataSender::IsEnableMonitoring() const
{
    return 1;
}

void MonitoringDataSender::RequestSendReportImpl(DataType dataType)
{
    (void)dataType;
}

}}}
