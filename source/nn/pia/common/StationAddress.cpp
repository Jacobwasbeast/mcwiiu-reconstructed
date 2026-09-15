#include "nn/pia/common/StationAddress.hpp"

namespace nn { namespace pia { namespace common {

StationAddress::StationAddress()
{
}

StationAddress::StationAddress(const StationAddress& other)
    : InetAddress(other)
{
}

StationAddress::~StationAddress()
{
}

Result StationAddress::SetInetAddress(const InetAddress& address)
{
    InetAddress::operator=(address);
    return 0;
}

void StationAddress::Clear()
{
    m_address = 0;
    m_port = 0;
}

Result StationAddress::Serialize(unsigned char* buffer, unsigned int* size,
                                 unsigned int maxSize) const
{
    const unsigned int serializedSize = cSerializedSize;
    if (buffer == 0)
        return 0x10c07;
    if (size == 0)
        return 0x10c07;
    if (maxSize < serializedSize)
        return 0x10c07;

    unsigned int inetAddressSize;
    const Result result = InetAddress::Serialize(buffer, &inetAddressSize, maxSize);
    if (result != 0)
        return result;

    *size = serializedSize;
    return 0;
}

Result StationAddress::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;

    const Result result = InetAddress::Deserialize(buffer);
    if (result != 0)
        return result;

    return 0;
}

unsigned int StationAddress::IsValid() const
{
    return InetAddress::IsValid();
}

bool StationAddress::operator==(const StationAddress& other) const
{
    return GetKey() == other.GetKey();
}

StationAddress& StationAddress::operator=(const StationAddress& other)
{
    if (this != &other)
        InetAddress::operator=(other);
    return *this;
}

bool StationAddress::operator<(const StationAddress& other) const
{
    return GetKey() < other.GetKey();
}

int StationAddress::Compare(const StationAddress& lhs, const StationAddress& rhs)
{
    if (lhs.GetKey() < rhs.GetKey())
        return -1;
    return lhs.GetKey() > rhs.GetKey();
}

void StationAddress::Trace(unsigned long long value) const
{
}

unsigned int StationAddress::GetSerializedSize() const
{
    return cSerializedSize;
}

}}}
