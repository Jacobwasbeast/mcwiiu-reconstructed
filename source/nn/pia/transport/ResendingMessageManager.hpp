#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RESENDING_MESSAGE_MANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_TRANSPORT_RESENDING_MESSAGE_MANAGER_HPP

#include "nn/pia.h"

namespace nn { namespace pia { namespace transport {

class PacketHandler;

class ResendingMessageManager
{
public:
    Result Startup(PacketHandler* packetHandler, bool isRelay);

    PacketHandler* m_packetHandler;
    bool m_isRelay;
};

}}}

#endif
