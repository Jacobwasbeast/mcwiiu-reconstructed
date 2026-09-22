#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSELFKEEPALIVEMESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_LANSELFKEEPALIVEMESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"

namespace nn { namespace pia { namespace lan {

class LanSelfKeepAliveMessage : public LanMessage
{
public:
    LanSelfKeepAliveMessage();
    virtual ~LanSelfKeepAliveMessage();
    void Serialize(unsigned char* buffer, unsigned int* size) const;
    void Deserialize(const unsigned char* buffer);
    unsigned int GetSerializedSize() const;
};

}}}

#endif
