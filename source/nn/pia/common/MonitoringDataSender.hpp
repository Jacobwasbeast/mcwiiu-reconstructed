#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_SENDER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_SENDER_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace common {

class MonitoringDataSender
{
public:
    enum DataType
    {
        cDataType0 = 0,
        cDataType1 = 1,
        cDataType2 = 2
    };

    ~MonitoringDataSender();

    void RequestSendReport(DataType dataType);
    static Result GetMonitoringServerURL(char* buffer, int size);
};

}}}

#endif
