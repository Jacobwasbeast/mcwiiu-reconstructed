#include "nn/pia/lan/LanSessionMessage.hpp"

#include <coreinit/memory.h>

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cMessageType = 6,
    cHeaderSize = 24
};
}

LanSessionMessage::LanSessionMessage()
    : m_fragmentSize(0)
{
    m_sessionId = 0;
    m_messageType = cMessageType;
    m_sequence = 0;
    m_count = 0;
    m_index = 0;
}

LanSessionMessage::LanSessionMessage(unsigned int sessionId, unsigned short sequence,
                                     unsigned char index, unsigned char count,
                                     const unsigned char* fragment, unsigned int fragmentSize)
{
    m_sessionId = sessionId;
    m_sequence = sequence;
    m_index = index;
    m_messageType = cMessageType;
    m_fragmentSize = fragmentSize;
    m_count = count;
    OSBlockMove(m_fragment, fragment, fragmentSize, FALSE);
}

Result LanSessionMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    Result result = LanMessage::Serialize(buffer, size);
    if (result != 0)
        return result;

    unsigned char* out = buffer + *size;
    out[0] = (m_sessionId >> 24) & 0xff;
    out[1] = (m_sessionId >> 16) & 0xff;
    out[2] = (m_sessionId >> 8) & 0xff;
    out[3] = m_sessionId & 0xff;
    out[4] = (m_sequence >> 8) & 0xff;
    out[5] = m_sequence & 0xff;
    out[6] = m_index;
    out[7] = m_count;
    out[8] = (m_fragmentSize >> 24) & 0xff;
    out[9] = (m_fragmentSize >> 16) & 0xff;
    out[10] = (m_fragmentSize >> 8) & 0xff;
    out[11] = m_fragmentSize & 0xff;
    OSBlockMove(out + 12, m_fragment, m_fragmentSize, FALSE);
    *size = out + 12 + m_fragmentSize - buffer;
    return 0;
}

Result LanSessionMessage::Deserialize(const unsigned char* buffer)
{
    Result result = LanMessage::Deserialize(buffer);
    if (result != 0)
        return result;

    m_sessionId = (static_cast<unsigned int>(buffer[12]) << 24) |
                  (static_cast<unsigned int>(buffer[13]) << 16) |
                  (static_cast<unsigned int>(buffer[14]) << 8) |
                  static_cast<unsigned int>(buffer[15]);
    m_sequence = static_cast<unsigned short>((buffer[16] << 8) | buffer[17]);
    m_index = buffer[18];
    m_count = buffer[19];
    m_fragmentSize = (static_cast<unsigned int>(buffer[20]) << 24) |
                     (static_cast<unsigned int>(buffer[21]) << 16) |
                     (static_cast<unsigned int>(buffer[22]) << 8) |
                     static_cast<unsigned int>(buffer[23]);
    OSBlockMove(m_fragment, buffer + cHeaderSize, m_fragmentSize, FALSE);
    return 0;
}

unsigned int LanSessionMessage::GetSerializedSize() const
{
    return m_fragmentSize + cHeaderSize;
}

}}}
