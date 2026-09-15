#include "nn/pia/transport/StationLocation.hpp"

#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace transport {

StationLocation::StationLocation()
    : common::StationAddress(), m_field10(0), m_field14(0),
      m_field18(0), m_field1c(0), m_field20(0), m_field21(0), m_field22(0),
      m_field23(0), m_field24(0), m_field25(0), m_field26(0), m_inetAddress()
{
}

StationLocation::StationLocation(const StationLocation& other)
    : common::StationAddress(), m_inetAddress()
{
    SetStationLocation(other);
}

StationLocation::~StationLocation()
{
}

void StationLocation::SetStationLocation(const StationLocation& other)
{
    common::StationAddress::operator=(other);
    m_field10 = other.m_field10;
    m_field14 = other.m_field14;
    m_field18 = other.m_field18;
    m_field1c = other.m_field1c;
    m_field20 = other.m_field20;
    m_field21 = other.m_field21;
    m_field22 = other.m_field22;
    m_field23 = other.m_field23;
    m_field24 = other.m_field24;
    m_field25 = other.m_field25;
    m_field26 = other.m_field26;
    m_inetAddress = other.m_inetAddress;
}

StationLocation& StationLocation::operator=(const StationLocation& other)
{
    if (this != &other)
        SetStationLocation(other);
    return *this;
}

void StationLocation::SetStationAddress(const common::StationAddress& address)
{
    if (static_cast<const void*>(this) != static_cast<const void*>(&address))
        common::InetAddress::operator=(address);
}

bool StationLocation::operator==(const StationLocation& other) const
{
    return common::StationAddress::operator==(other) &&
           m_field10 == other.m_field10 && m_field14 == other.m_field14 &&
           m_field18 == other.m_field18 && m_field1c == other.m_field1c &&
           m_field20 == other.m_field20 && m_field21 == other.m_field21 &&
           m_field22 == other.m_field22 && m_field23 == other.m_field23 &&
           m_field24 == other.m_field24 && m_field25 == other.m_field25 &&
           m_field26 == other.m_field26 &&
           m_inetAddress.GetKey() == other.m_inetAddress.GetKey();
}

unsigned int StationLocation::GetSerializedSize() const
{
    return common::StationAddress::GetSerializedSize() + 0x1d;
}

common::Result StationLocation::Serialize(unsigned char* buffer, unsigned int* size,
                                           unsigned int maxSize) const
{
    if (buffer == 0 || size == 0)
        return 0x10c07;
    if (maxSize < GetSerializedSize())
        return 0x4c0d;

    unsigned int addressSize = 0;
    common::Result result = common::StationAddress::Serialize(buffer, &addressSize, maxSize);
    if (result != 0)
        return result;
    unsigned char* cursor = buffer + addressSize;
    common::serializeU32(cursor, static_cast<unsigned int>(m_field10));
    common::serializeU32(cursor + 4, static_cast<unsigned int>(m_field14));
    common::serializeU32(cursor + 8, static_cast<unsigned int>(m_field18));
    common::serializeU32(cursor + 12, static_cast<unsigned int>(m_field1c));
    cursor[16] = m_field20;
    cursor[17] = m_field21;
    cursor[18] = m_field22;
    cursor[19] = m_field23;
    cursor[20] = m_field24;
    cursor[21] = m_field25;
    cursor[22] = m_field26;
    unsigned int inetSize = 0;
    result = m_inetAddress.Serialize(cursor + 23, &inetSize, maxSize - addressSize - 23);
    if (result != 0)
        return result;
    *size = addressSize + 23 + inetSize;
    return 0;
}

common::Result StationLocation::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    common::Result result = common::StationAddress::Deserialize(buffer);
    if (result != 0)
        return result;
    const unsigned char* cursor = buffer + common::StationAddress::GetSerializedSize();
    m_field10 = static_cast<int>(common::deserializeU32(cursor));
    m_field14 = static_cast<int>(common::deserializeU32(cursor + 4));
    m_field18 = static_cast<int>(common::deserializeU32(cursor + 8));
    m_field1c = static_cast<int>(common::deserializeU32(cursor + 12));
    m_field20 = cursor[16];
    m_field21 = cursor[17];
    m_field22 = cursor[18];
    m_field23 = cursor[19];
    m_field24 = cursor[20];
    m_field25 = cursor[21];
    m_field26 = cursor[22];
    return m_inetAddress.Deserialize(cursor + 23);
}

void StationLocation::Trace(unsigned long long) const
{
}

}}}
