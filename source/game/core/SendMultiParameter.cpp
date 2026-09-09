#include "game/core/SendMultiParameter.hpp"

#include <string.h>

SendMultiParameter::SendMultiParameter()
    : m_lock(0x20000000)
{
    m_uiPeerAddressCount = 0;
    memset(m_peerAddresses, 0, sizeof(m_peerAddresses));
    m_uiSendFlags = 0;
}
