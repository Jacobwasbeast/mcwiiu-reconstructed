#include "nn/pia/common/MonitoringDataSender.hpp"

namespace nn { namespace pia { namespace common {

MonitoringDataSender::~MonitoringDataSender()
{
}

void MonitoringDataSender::RequestSendReport(DataType dataType)
{
    typedef void (*RequestSendReportFunction)(MonitoringDataSender*, unsigned int);
    const unsigned int* vtable = *reinterpret_cast<const unsigned int* const*>(this);
    RequestSendReportFunction request =
        reinterpret_cast<RequestSendReportFunction>(vtable[0x44 / sizeof(unsigned int)]);
    request(this, static_cast<unsigned int>(dataType));
}

}}}
