#include "nn/pia/common/SocketAddress.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

void SocketAddress::Init()
{
    memset(&m_address, 0, sizeof(m_address));
    m_address.bytes[1] = 2;
}

void SocketAddress::SetInetAddress(const InetAddress& address)
{
    memcpy(m_address.bytes + 4, &address.m_address, sizeof(address.m_address));
    memcpy(m_address.bytes + 2, &address.m_port, sizeof(address.m_port));
}

void SocketAddress::GetInetAddress(InetAddress* address)
{
    if (address == 0) return;
    unsigned int hostAddress = 0;
    unsigned short port = 0;
    memcpy(&hostAddress, m_address.bytes + 4, sizeof(hostAddress));
    memcpy(&port, m_address.bytes + 2, sizeof(port));
    *address = InetAddress(hostAddress, port);
}

void SocketAddress::SetSockAddrIn(const SockAddrIn& address)
{
    m_address = address;
}

SockAddrIn SocketAddress::GetSockAddrIn()
{
    return m_address;
}

bool SocketAddress::GetAddressInfo(const char* host, const char* service, const AddrInfo* hints)
{
    if (host == 0 || service == 0 || hints == 0)
        return false;
    return false;
}

void SocketAddress::Trace(unsigned long long value) const
{
    (void)value;
}

const SockAddrIn* SocketAddress::GetSockAddrInPointer() const
{
    return &m_address;
}

}}}
