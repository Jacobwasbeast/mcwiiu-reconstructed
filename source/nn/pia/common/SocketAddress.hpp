#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_ADDRESS_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_COMMON_SOCKET_ADDRESS_HPP

#include "nn/pia/Common.hpp"
#include "nn/pia/common/InetAddress.hpp"

namespace nn { namespace pia { namespace common {

struct SockAddrIn { unsigned char bytes[16]; };
struct AddrInfo { unsigned char bytes[32]; };

class SocketAddress
{
public:
    void Init();
    void SetInetAddress(const InetAddress& address);
    void GetInetAddress(InetAddress* address);
    void SetSockAddrIn(const SockAddrIn& address);
    SockAddrIn GetSockAddrIn();
    bool GetAddressInfo(const char* host, const char* service, const AddrInfo* hints);
    void Trace(unsigned long long value) const;
    const SockAddrIn* GetSockAddrInPointer() const;
    void GetHostByName(const char* host);

private:
    SockAddrIn m_address;
};

}}}

#endif
