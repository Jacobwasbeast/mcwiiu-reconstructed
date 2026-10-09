#ifndef GAME_CORE_SENDMULTIPARAMETER_HPP
#define GAME_CORE_SENDMULTIPARAMETER_HPP

#include <netinet/in.h>

#include <nex.h>

class SendMultiParameter : public nn::nex::RootObject
{
public:
    enum
    {
        PeerAddressMax = 15
    };

    SendMultiParameter();

    nn::nex::ForcedCriticalSection m_lock;
    unsigned int m_uiPeerAddressCount;
    sockaddr_in m_peerAddresses[PeerAddressMax];
    unsigned int m_uiSendFlags;
};

#endif
