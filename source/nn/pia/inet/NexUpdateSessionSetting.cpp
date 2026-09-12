#include "nn/pia/inet/NexUpdateSessionSetting.hpp"

#include <cstring>

namespace nn { namespace pia { namespace inet {

NexUpdateSessionSetting::NexUpdateSessionSetting()
    : session::UpdateSessionSetting(),
      m_minParticipantNum(0xffff),
      m_maxParticipantNum(0xffff),
      m_sessionType(cSessionType_Normal),
      m_autoCloseSession(false),
      m_applicationDataSize(0),
      m_sessionUserPassword(),
      m_sessionMatchmakeKeyword(),
      m_ratingValue(0),
      m_isSetRatingValue(false),
      m_disconnectionRate(0),
      m_isSetDisconnectionRate(false),
      m_violationRate(0),
      m_isSetViolationRate(false),
      m_countryCode(0),
      m_isSetCountryCode(false),
      m_updateGeoIp(false),
      m_isSetUpdateGeoIp(false),
      m_selectionPriority(0),
      m_nexModificationFlag(0)
{
    for (unsigned int index = 0; index < cAttributeNum; ++index)
    {
        m_attribute[index] = 0;
        m_attributeUpdated[index] = false;
    }
    std::memset(m_applicationData, 0, sizeof(m_applicationData));
}

NexUpdateSessionSetting::~NexUpdateSessionSetting()
{
}

void NexUpdateSessionSetting::Reset()
{
    session::UpdateSessionSetting::Reset();
    m_minParticipantNum = 0xffff;
    m_maxParticipantNum = 0xffff;
    m_sessionType = cSessionType_Normal;
    m_autoCloseSession = false;
    for (unsigned int index = 0; index < cAttributeNum; ++index)
    {
        m_attribute[index] = 0;
        m_attributeUpdated[index] = false;
    }
    std::memset(m_applicationData, 0, sizeof(m_applicationData));
    m_applicationDataSize = 0;
    m_sessionUserPassword.Clear();
    m_sessionMatchmakeKeyword.Clear();
    m_ratingValue = 0;
    m_isSetRatingValue = false;
    m_disconnectionRate = 0;
    m_isSetDisconnectionRate = false;
    m_violationRate = 0;
    m_isSetViolationRate = false;
    m_countryCode = 0;
    m_isSetCountryCode = false;
    m_updateGeoIp = false;
    m_isSetUpdateGeoIp = false;
    m_selectionPriority = 0;
    m_nexModificationFlag = 0;
}

void NexUpdateSessionSetting::Trace(unsigned long long value) const
{
}

void NexUpdateSessionSetting::AddNexModificationFlag(unsigned int flag)
{
    m_nexModificationFlag |= flag;
}

unsigned int NexUpdateSessionSetting::GetNexModificationFlag() const
{
    return m_nexModificationFlag;
}

unsigned short NexUpdateSessionSetting::GetMinParticipantNum() const
{
    return m_minParticipantNum;
}

unsigned short NexUpdateSessionSetting::GetMaxParticipantNum() const
{
    return m_maxParticipantNum;
}

SessionType NexUpdateSessionSetting::GetSessionType() const
{
    return m_sessionType;
}

bool NexUpdateSessionSetting::IsAutoCloseSession() const
{
    return m_autoCloseSession;
}

Result NexUpdateSessionSetting::SetAttribute(unsigned int index, unsigned int value)
{
    if (index >= cAttributeNum)
        return 0x10c07;
    if ((m_nexModificationFlag != 0 && m_nexModificationFlag != 1) || GetModificationFlag() != 0)
        return 0x10c08;

    m_attribute[index] = value;
    m_attributeUpdated[index] = true;

    bool allUpdated = true;
    for (unsigned int i = 0; i < cAttributeNum; ++i)
        allUpdated = allUpdated && m_attributeUpdated[i];
    if (allUpdated)
        AddNexModificationFlag(1);

    return 0;
}

unsigned int NexUpdateSessionSetting::GetAttribute(unsigned int index) const
{
    if (index >= cAttributeNum)
        return 0;
    return m_attribute[index];
}

bool NexUpdateSessionSetting::IsAttributeUpdated(unsigned int index) const
{
    if (index >= cAttributeNum)
        return false;
    return m_attributeUpdated[index];
}

Result NexUpdateSessionSetting::SetApplicationData(const void* data, unsigned int size)
{
    if (data == 0)
        return 0x10c07;
    if (size > sizeof(m_applicationData))
        return 0x10c07;

    std::memset(m_applicationData, 0, sizeof(m_applicationData));
    std::memcpy(m_applicationData, data, size);
    m_applicationDataSize = size;
    m_modificationFlag |= 2;
    return 0;
}

Result NexUpdateSessionSetting::GetApplicationData(void* buffer, unsigned int bufferSize) const
{
    if (buffer == 0)
        return 0x10c07;
    if (m_applicationDataSize == 0)
        return 0x10c08;
    if (bufferSize < m_applicationDataSize)
        return 0x10c04;

    std::memcpy(buffer, m_applicationData, m_applicationDataSize);
    return 0;
}

unsigned int NexUpdateSessionSetting::GetApplicationDataSize() const
{
    return m_applicationDataSize;
}

const common::String& NexUpdateSessionSetting::GetSessionUserPassword() const
{
    return m_sessionUserPassword;
}

unsigned int NexUpdateSessionSetting::GetRatingValue() const
{
    return m_ratingValue;
}

bool NexUpdateSessionSetting::IsSetRatingValue() const
{
    return m_isSetRatingValue;
}

unsigned int NexUpdateSessionSetting::GetDisconnectionRate() const
{
    return m_disconnectionRate;
}

bool NexUpdateSessionSetting::IsSetDisconnectionRate() const
{
    return m_isSetDisconnectionRate;
}

unsigned int NexUpdateSessionSetting::GetViolationRate() const
{
    return m_violationRate;
}

bool NexUpdateSessionSetting::IsSetViolationRate() const
{
    return m_isSetViolationRate;
}

unsigned int NexUpdateSessionSetting::GetCountryCode() const
{
    return m_countryCode;
}

bool NexUpdateSessionSetting::IsSetCountryCode() const
{
    return m_isSetCountryCode;
}

bool NexUpdateSessionSetting::GetUpdateGeoIp() const
{
    return m_updateGeoIp;
}

unsigned char NexUpdateSessionSetting::GetSelectionPriority() const
{
    return m_selectionPriority;
}

}}}
