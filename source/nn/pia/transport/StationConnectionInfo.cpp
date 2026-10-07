#include "nn/pia/transport/StationConnectionInfo.hpp"
#include "nn/pia/Common.hpp"

namespace nn { namespace pia { namespace transport {

using common::Result;

StationConnectionInfo::StationConnectionInfo()
    : m_stationLocation(),
      m_localStationLocation()
{
}

StationConnectionInfo::StationConnectionInfo(const StationConnectionInfo& other)
    : m_stationLocation(),
      m_localStationLocation()
{
    SetStationConnectionInfo(other);
}

StationConnectionInfo::~StationConnectionInfo()
{
}

void StationConnectionInfo::SetStationConnectionInfo(const StationConnectionInfo& other)
{
    m_stationLocation.SetStationLocation(other.m_stationLocation);
    m_localStationLocation.SetStationLocation(other.m_localStationLocation);
}

StationConnectionInfo& StationConnectionInfo::operator=(const StationConnectionInfo& other)
{
    if (this != &other)
        SetStationConnectionInfo(other);
    return *this;
}

bool StationConnectionInfo::operator==(const StationConnectionInfo& other) const
{
    return m_stationLocation == other.m_stationLocation &&
           m_localStationLocation == other.m_localStationLocation;
}

unsigned int StationConnectionInfo::GetSerializedSize() const
{
    return m_stationLocation.GetSerializedSize() + m_localStationLocation.GetSerializedSize();
}

Result StationConnectionInfo::Serialize(unsigned char* buffer, unsigned int* size,
                                         unsigned int maxSize) const
{
    if (buffer == 0 || size == 0)
        return 0x10c07;
    if (maxSize < GetSerializedSize())
        return 0x4c0d;

    unsigned int written = 0;
    Result result = m_stationLocation.Serialize(buffer, &written, maxSize);
    if (result != 0)
        return result;
    unsigned int localWritten = 0;
    result = m_localStationLocation.Serialize(buffer + written, &localWritten,
                                               maxSize - written);
    if (result == 0)
        *size = written + localWritten;
    return result;
}

Result StationConnectionInfo::Deserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    Result result = m_stationLocation.Deserialize(buffer);
    if (result != 0)
        return result;
    return m_localStationLocation.Deserialize(buffer + m_stationLocation.GetSerializedSize());
}

unsigned int StationConnectionInfo::GetStreamlinedSerializedSize() const
{
    return 0x60;
}

Result StationConnectionInfo::StreamlinedSerialize(unsigned char* buffer, unsigned int* size,
                                                    unsigned int maxSize) const
{
    if (buffer == 0 || size == 0)
        return 0x10c07;
    if (maxSize < GetStreamlinedSerializedSize())
        return 0x4c0d;

    unsigned int written = 0;
    Result result = m_stationLocation.m_stationAddress.m_inetAddress
                        .Serialize(buffer, &written, common::InetAddress::cSerializedSize);
    if (result != 0)
        return result;
    result = m_localStationLocation.m_stationAddress.m_inetAddress
                 .Serialize(buffer + common::InetAddress::cSerializedSize, &written,
                            common::InetAddress::cSerializedSize);
    if (result != 0)
        return result;
    unsigned char* members = buffer + 2 * common::InetAddress::cSerializedSize;
    unsigned int membersSize = LittleMembersSerialize(members, m_stationLocation);
    unsigned int localMembersSize = LittleMembersSerialize(members + membersSize,
                                                           m_localStationLocation);
    *size = 2 * common::InetAddress::cSerializedSize + membersSize + localMembersSize;
    return 0;
}

Result StationConnectionInfo::StreamlinedDeserialize(const unsigned char* buffer)
{
    if (buffer == 0)
        return 0x10c07;
    common::InetAddress address;
    Result result = address.Deserialize(buffer);
    if (result != 0)
        return result;
    result = m_stationLocation.m_stationAddress.SetInetAddress(address);
    if (result != 0)
        return result;
    result = address.Deserialize(buffer + common::InetAddress::cSerializedSize);
    if (result != 0)
        return result;
    result = m_localStationLocation.m_stationAddress.SetInetAddress(address);
    if (result != 0)
        return result;
    const unsigned char* members = buffer + 2 * common::InetAddress::cSerializedSize;
    unsigned int membersSize = LittleMembersDeserialize(&m_stationLocation, members);
    LittleMembersDeserialize(&m_localStationLocation, members + membersSize);
    return 0;
}

void StationConnectionInfo::Trace(unsigned long long) const
{
}

unsigned int StationConnectionInfo::LittleMembersSerialize(unsigned char* buffer,
                                                            const StationLocation& location) const
{
    common::serializeU64(buffer, location.m_principalId);
    common::serializeU32(buffer + 8, location.m_field25);
    common::serializeU32(buffer + 12, (location.m_field18 & 0x3fffffU)
        | ((location.m_field20 & 3U) << 22) | (location.m_field21 << 24));
    common::serializeU32(buffer + 16, (location.m_field1c & 0x3fffffU)
        | ((location.m_field23 & 3U) << 24) | ((location.m_field24 & 3U) << 26)
        | (location.m_field22 << 28));
    return 20;
}

unsigned int StationConnectionInfo::LittleMembersDeserialize(StationLocation* location,
                                                              const unsigned char* buffer)
{
    location->m_principalId = common::deserializeU64(buffer);
    location->m_field25 = static_cast<unsigned char>(common::deserializeU32(buffer + 8));
    unsigned int first = common::deserializeU32(buffer + 12);
    location->m_field18 = first & 0x3fffffU;
    location->m_field20 = static_cast<unsigned char>((first >> 22) & 3U);
    location->m_field21 = static_cast<unsigned char>(first >> 24);
    unsigned int second = common::deserializeU32(buffer + 16);
    location->m_field1c = second & 0x3fffffU;
    location->m_field23 = static_cast<unsigned char>((second >> 24) & 3U);
    location->m_field24 = static_cast<unsigned char>((second >> 26) & 3U);
    location->m_field22 = static_cast<unsigned char>(second >> 28);
    return 20;
}

}}}
