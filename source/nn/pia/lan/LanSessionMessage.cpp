#include "nn/pia/lan/LanSessionMessage.hpp"

#include <cstring>

namespace nn { namespace pia { namespace lan {

namespace {
void put32(unsigned char* p, unsigned int value)
{
    p[0] = static_cast<unsigned char>(value >> 24);
    p[1] = static_cast<unsigned char>(value >> 16);
    p[2] = static_cast<unsigned char>(value >> 8);
    p[3] = static_cast<unsigned char>(value);
}

unsigned int get32(const unsigned char* p)
{
    return (static_cast<unsigned int>(p[0]) << 24)
         | (static_cast<unsigned int>(p[1]) << 16)
         | (static_cast<unsigned int>(p[2]) << 8)
         | static_cast<unsigned int>(p[3]);
}
}

LanSessionMessage::LanSessionMessage()
    : LanMessage(), mSessionId(0), mPort(0), mType(0), mFlags(0), mPayloadSize(0)
{
}

LanSessionMessage::LanSessionMessage(unsigned int sessionId, unsigned short port,
                                     unsigned char type, unsigned char flags,
                                     const unsigned char* payload, unsigned int payloadSize)
    : LanMessage(), mSessionId(sessionId), mPort(port), mType(type), mFlags(flags),
      mPayloadSize(payloadSize)
{
    if (payload != NULL && mPayloadSize != 0)
        std::memcpy(mPayload, payload, mPayloadSize);
}

LanSessionMessage::~LanSessionMessage()
{
}

void LanSessionMessage::Serialize(unsigned char* buffer, unsigned int* size) const
{
    const unsigned int baseSize = 12;
    std::memset(buffer, 0, baseSize);
    unsigned char* out = buffer + baseSize;
    put32(out, mSessionId);
    out[4] = static_cast<unsigned char>(mPort >> 8);
    out[5] = static_cast<unsigned char>(mPort);
    out[6] = mType;
    out[7] = mFlags;
    put32(out + 8, mPayloadSize);
    std::memcpy(out + 12, mPayload, mPayloadSize);
    *size = baseSize + 12 + mPayloadSize;
}

void LanSessionMessage::Deserialize(const unsigned char* buffer)
{
    const unsigned char* in = buffer + 12;
    mSessionId = get32(in);
    mPort = static_cast<unsigned short>(in[4] << 8 | in[5]);
    mType = in[6];
    mFlags = in[7];
    mPayloadSize = get32(in + 8);
    std::memcpy(mPayload, in + 12, mPayloadSize);
}

unsigned int LanSessionMessage::GetSerializedSize() const
{
    return 24 + mPayloadSize;
}

}}}
