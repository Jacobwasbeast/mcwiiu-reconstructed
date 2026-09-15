#include "nn/pia/common/MonitoringDataHeader.hpp"

namespace nn { namespace pia { namespace common {

unsigned int MonitoringDataHeader::GetSerializedSize()
{
    return 16;
}

Result MonitoringDataHeader::Serialize(unsigned char* buffer, unsigned int* size,
                                        unsigned int maxSize) const
{
    if (buffer == 0 || size == 0 || maxSize < GetSerializedSize())
        return 0x10c07;

    for (unsigned int index = 0; index < 4; ++index)
        buffer[index] = m_prefix[index];
    serializeU16(buffer + 4, m_serializedValue);
    for (unsigned int index = 0; index < 10; ++index)
        buffer[6 + index] = m_payload[index];

    *size = GetSerializedSize();
    return 0;
}

}}}
