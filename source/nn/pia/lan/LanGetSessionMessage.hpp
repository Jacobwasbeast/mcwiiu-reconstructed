#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_GET_SESSION_MESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_GET_SESSION_MESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"

namespace nn { namespace pia { namespace lan {

enum LanGetSessionMessageKind { cLanGetSessionMessageKind = 5 };

class LanGetSessionMessage : public LanMessage
{
public:
    LanGetSessionMessage();
    explicit LanGetSessionMessage(unsigned int sessionId);
    virtual ~LanGetSessionMessage();
    void Serialize(unsigned char* buffer, unsigned int* size) const;
    void Deserialize(const unsigned char* buffer);
    unsigned int GetSerializedSize() const;

private:
    LanGetSessionMessageKind messageType;
    unsigned int sessionId;
};

}}}

#endif
