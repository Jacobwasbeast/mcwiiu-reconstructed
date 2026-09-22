#include "nn/pia/lan/LanMessage.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanMessage::LanMessage()
    : m_messageType(0), m_payload{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
{
}

LanMessage::~LanMessage()
{
}

void LanMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer == 0)
    {
        if (size != 0)
            *size = 0x10c07;
        return;
    }
    if (size == 0)
        return;
    buffer[0] = static_cast<unsigned char>(m_messageType >> 24);
    std::memcpy(buffer + 1, m_payload, sizeof(m_payload));
    *size = 12;
}

void LanMessage::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return;
    m_messageType = buffer[0];
    std::memcpy(m_payload, buffer + 1, sizeof(m_payload));
}

void LanMessage::Trace(unsigned long long)
{
}

unsigned int LanMessage::GetSerializedSize() const
{
    return 12;
}

}}}
