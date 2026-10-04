#include "nn/pia/lan/LanSessionInfo.hpp"

#include "platform/wiiu/pead.hpp"

#include <coreinit/memory.h>

namespace nn { namespace pia { namespace lan {

LanSessionInfo::LanSessionInfo()
    : m_gameMode(0),
      m_sessionId(0),
      m_currentParticipantNum(0),
      m_minParticipantNum(0),
      m_maxParticipantNum(0),
      m_open(false),
      m_systemCommunicationVersion(5),
      m_applicationCommunicationVersion(0),
      m_sessionType(0)
{
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
    OSBlockSet(m_attribute, 0, sizeof(m_attribute));
    OSBlockSet(m_sessionParam, 0, sizeof(m_sessionParam));
    m_stationConnectionInfo = transport::StationConnectionInfo();
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
        m_stationInfo[index].Clear();
    m_stationInfoCount = 0;
}

LanSessionInfo::LanSessionInfo(const LanSessionInfo& other)
{
    Copy(other);
}

LanSessionInfo::~LanSessionInfo()
{
}

LanSessionInfo& LanSessionInfo::operator=(const LanSessionInfo& other)
{
    Copy(other);
    return *this;
}

unsigned int LanSessionInfo::GetGameMode() const
{
    return m_gameMode;
}

unsigned int LanSessionInfo::GetSessionId() const
{
    return m_sessionId;
}

unsigned short LanSessionInfo::GetCurrentParticipantNum() const
{
    return m_currentParticipantNum;
}

unsigned short LanSessionInfo::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}

unsigned short LanSessionInfo::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}

bool LanSessionInfo::IsOpened() const
{
    return m_open;
}

common::Result LanSessionInfo::GetApplicationData(void* buffer, unsigned int bufferSize) const
{
    if (buffer == 0)
        return 0x10c07;
    if (bufferSize < m_applicationDataSize)
        return 0x10c04;

    OSBlockMove(buffer, m_applicationData, m_applicationDataSize, FALSE);
    return 0;
}

unsigned int LanSessionInfo::GetApplicationDataSize() const
{
    return m_applicationDataSize;
}

void LanSessionInfo::Clear()
{
    m_minParticipantNum = 0;
    m_gameMode = 0;
    m_sessionId = 0;
    m_open = false;
    m_currentParticipantNum = 0;
    m_maxParticipantNum = 0;
    OSBlockSet(m_attribute, 0, sizeof(m_attribute));
    OSBlockSet(m_sessionParam, 0, sizeof(m_sessionParam));
    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    m_sessionType = 0;
    m_applicationDataSize = 0;
    m_systemCommunicationVersion = 5;
    m_applicationCommunicationVersion = 0;
    m_stationConnectionInfo = transport::StationConnectionInfo();
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
        m_stationInfo[index].Clear();
    m_stationInfoCount = 0;
}

void LanSessionInfo::Trace(unsigned long long) const
{
}

common::Result LanSessionInfo::GetAttribute(unsigned char index, unsigned int* value) const
{
    if (value == 0)
        return 0x10c07;
    if (index >= cAttributeCount)
        return 0x10c07;
    *value = m_attribute[index];
    return 0;
}

unsigned short LanSessionInfo::GetSessionType() const
{
    return m_sessionType;
}

unsigned long long LanSessionInfo::GetHostPrincipalId() const
{
    return m_stationConnectionInfo.m_stationLocation.m_principalId;
}

common::Result LanSessionInfo::GetSessionStationInfo(LanStationInfo* stationInfo, unsigned int count)
{
    if (stationInfo == 0)
        return 0x10c07;
    if (m_stationInfoCount == 0)
        return 0x10c08;
    if (count < m_stationInfoCount)
        return 0x10c04;

    for (unsigned int index = 0; index < m_stationInfoCount; ++index)
    {
        stationInfo[index].SetRole(m_stationInfo[index].GetRole());
        stationInfo[index].SetUserName(m_stationInfo[index].GetUserName());
        stationInfo[index].SetStationId(m_stationInfo[index].GetStationId());
    }
    return 0;
}

unsigned int LanSessionInfo::GetSessionStationInfoSize() const
{
    return m_stationInfoCount;
}

unsigned char LanSessionInfo::GetApplicationCommunicationVersion() const
{
    return m_applicationCommunicationVersion;
}

unsigned char LanSessionInfo::GetSystemCommunicationVersion() const
{
    return m_systemCommunicationVersion;
}

void LanSessionInfo::Copy(const LanSessionInfo& other)
{
    m_gameMode = other.m_gameMode;
    m_sessionId = other.m_sessionId;
    m_currentParticipantNum = other.m_currentParticipantNum;
    m_minParticipantNum = other.m_minParticipantNum;
    m_maxParticipantNum = other.m_maxParticipantNum;
    m_open = other.m_open;
    OSBlockMove(m_attribute, other.m_attribute, sizeof(m_attribute), FALSE);
    OSBlockMove(m_applicationData, other.m_applicationData, sizeof(m_applicationData), FALSE);
    OSBlockMove(m_sessionParam, other.m_sessionParam, sizeof(m_sessionParam), FALSE);
    m_applicationDataSize = other.m_applicationDataSize;
    m_sessionType = other.m_sessionType;
    m_systemCommunicationVersion = other.m_systemCommunicationVersion;
    m_applicationCommunicationVersion = other.m_applicationCommunicationVersion;
    m_stationConnectionInfo = other.m_stationConnectionInfo;
    m_stationInfoCount = other.m_stationInfoCount;

    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
    {
        m_stationInfo[index].SetUserName(other.m_stationInfo[index].GetUserName());
        m_stationInfo[index].SetRole(other.m_stationInfo[index].GetRole());
        m_stationInfo[index].SetStationId(other.m_stationInfo[index].GetStationId());
    }
}

void LanSessionInfo::SetGameMode(unsigned int gameMode)
{
    m_gameMode = gameMode;
}

void LanSessionInfo::SetSessionId(unsigned int sessionId)
{
    m_sessionId = sessionId;
}

void LanSessionInfo::SetCurrentParticipantNum(unsigned short participantNum)
{
    m_currentParticipantNum = participantNum;
}

void LanSessionInfo::SetMinParticipantNum(unsigned short participantNum)
{
    m_minParticipantNum = participantNum;
}

void LanSessionInfo::SetMaxParticipantNum(unsigned short participantNum)
{
    m_maxParticipantNum = participantNum;
}

void LanSessionInfo::SetOpen(bool open)
{
    m_open = open;
}

void LanSessionInfo::SetSessionType(LanSessionType sessionType)
{
    m_sessionType = sessionType;
}

void LanSessionInfo::SetAttribute(unsigned char index, unsigned int value)
{
    m_attribute[index] = value;
}

void LanSessionInfo::SetApplicationData(const void* data, unsigned int size)
{
    if (size > cApplicationDataMaxSize)
        size = cApplicationDataMaxSize;

    OSBlockSet(m_applicationData, 0, sizeof(m_applicationData));
    OSBlockMove(m_applicationData, data, size, FALSE);
    m_applicationDataSize = size;
}

transport::StationConnectionInfo& LanSessionInfo::GetStationConnectionInfo()
{
    return m_stationConnectionInfo;
}

void LanSessionInfo::SetStationConnectionInfo(const transport::StationConnectionInfo& info)
{
    m_stationConnectionInfo = info;
}

void LanSessionInfo::AddSessionStationInfo(const LanStationInfo& stationInfo)
{
    if (m_stationInfoCount < cMaxStationInfo)
    {
        m_stationInfo[m_stationInfoCount].SetRole(stationInfo.GetRole());
        m_stationInfo[m_stationInfoCount].SetUserName(stationInfo.GetUserName());
        m_stationInfo[m_stationInfoCount].SetStationId(stationInfo.GetStationId());
        ++m_stationInfoCount;
    }
}

void LanSessionInfo::ClearSessionStationInfo()
{
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
        m_stationInfo[index].Clear();
    m_stationInfoCount = 0;
}

void LanSessionInfo::SetSystemCommunicationVersion(unsigned char version)
{
    m_systemCommunicationVersion = version;
}

void LanSessionInfo::SetApplicationCommunicationVersion(unsigned char version)
{
    m_applicationCommunicationVersion = version;
}

const unsigned char* LanSessionInfo::GetSessionParam() const
{
    return m_sessionParam;
}

}}}
