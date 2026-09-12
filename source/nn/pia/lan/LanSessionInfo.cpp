#include "nn/pia/lan/LanSessionInfo.hpp"

namespace nn { namespace pia { namespace lan {

LanSessionInfo::LanSessionInfo()
    : m_field04(0),
      m_field08(0),
      m_field0c(0),
      m_field0e(0),
      m_field10(0),
      m_field12(0),
      m_field2c(5),
      m_field2d(0),
      m_field2e(0),
      m_applicationDataSize(0),
      m_stationConnectionInfo(),
      m_stationInfoCount(0)
{
    for (unsigned int index = 0; index < 6; ++index)
        m_attribute[index] = 0;

    for (unsigned int index = 0; index < sizeof(m_reserved30); ++index)
        m_reserved30[index] = 0;

    for (unsigned int index = 0; index < sizeof(m_applicationData); ++index)
        m_applicationData[index] = 0;

    for (unsigned int index = 0; index < 16; ++index)
        m_stationInfo[index].Clear();
}

LanSessionInfo::~LanSessionInfo()
{
}

void LanSessionInfo::Copy(const LanSessionInfo& other)
{
    m_field04 = other.m_field04;
    m_field08 = other.m_field08;
    m_field0c = other.m_field0c;
    m_field0e = other.m_field0e;
    m_field10 = other.m_field10;
    m_field12 = other.m_field12;

    for (unsigned int index = 0; index < 6; ++index)
        m_attribute[index] = other.m_attribute[index];

    for (unsigned int index = 0; index < sizeof(m_applicationData); ++index)
        m_applicationData[index] = other.m_applicationData[index];

    for (unsigned int index = 0; index < sizeof(m_reserved30); ++index)
        m_reserved30[index] = other.m_reserved30[index];

    m_applicationDataSize = other.m_applicationDataSize;
    m_field2e = other.m_field2e;
    m_field2c = other.m_field2c;
    m_field2d = other.m_field2d;

    m_stationConnectionInfo = other.m_stationConnectionInfo;

    for (unsigned int index = 0; index < 16; ++index)
    {
        m_stationInfo[index].SetUserName(other.m_stationInfo[index].GetUserName());
        m_stationInfo[index].SetRole(other.m_stationInfo[index].GetRole());
        m_stationInfo[index].SetStationId(other.m_stationInfo[index].GetStationId());
    }

    m_stationInfoCount = other.m_stationInfoCount;
}

}}}
