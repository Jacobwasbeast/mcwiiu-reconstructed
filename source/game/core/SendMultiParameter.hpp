#ifndef GAME_CORE_SENDMULTIPARAMETER_HPP
#define GAME_CORE_SENDMULTIPARAMETER_HPP

#include <nex.h>

struct sockaddr_in
{
    unsigned short sin_family;
    unsigned short sin_port;
    unsigned int sin_addr;
    unsigned char sin_zero[8];
};

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
