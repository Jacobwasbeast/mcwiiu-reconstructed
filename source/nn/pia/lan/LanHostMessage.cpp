#include "nn/pia/lan/LanHostMessage.hpp"

namespace nn { namespace pia { namespace lan {

namespace
{
enum
{
    cMessageType = 4,
    cHeaderSize = 16
};
}

LanHostMessage::LanHostMessage()
    : m_sessionId(0), m_connectionInfo()
{
    m_messageType = cMessageType;
}

LanHostMessage::LanHostMessage(unsigned int sessionId,
                               const transport::StationConnectionInfo& connectionInfo)
    : m_sessionId(sessionId), m_connectionInfo(connectionInfo)
{
    m_messageType = cMessageType;
}

Result LanHostMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    Result result = LanMessage::Serialize(buffer, size);
    if (result != 0)
        return result;

    unsigned char* out = buffer + *size;
    out[0] = (m_sessionId >> 24) & 0xff;
    out[1] = (m_sessionId >> 16) & 0xff;
    out[2] = (m_sessionId >> 8) & 0xff;
    out[3] = m_sessionId & 0xff;
    unsigned int connectionInfoSize = 0;
    result = m_connectionInfo.Serialize(out + 4, &connectionInfoSize,
                                        m_connectionInfo.GetSerializedSize());
    if (result != 0)
        return result;

    *size = out + 4 + connectionInfoSize - buffer;
    return 0;
}

Result LanHostMessage::Deserialize(const unsigned char* buffer)
{
    Result result = LanMessage::Deserialize(buffer);
    if (result != 0)
        return result;

    m_sessionId = (static_cast<unsigned int>(buffer[12]) << 24) |
                  (static_cast<unsigned int>(buffer[13]) << 16) |
                  (static_cast<unsigned int>(buffer[14]) << 8) |
                  static_cast<unsigned int>(buffer[15]);
    return m_connectionInfo.Deserialize(buffer + cHeaderSize);
}

unsigned int LanHostMessage::GetSerializedSize() const
{
    return m_connectionInfo.GetSerializedSize() + cHeaderSize;
}

}}}
