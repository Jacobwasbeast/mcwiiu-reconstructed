#include "nn/pia/lan/LanSessionInfo.hpp"

#include "nn/pia/String.hpp"
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

namespace
{
enum
{
    cResultInvalidArgument = 0x10c07,
    cSerializedFixedSize = 0x1cf,
    cUserNameAsciiBytes = 0x28,
    cUserNameUtf16Units = 0x14,
    cSessionParamSize = 0x20,
    cApplicationDataSize = 0x180,
    cAttributeCount = 6
};

const unsigned long long cTraceSessionInfo = 0x0100000000000000ULL;
}

unsigned int LanSessionInfo::GetSerializedSize() const
{
    return (m_stationConnectionInfo.GetSerializedSize() >> 1) + cSerializedFixedSize +
           LanStationInfo::GetSerializedSize() * cMaxStationInfo;
}

common::Result LanSessionInfo::Serialize(unsigned char* buffer, unsigned int* size,
                                         unsigned int bufferSize) const
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (size == 0)
        return cResultInvalidArgument;
    if (bufferSize < GetSerializedSize())
        return cResultInvalidArgument;

    const transport::StationLocation& location = m_stationConnectionInfo.m_stationLocation;
    unsigned char* cursor = buffer;
    common::serializeU32(cursor, m_gameMode);
    common::serializeU32(cursor + 4, m_sessionId);
    cursor += 8;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        common::serializeU32(cursor, m_attribute[index]);
        cursor += 4;
    }
    common::serializeU16(cursor, m_currentParticipantNum);
    common::serializeU16(cursor + 2, m_minParticipantNum);
    common::serializeU16(cursor + 4, m_maxParticipantNum);
    common::serializeU8(cursor + 6, m_systemCommunicationVersion);
    common::serializeU8(cursor + 7, m_applicationCommunicationVersion);
    common::serializeU16(cursor + 8, m_sessionType);
    cursor += 10;
    for (unsigned int index = 0; index < cApplicationDataSize; ++index)
    {
        common::serializeU8(cursor, m_applicationData[index]);
        cursor += 1;
    }
    common::serializeU32(cursor, m_applicationDataSize);
    common::serializeU8(cursor + 4, m_open);
    cursor += 5;

    unsigned int written;
    location.m_stationAddress.Serialize(cursor, &written, bufferSize - (cursor - buffer));
    cursor += location.m_stationAddress.GetSerializedSize();
    common::serializeU64(cursor, location.m_principalId);
    common::serializeU32(cursor + 8, location.m_field18);
    common::serializeU32(cursor + 12, location.m_field1c);
    common::serializeU8(cursor + 16, location.m_field20);
    common::serializeU8(cursor + 17, location.m_field21);
    common::serializeU8(cursor + 18, location.m_field22);
    common::serializeU8(cursor + 19, location.m_field23);
    common::serializeU8(cursor + 20, location.m_field24);
    common::serializeU8(cursor + 21, location.m_field25);
    common::serializeU8(cursor + 22, location.m_field26);
    location.m_inetAddress.Serialize(cursor + 23, &written, bufferSize - ((cursor + 23) - buffer));
    cursor += 29;

    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
    {
        if (index < m_stationInfoCount)
        {
            const LanStationInfo& stationInfo = m_stationInfo[index];
            stationInfo.GetUserName().Trace(cTraceSessionInfo);
            common::serializeU8(cursor, stationInfo.GetRole());
            common::serializeU8(cursor + 1, stationInfo.GetUserName().GetEncodingType());
            if (stationInfo.GetUserName().GetEncodingType() != common::String::EncodingType_Utf16)
            {
                const char* name = stationInfo.GetUserName().GetCharArray();
                cursor += 2;
                for (unsigned int unit = 0; unit < cUserNameAsciiBytes; ++unit)
                {
                    common::serializeU8(cursor, name[unit]);
                    cursor += 1;
                }
            }
            else
            {
                const unsigned short* name =
                    static_cast<const unsigned short*>(stationInfo.GetUserName().GetBuffer());
                cursor += 2;
                for (unsigned int unit = 0; unit < cUserNameUtf16Units; ++unit)
                {
                    common::serializeU16(cursor, name[unit]);
                    cursor += 2;
                }
            }
            common::serializeU64(cursor, stationInfo.GetStationId());
            cursor += 8;
        }
        else
        {
            for (unsigned int offset = 0; offset < LanStationInfo::GetSerializedSize(); ++offset)
            {
                common::serializeU8(cursor, 0);
                cursor += 1;
            }
        }
    }

    for (unsigned int index = 0; index < cSessionParamSize; ++index)
    {
        common::serializeU8(cursor, m_sessionParam[index]);
        cursor += 1;
    }
    *size = cursor - buffer;
    return 0;
}

