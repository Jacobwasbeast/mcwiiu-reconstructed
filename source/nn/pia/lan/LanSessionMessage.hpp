#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSESSIONMESSAGE_HPP

#include <cstdint>

#include "nn/pia/lan/LanMessage.hpp"

namespace nn { namespace pia { namespace lan {

class LanSessionMessage : public LanMessage
{
public:
    LanSessionMessage();
    LanSessionMessage(unsigned int sessionId, unsigned short port, unsigned char type,
                      unsigned char flags, const unsigned char* payload, unsigned int payloadSize);
    virtual ~LanSessionMessage();
    void Serialize(unsigned char* buffer, unsigned int* size) const;
    void Deserialize(const unsigned char* buffer);
    unsigned int GetSerializedSize() const;

private:
    unsigned int mSessionId;
    unsigned short mPort;
    unsigned char mType;
    unsigned char mFlags;
    unsigned char mPayload[800];
    unsigned int mPayloadSize;
};

}}}

#endif
