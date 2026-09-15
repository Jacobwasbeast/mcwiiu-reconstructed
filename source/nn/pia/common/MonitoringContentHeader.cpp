#include "nn/pia/common/MonitoringContentHeader.hpp"

namespace nn { namespace pia { namespace common {

unsigned int MonitoringContentHeader::GetSerializedSize()
{
    return 16;
}

Result MonitoringContentHeader::Serialize(unsigned char* buffer, unsigned int* size,
                                         unsigned int maxSize) const
{
    if (buffer == 0 || size == 0 || maxSize < GetSerializedSize())
        return 0x10c07;

    buffer[0] = m_contentType;
    buffer[1] = m_contentVersion;
    buffer[2] = m_attribute;
    buffer[3] = m_status;
    serializeU16(buffer + 4, m_contentSize);
    for (unsigned int index = 0; index < 10; ++index)
        buffer[6 + index] = m_metadata[index];

    *size = 16;
    return 0;
}

}}}
