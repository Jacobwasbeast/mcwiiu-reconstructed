#include "nn/pia/common/SocketAddress.hpp"

#include <coreinit/memory.h>

#include "nn/pia/String.hpp"

extern "C"
{
int getaddrinfo(const char* host, const char* service, const nn::pia::common::AddrInfo* hints,
                nn::pia::common::AddrInfo** result);
void freeaddrinfo(nn::pia::common::AddrInfo* info);
const char* gai_strerror(int error);
}

namespace nn { namespace pia { namespace common {

namespace
{
enum
{
    cFamilyInet = 2,
    cAddressOffset = 4,
    cPortOffset = 2,
    cFormattedLength = 32
};
}

void SocketAddress::Init()
{
    m_pointer = &m_address;
    OSBlockSet(&m_address, 0, sizeof(m_address));
    *reinterpret_cast<unsigned short*>(m_pointer->bytes) = cFamilyInet;
}

void SocketAddress::SetInetAddress(const InetAddress& address)
{
    *reinterpret_cast<unsigned int*>(m_pointer->bytes + cAddressOffset) = address.m_address;
    *reinterpret_cast<unsigned short*>(m_pointer->bytes + cPortOffset) = address.m_port;
}

void SocketAddress::GetInetAddress(InetAddress* address)
{
    if (address == 0)
        return;
    address->m_address = *reinterpret_cast<const unsigned int*>(m_pointer->bytes + cAddressOffset);
    address->m_port = *reinterpret_cast<const unsigned short*>(m_pointer->bytes + cPortOffset);
}

void SocketAddress::SetSockAddrIn(const SockAddrIn& address)
{
    *m_pointer = address;
}

SockAddrIn SocketAddress::GetSockAddrIn()
{
    return *m_pointer;
}

bool SocketAddress::GetAddressInfo(const char* host, const char* service, const AddrInfo* hints)
{
    bool success = false;
    AddrInfo* result;
    int error;
    if (hints != 0)
    {
        error = getaddrinfo(host, service, hints, &result);
    }
    else
    {
        AddrInfo defaultHints;
        OSBlockSet(&defaultHints, 0, sizeof(defaultHints));
        defaultHints.family = cFamilyInet;
        error = getaddrinfo(host, service, &defaultHints, &result);
    }
    if (error == 0)
    {
        *reinterpret_cast<unsigned int*>(m_pointer->bytes + cAddressOffset) =
            *reinterpret_cast<const unsigned int*>(result->address->bytes + cAddressOffset);
        freeaddrinfo(result);
        success = true;
    }
    else
    {
        gai_strerror(error);
    }
    return success;
}

void SocketAddress::Trace(unsigned long long value) const
{
    FixedString<cFormattedLength> text;
    const unsigned int address = *reinterpret_cast<const unsigned int*>(m_pointer->bytes + cAddressOffset);
    text.Format("%d.%d.%d.%d:%d", address >> 24, (address >> 16) & 0xff, (address >> 8) & 0xff,
                address & 0xff, *reinterpret_cast<const unsigned short*>(m_pointer->bytes + cPortOffset));
}

const SockAddrIn* SocketAddress::GetSockAddrInPointer() const
{
    return m_pointer;
}

}}}
