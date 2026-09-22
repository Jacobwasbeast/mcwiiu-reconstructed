#include "nn/pia/lan/LanSelfKeepAliveMessage.hpp"

namespace nn { namespace pia { namespace lan {

LanSelfKeepAliveMessage::LanSelfKeepAliveMessage()
    : LanMessage()
{
    reinterpret_cast<unsigned int*>(this)[1] = 7;
}

LanSelfKeepAliveMessage::~LanSelfKeepAliveMessage()
{
}

void LanSelfKeepAliveMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    unsigned int serializedSize = 0;
    LanMessage::Serialize(buffer, &serializedSize);
    if (size != 0)
        *size = serializedSize;
}

void LanSelfKeepAliveMessage::Deserialize(const unsigned char* buffer)
{
    LanMessage::Deserialize(buffer);
}

unsigned int LanSelfKeepAliveMessage::GetSerializedSize() const
{
    return 12;
}

}}}
