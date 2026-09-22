#include "nn/pia/common/Socket.hpp"

#include <string.h>

namespace nn { namespace pia { namespace common {

Socket::Socket() : m_handle(-1), m_family(0), m_address(), m_ttl(0)
{
}

Socket::~Socket()
{
    Close();
}

Result Socket::Open(int family, int type)
{
    m_family = family;
    m_handle = type;
    return 0;
}

void Socket::Open()
{
    Open(2, 2);
}

Result Socket::Close()
{
    m_handle = -1;
    return 0;
}

Result Socket::SetReuseAddress(bool enabled)
{
    (void)enabled;
    return 0;
}

Result Socket::Bind(const InetAddress& address, bool reuseAddress)
{
    (void)reuseAddress;
    m_address = address;
    return 0;
}

Result Socket::Bind(unsigned short port, bool reuseAddress)
{
    InetAddress address(0, port);
    return Bind(address, reuseAddress);
}

Result Socket::RecvFrom(unsigned char* buffer, int size, InetAddress* address, unsigned char* ancillary, int* ancillarySize)
{
    (void)buffer; (void)size; (void)address; (void)ancillary; (void)ancillarySize;
    return 0;
}

Result Socket::SendTo(const void* buffer, int size, const InetAddress& address, int* sentSize)
{
    (void)buffer; (void)address;
    if (sentSize != 0) *sentSize = size;
    return 0;
}

Result Socket::SendToMulti(const void* buffer, int size, const SockAddrIn* addresses, int count, int* sentSize)
{
    (void)buffer; (void)addresses; (void)count;
    if (sentSize != 0) *sentSize = size;
    return 0;
}

Result Socket::SetTtl(unsigned char ttl)
{
    m_ttl = ttl;
    return 0;
}

Result Socket::InetAtoH(const char* text, InetAddress* address)
{
    if (text == 0 || address == 0)
        return 0x10c07;
    unsigned int value = 0;
    unsigned int part = 0;
    unsigned int parts = 0;
    for (const char* p = text; ; ++p)
    {
        const unsigned char c = static_cast<unsigned char>(*p);
        if (c >= '0' && c <= '9')
            part = part * 10 + static_cast<unsigned int>(c - '0');
        else if (c == '.' || c == '\0')
        {
            if (part > 255 || parts == 4)
                return 0x10c07;
            value = (value << 8) | part;
            part = 0;
            ++parts;
            if (c == '\0')
                break;
        }
        else
            return 0x10c07;
    }
    if (parts != 4)
        return 0x10c07;
    address->m_address = value;
    return 0;
}

}}}
