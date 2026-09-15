#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_CONTENT_HEADER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_MONITORING_CONTENT_HEADER_HPP

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace common {

class MonitoringContentHeader
{
public:
    static unsigned int GetSerializedSize();
    Result Serialize(unsigned char* buffer, unsigned int* size, unsigned int maxSize) const;
    Result Deserialize(const unsigned char* buffer, unsigned int size);
    void Trace(unsigned long long value) const;

    unsigned char m_contentType;
    unsigned char m_contentVersion;
    unsigned char m_attribute;
    unsigned char m_status;
    unsigned short m_contentSize;
    unsigned char m_metadata[10];
};

}}}

#endif
