#include "nn/pia/transport/ResendingMessageManager.hpp"

namespace nn { namespace pia { namespace transport {

Result ResendingMessageManager::Startup(PacketHandler* packetHandler, bool isRelay)
{
    if (packetHandler == 0)
        return static_cast<Result>(0x10c07);

    m_packetHandler = packetHandler;
    m_isRelay = isRelay;
    return 0;
}

}}}
