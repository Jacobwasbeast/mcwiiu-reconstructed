#include "nn/pia/common/InetAddress.hpp"

#include "nn/pia/String.hpp"

namespace nn { namespace pia { namespace common {

InetAddress::InetAddress()
    : m_address(0), m_port(0)
{
}

InetAddress::InetAddress(unsigned int address, unsigned short port)
    : m_address(address), m_port(port)
{
}

InetAddress::InetAddress(const InetAddress& other)
{
    *this = other;
}

InetAddress::~InetAddress()
{
}

void InetAddress::operator=(const InetAddress& other)
{
    m_address = other.m_address;
    m_port = other.m_port;
}

unsigned long long InetAddress::GetKey() const
{
    return (static_cast<unsigned long long>(m_port) << 32) | m_address;
}

void InetAddress::GetAddressString(String* string) const
{
    string->Clear();
    string->Format("%d.%d.%d.%d:%d", m_address >> 24, (m_address >> 16) & 0xff,
                   (m_address >> 8) & 0xff, m_address & 0xff, m_port);
}

void InetAddress::Trace(unsigned long long value) const
{
}

Result InetAddress::Serialize(unsigned char* buffer, unsigned int* size,
                              unsigned int maxSize) const
{
    if (buffer == 0)
        return 0x10c07;
    if (size == 0)
        return 0x10c07;
    if (maxSize < cSerializedSize)
        return 0x10c07;

    serializeU32(buffer, m_address);
    serializeU16(buffer + 4, m_port);

    *size = cSerializedSize;
    return 0;
}

Result InetAddress::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;

    m_address = deserializeU32(buffer);
    m_port = deserializeU16(buffer + 4);
    return 0;
}

unsigned int InetAddress::IsValid() const
{
    if (m_address != 0 && m_port != 0)
        return 1;
    return 0;
}

bool InetAddress::IsValidAddress() const
{
    return m_address != 0;
}

unsigned int InetAddress::IsPrivate() const
{
    const unsigned int first = m_address >> 24;
    const unsigned int second = (m_address >> 16) & 0xff;

    if (first == 10)
        return 1;
    if (first == 172 && second >= 16 && second <= 31)
        return 1;
    if (first == 192 && second == 168)
        return 1;
    return 0;
}

}}}
