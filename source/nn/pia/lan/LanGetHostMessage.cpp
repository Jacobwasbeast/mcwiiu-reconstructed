#include "nn/pia/lan/LanGetHostMessage.hpp"
#include <cstring>

namespace nn { namespace pia { namespace lan {

LanGetHostMessage::LanGetHostMessage() : LanMessage(), messageType(cLanGetHostMessageKind), sessionId(0) {}
LanGetHostMessage::LanGetHostMessage(unsigned int value) : LanMessage(), messageType(cLanGetHostMessageKind), sessionId(value) {}
LanGetHostMessage::~LanGetHostMessage() {}

void LanGetHostMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    if (buffer != 0)
    {
        std::memset(buffer, 0, GetSerializedSize());
        buffer[3] = static_cast<unsigned char>(messageType);
        buffer[12] = static_cast<unsigned char>(sessionId >> 24);
        buffer[13] = static_cast<unsigned char>(sessionId >> 16);
        buffer[14] = static_cast<unsigned char>(sessionId >> 8);
        buffer[15] = static_cast<unsigned char>(sessionId);
    }
    if (size != 0)
        *size = GetSerializedSize();
}

void LanGetHostMessage::Deserialize(const unsigned char* buffer)
{
    if (buffer != 0)
        sessionId = (static_cast<unsigned int>(buffer[12]) << 24) |
                    (static_cast<unsigned int>(buffer[13]) << 16) |
                    (static_cast<unsigned int>(buffer[14]) << 8) |
                    static_cast<unsigned int>(buffer[15]);
}

unsigned int LanGetHostMessage::GetSerializedSize() const { return 16; }

}}}
