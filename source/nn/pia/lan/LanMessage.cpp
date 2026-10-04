#include "nn/pia/lan/LanMessage.hpp"

#include <coreinit/memory.h>

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cSerializedSize = 12
};
}

LanMessage::LanMessage()
    : m_messageType(0)
{
    OSBlockSet(m_payload, 0, sizeof(m_payload));
}

Result LanMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;

    buffer[0] = static_cast<unsigned char>(m_messageType);
    OSBlockMove(buffer + 1, m_payload, sizeof(m_payload), FALSE);
    *size = cSerializedSize;
    return 0;
}

Result LanMessage::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return cResultInvalidArgument;

    m_messageType = buffer[0];
    OSBlockMove(m_payload, buffer + 1, sizeof(m_payload), FALSE);
    return 0;
}

unsigned int LanMessage::GetSerializedSize() const
{
    return cSerializedSize;
}

void LanMessage::Trace(unsigned long long)
{
}

}}}