common::Result LanSessionInfo::Deserialize(const unsigned char* buffer, unsigned int bufferSize)
{
    if (buffer == 0)
        return cResultInvalidArgument;
    if (bufferSize < GetSerializedSize())
        return cResultInvalidArgument;

    transport::StationLocation& location = m_stationConnectionInfo.m_stationLocation;
    const unsigned char* cursor = buffer;
    m_gameMode = common::deserializeU32(cursor);
    m_sessionId = common::deserializeU32(cursor + 4);
    cursor += 8;
    for (unsigned int index = 0; index < cAttributeCount; ++index)
    {
        m_attribute[index] = common::deserializeU32(cursor);
        cursor += 4;
    }
    m_currentParticipantNum = common::deserializeU16(cursor);
    m_minParticipantNum = common::deserializeU16(cursor + 2);
    m_maxParticipantNum = common::deserializeU16(cursor + 4);
    m_systemCommunicationVersion = common::deserializeU8(cursor + 6);
    m_applicationCommunicationVersion = common::deserializeU8(cursor + 7);
    m_sessionType = common::deserializeU16(cursor + 8);
    cursor += 10;
    for (unsigned int index = 0; index < cApplicationDataSize; ++index)
    {
        m_applicationData[index] = common::deserializeU8(cursor);
        cursor += 1;
    }
    m_applicationDataSize = common::deserializeU32(cursor);
    m_open = common::deserializeU8(cursor + 4) != 0;
    cursor += 5;

    location.m_stationAddress.Deserialize(cursor);
    cursor += location.m_stationAddress.GetSerializedSize();
    location.m_principalId = common::deserializeU64(cursor);
    location.m_field18 = common::deserializeU32(cursor + 8);
    location.m_field1c = common::deserializeU32(cursor + 12);
    location.m_field20 = common::deserializeU8(cursor + 16);
    location.m_field21 = common::deserializeU8(cursor + 17);
    location.m_field22 = common::deserializeU8(cursor + 18);
    location.m_field23 = common::deserializeU8(cursor + 19);
    location.m_field24 = common::deserializeU8(cursor + 20);
    location.m_field25 = common::deserializeU8(cursor + 21);
    location.m_field26 = common::deserializeU8(cursor + 22);
    location.m_inetAddress.Deserialize(cursor + 23);
    cursor += 29;
    m_stationConnectionInfo.m_localStationLocation.SetStationLocation(location);

    m_stationInfoCount = 0;
    for (unsigned int index = 0; index < cMaxStationInfo; ++index)
    {
        LanStationInfo& stationInfo = m_stationInfo[index];
        stationInfo.SetRole(static_cast<session::SessionStationInfo::StationInfoRole>(
            common::deserializeU8(cursor)));
        if (common::deserializeU8(cursor + 1) == common::String::EncodingType_Utf16)
        {
            unsigned short name[cUserNameUtf16Units];
            cursor += 2;
            for (unsigned int unit = 0; unit < cUserNameUtf16Units; ++unit)
            {
                name[unit] = common::deserializeU16(cursor);
                cursor += 2;
            }
            common::FixedString<10> userName;
            userName.SetString(name, userName.CalcBytes(name, common::String::EncodingType_Utf16),
                               common::String::EncodingType_Utf16);
            stationInfo.SetUserName(userName);
            stationInfo.SetStationId(common::deserializeU64(cursor));
            cursor += 8;
        }
        else
        {
            char name[cUserNameAsciiBytes];
            cursor += 2;
            for (unsigned int unit = 0; unit < cUserNameAsciiBytes; ++unit)
            {
                name[unit] = common::deserializeU8(cursor);
                cursor += 1;
            }
            common::FixedString<10> userName;
            userName.SetString(name, userName.CalcBytes(name, common::String::EncodingType_Ascii),
                               common::String::EncodingType_Ascii);
            stationInfo.SetUserName(userName);
            stationInfo.SetStationId(common::deserializeU64(cursor));
            cursor += 8;
        }
        stationInfo.GetUserName().Trace(cTraceSessionInfo);
        if (stationInfo.GetRole() != session::SessionStationInfo::cStationInfoRole_Invalid ||
            stationInfo.GetUserName().GetStringLength() != 0)
            ++m_stationInfoCount;
    }

    for (unsigned int index = 0; index < cSessionParamSize; ++index)
    {
        m_sessionParam[index] = common::deserializeU8(cursor);
        cursor += 1;
    }
    return 0;
}

}}}
