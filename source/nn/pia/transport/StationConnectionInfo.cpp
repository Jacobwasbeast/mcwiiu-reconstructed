#include "nn/pia/transport/StationConnectionInfo.hpp"

namespace nn { namespace pia { namespace transport {

StationLocation::StationLocation()
    : common::StationAddress(),
      m_field0c(0),
      m_field10(0),
      m_field14(0),
      m_field18(0),
      m_field1c(0),
      m_field20(0),
      m_field21(0),
      m_field22(0),
      m_field23(0),
      m_field24(0),
      m_field25(0),
      m_field26(0),
      m_inetAddress()
{
}

StationLocation::StationLocation(const StationLocation& other)
    : common::StationAddress(other)
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

void StationLocation::SetStationAddress(const common::StationAddress& address)
{
    common::StationAddress::operator=(address);
}

bool StationLocation::operator==(const StationLocation& other) const
{
    return common::StationAddress::operator==(other);
}

StationLocation& StationLocation::operator=(const StationLocation& other)
{
    if (this != &other)
        SetStationLocation(other);
    return *this;
}

void StationLocation::GetLocationString(common::String*) const
{
}

unsigned int StationLocation::GetSerializedSize() const
{
    return 35;
}

void StationLocation::Serialize(unsigned char*, unsigned int*, unsigned int) const
{
}

void StationLocation::Deserialize(const unsigned char*)
{
}

void StationLocation::Trace(unsigned long long) const
{
}

StationConnectionInfo::StationConnectionInfo()
    : StationLocation(),
      m_localStationLocation()
{
}

StationConnectionInfo::StationConnectionInfo(const StationConnectionInfo& other)
    : StationLocation(other),
      m_localStationLocation(other.m_localStationLocation)
{
    SetStationConnectionInfo(other);
}

StationConnectionInfo::~StationConnectionInfo()
{
}

void StationConnectionInfo::SetStationConnectionInfo(const StationConnectionInfo& other)
{
    SetStationLocation(other);
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
    return StationLocation::operator==(other) &&
           m_localStationLocation == other.m_localStationLocation;
}

unsigned int StationConnectionInfo::GetSerializedSize() const
{
    return StationLocation::GetSerializedSize() + m_localStationLocation.GetSerializedSize();
}

void StationConnectionInfo::Serialize(unsigned char*, unsigned int*, unsigned int) const
{
}

void StationConnectionInfo::Deserialize(const unsigned char*)
{
}

unsigned int StationConnectionInfo::GetStreamlinedSerializedSize() const
{
    return 0x60;
}

void StationConnectionInfo::StreamlinedSerialize(unsigned char*, unsigned int*, unsigned int) const
{
}

void StationConnectionInfo::StreamlinedDeserialize(const unsigned char*)
{
}

void StationConnectionInfo::Trace(unsigned long long) const
{
}

void StationConnectionInfo::LittleMembersSerialize(unsigned char*, const StationLocation&) const
{
}

void StationConnectionInfo::LittleMembersDeserialize(StationLocation*, const unsigned char*)
{
}

}}}
