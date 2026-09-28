#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_LAN_HOST_MESSAGE_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_LAN_HOST_MESSAGE_HPP

#include "nn/pia/lan/LanMessage.hpp"
#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace transport { } namespace lan {

class LanHostMessage : public LanMessage
{
public:
    LanHostMessage();
    LanHostMessage(unsigned int sessionId, const transport::StationConnectionInfo& connectionInfo);
    virtual ~LanHostMessage();
    void Serialize(unsigned char* buffer, unsigned int* size) const;
    void Deserialize(const unsigned char* buffer);
    unsigned int GetSerializedSize() const;

private:
    unsigned int messageType;
    unsigned int sessionId;
    transport::StationConnectionInfo connectionInfo;
};

}}}

#endif
