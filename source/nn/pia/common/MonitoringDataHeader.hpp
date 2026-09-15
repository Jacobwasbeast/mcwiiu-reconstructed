#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_HEADER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_DATA_HEADER_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace common {

class MonitoringDataHeader
{
public:
    static unsigned int GetSerializedSize();
    Result Serialize(unsigned char* buffer, unsigned int* size,
                     unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(unsigned long long value) const;

    unsigned char m_prefix[4];
    unsigned short m_serializedValue;
    unsigned char m_payload[10];
};

}}}

#endif
