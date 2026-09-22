#include "nn/pia/inet/MonitoringDataSendJob.hpp"

namespace nn { namespace pia { namespace inet {

MonitoringDataSendJob::MonitoringDataSendJob() {}
MonitoringDataSendJob::~MonitoringDataSendJob() {}
void MonitoringDataSendJob::Cleanup() { m_callContext.Reset(); }
void MonitoringDataSendJob::Trace(unsigned long long) const {}

}}}
