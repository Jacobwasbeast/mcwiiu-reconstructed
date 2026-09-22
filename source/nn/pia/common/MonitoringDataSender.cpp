#include "nn/pia/common/MonitoringDataSender.hpp"

namespace nn { namespace pia { namespace common {

MonitoringDataSender::~MonitoringDataSender()
{
}

void MonitoringDataSender::RequestSendReport(DataType dataType)
{
    RequestSendReportImpl(dataType);
}

}}}
