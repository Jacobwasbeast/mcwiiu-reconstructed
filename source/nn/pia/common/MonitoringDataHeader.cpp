#include "nn/pia/common/MonitoringDataHeader.hpp"

namespace nn { namespace pia { namespace common {

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07
};
}

unsigned int MonitoringDataHeader::GetSerializedSize()
{
    return 16;
}

Result MonitoringDataHeader::Serialize(unsigned char* buffer, unsigned int* size,
                                        unsigned int maxSize) const
{
    const Result invalid = cResultInvalidArgument;
    if (buffer == 0)
        return invalid;
    if (size == 0)
        return invalid;
    if (maxSize < GetSerializedSize())
        return invalid;

    unsigned char* cursor = buffer;
    serializeU8(cursor, m_headerType);
    cursor += sizeof(m_headerType);
    serializeU8(cursor, m_dataType);
    cursor += sizeof(m_dataType);
    serializeU8(cursor, m_attribute);
    cursor += sizeof(m_attribute);
    serializeU8(cursor, m_reserved1);
    cursor += sizeof(m_reserved1);
    serializeU16(cursor, m_contentSize);
    cursor += sizeof(m_contentSize);
    for (int index = 0; index < cIdentifierSize; ++index)
    {
        serializeU8(cursor, m_identifier[index]);
        cursor += sizeof(m_identifier[index]);
    }
    serializeU8(cursor, m_keyIndex);
    cursor += sizeof(m_keyIndex);
    serializeU8(cursor, m_reserved2);
    cursor += sizeof(m_reserved2);

    *size = static_cast<unsigned int>(cursor - buffer);
    return 0;
}

}}}
