#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_GET_HOST_MESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_GET_HOST_MESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"

namespace nn { namespace pia { namespace lan {

enum LanGetHostMessageKind { cLanGetHostMessageKind = 3 };

class LanGetHostMessage : public LanMessage
{
public:
    LanGetHostMessage();
    explicit LanGetHostMessage(unsigned int sessionId);
    virtual ~LanGetHostMessage();
    void Serialize(unsigned char* buffer, unsigned int* size) const;
    void Deserialize(const unsigned char* buffer);
    unsigned int GetSerializedSize() const;

private:
    LanGetHostMessageKind messageType;
    unsigned int sessionId;
};

}}}

#endif
