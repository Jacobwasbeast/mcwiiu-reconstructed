#include "nn/pia/transport/ProtocolMessageAccessor.hpp"

#include <string.h>

namespace nn { namespace pia { namespace transport {

ProtocolMessageAccessor::Header::Header()
    : m_flags(0), m_messageSize(0), m_destination(0), m_source(0), m_protocolId(0)
{
    m_field1a[0] = 0;
    m_field1a[1] = 0;
    m_field1a[2] = 0;
}

ProtocolMessageAccessor::Header::~Header()
{
}

Result ProtocolMessageAccessor::Header::Serialize(unsigned char* buffer,
                                                  unsigned int* size) const
{
    if (buffer == 0)
        return static_cast<Result>(0x10c07);
    if (size == 0)
        return static_cast<Result>(0x10c07);

    const unsigned int destinationHigh = static_cast<unsigned int>(m_destination >> 32);
    const unsigned int destinationLow = static_cast<unsigned int>(m_destination);
    const unsigned int sourceHigh = static_cast<unsigned int>(m_source >> 32);
    const unsigned int sourceLow = static_cast<unsigned int>(m_source);

    buffer[0] = m_flags;
    buffer[1] = static_cast<unsigned char>(m_messageSize >> 8);
    buffer[2] = static_cast<unsigned char>(m_messageSize);
    buffer[3] = static_cast<unsigned char>(destinationHigh >> 24);
    buffer[4] = static_cast<unsigned char>(destinationHigh >> 16);
    buffer[5] = static_cast<unsigned char>(destinationHigh >> 8);
    buffer[6] = static_cast<unsigned char>(destinationHigh);
    buffer[7] = static_cast<unsigned char>(destinationLow >> 24);
    buffer[8] = static_cast<unsigned char>(destinationLow >> 16);
    buffer[9] = static_cast<unsigned char>(destinationLow >> 8);
    buffer[10] = static_cast<unsigned char>(destinationLow);
    buffer[11] = static_cast<unsigned char>(sourceHigh >> 24);
    buffer[12] = static_cast<unsigned char>(sourceHigh >> 16);
    buffer[13] = static_cast<unsigned char>(sourceHigh >> 8);
    buffer[14] = static_cast<unsigned char>(sourceHigh);
    buffer[15] = static_cast<unsigned char>(sourceLow >> 24);
    buffer[16] = static_cast<unsigned char>(sourceLow >> 16);
    buffer[17] = static_cast<unsigned char>(sourceLow >> 8);
    buffer[18] = static_cast<unsigned char>(sourceLow);
    buffer[19] = static_cast<unsigned char>(m_protocolId.value() >> 8);
    buffer[20] = static_cast<unsigned char>(m_protocolId.value());
    memcpy(buffer + 21, m_field1a, sizeof(m_field1a));

    *size = cSerializedSize;
    return 0;
}

Result ProtocolMessageAccessor::Header::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return static_cast<Result>(0x10c07);

    m_flags = buffer[0];
    m_messageSize = static_cast<unsigned short>(
        (static_cast<unsigned int>(buffer[1]) << 8) | buffer[2]);
    const unsigned int destinationHigh =
        (static_cast<unsigned int>(buffer[3]) << 24) |
        (static_cast<unsigned int>(buffer[4]) << 16) |
        (static_cast<unsigned int>(buffer[5]) << 8) | buffer[6];
    const unsigned int destinationLow =
        (static_cast<unsigned int>(buffer[7]) << 24) |
        (static_cast<unsigned int>(buffer[8]) << 16) |
        (static_cast<unsigned int>(buffer[9]) << 8) | buffer[10];
    m_destination = (static_cast<unsigned long long>(destinationHigh) << 32) | destinationLow;
    const unsigned int sourceHigh =
        (static_cast<unsigned int>(buffer[11]) << 24) |
        (static_cast<unsigned int>(buffer[12]) << 16) |
        (static_cast<unsigned int>(buffer[13]) << 8) | buffer[14];
    const unsigned int sourceLow =
        (static_cast<unsigned int>(buffer[15]) << 24) |
        (static_cast<unsigned int>(buffer[16]) << 16) |
        (static_cast<unsigned int>(buffer[17]) << 8) | buffer[18];
    m_source = (static_cast<unsigned long long>(sourceHigh) << 32) | sourceLow;
    m_protocolId = ProtocolId(static_cast<unsigned short>(
        (static_cast<unsigned int>(buffer[19]) << 8) | buffer[20]));
    memcpy(m_field1a, buffer + 21, sizeof(m_field1a));

    return 0;
}

unsigned int ProtocolMessageAccessor::Header::GetSerializedSize() const
{
    return cSerializedSize;
}

}}}
