#include "nn/pia/inet/NexSessionSearchCriteria.hpp"

namespace nn { namespace pia { namespace inet {

NexSessionSearchCriteria::NexSessionSearchCriteria()
    : session::SessionSearchCriteria(),
      m_gameMode(0),
      m_sessionType(cSessionType_Normal),
      m_attributeList(),
      m_attributeListSize(),
      m_attributeRangeMin(),
      m_attributeRangeMax(),
      m_attributeRangeUsed(),
      m_excludeNonHostPrincipalId(true),
      m_excludeUserPasswordSet(true),
      m_randomSessionSelectionMethod(cSelectionMethod_0),
      m_scoreBasedSettingIndex(0),
      m_scoreBasedRatingValue(0),
      m_scoreBasedDisconnectionRate(0),
      m_scoreBasedViolationRate(0),
      m_countryCode(0),
      m_scoreBasedGeoIpUsed(false),
      m_overrideIpAddress(),
      m_communityId(0),
      m_minParticipantRangeMin(0xffff),
      m_minParticipantRangeMax(0xffff),
      m_maxParticipantRangeMin(0xffff),
      m_maxParticipantRangeMax(0xffff),
      m_openedOnly(true),
      m_vacantOnly(true),
      m_additionalScoreBasedParameterValueKey(),
      m_additionalScoreBasedParameterValues(),
      m_additionalScoreBasedParameterStringKey(),
      m_additionalScoreBasedParameterString(),
      m_sessionMatchmakeKeyword(),
      m_conditionMask(0)
{
    m_resultRangeOffset = static_cast<unsigned int>(-1);
    m_resultRangeSize = 100;
    for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
    {
        m_additionalScoreBasedParameterValues[i].m_words[0] = 0;
        m_additionalScoreBasedParameterValues[i].m_words[1] = 0;
        m_additionalScoreBasedParameterValues[i].m_words[2] = 0;
    }
}

NexSessionSearchCriteria::~NexSessionSearchCriteria()
{
}

nn::pia::common::Result NexSessionSearchCriteria::SetResultRange(unsigned int offset, unsigned int size)
{
    if (size > cAttributeListSize)
        return 0x10c07;
    return session::SessionSearchCriteria::SetResultRange(offset, size);
}

void NexSessionSearchCriteria::SetMinParticipantNum(unsigned short value)
{
    m_minParticipantRangeMax = value;
    m_minParticipantRangeMin = 0xffff;
    m_conditionMask |= 1;
}

void NexSessionSearchCriteria::SetMaxParticipantNum(unsigned short value)
{
    m_maxParticipantRangeMax = value;
    m_maxParticipantRangeMin = 0xffff;
    m_conditionMask |= 2;
}

void NexSessionSearchCriteria::SetGameMode(unsigned int value)
{
    m_gameMode = value;
    m_conditionMask |= 0x10;
}

unsigned int NexSessionSearchCriteria::IsSetSessionMatchmakeKeyword() const
{
    return (m_conditionMask >> 23) & 1;
}

nn::pia::common::Result NexSessionSearchCriteria::SetAttribute(unsigned int index, unsigned int value)
{
    if (index >= cAttributeNum)
        return 0x10c07;
    if (IsSetSessionMatchmakeKeyword() != 0)
        return 0x10c08;

    m_attributeList[index][0] = value;
    m_attributeListSize[index] = 1;
    m_attributeRangeUsed[index] = false;
    m_conditionMask |= 1u << (index + 6);
    return 0;
}

nn::pia::common::Result NexSessionSearchCriteria::SetAttributeRange(
    unsigned int index, unsigned int minimum, unsigned int maximum)
{
    if (index >= cAttributeNum || maximum < minimum)
        return 0x10c07;
    if (IsSetSessionMatchmakeKeyword() != 0)
        return 0x10c08;

    m_attributeRangeMin[index] = minimum;
    m_attributeRangeMax[index] = maximum;
    m_attributeRangeUsed[index] = true;
    m_conditionMask |= 1u << (index + 6);
    return 0;
}

void NexSessionSearchCriteria::SetRandomSessionSelectionMethod(SelectionMethod method)
{
    m_randomSessionSelectionMethod = method;
    m_conditionMask |= 0x4000;
}

void NexSessionSearchCriteria::Copy(const NexSessionSearchCriteria& other)
{
    session::SessionSearchCriteria::Copy(other);
    m_gameMode = other.m_gameMode;
    m_sessionType = other.m_sessionType;
    for (unsigned int i = 0; i < cAttributeNum; ++i)
    {
        for (unsigned int j = 0; j < cAttributeListSize; ++j)
            m_attributeList[i][j] = other.m_attributeList[i][j];
        m_attributeListSize[i] = other.m_attributeListSize[i];
        m_attributeRangeMin[i] = other.m_attributeRangeMin[i];
        m_attributeRangeMax[i] = other.m_attributeRangeMax[i];
        m_attributeRangeUsed[i] = other.m_attributeRangeUsed[i];
    }
    m_excludeNonHostPrincipalId = other.m_excludeNonHostPrincipalId;
    m_excludeUserPasswordSet = other.m_excludeUserPasswordSet;
    m_randomSessionSelectionMethod = other.m_randomSessionSelectionMethod;
    m_scoreBasedSettingIndex = other.m_scoreBasedSettingIndex;
    m_scoreBasedRatingValue = other.m_scoreBasedRatingValue;
    m_scoreBasedDisconnectionRate = other.m_scoreBasedDisconnectionRate;
    m_scoreBasedViolationRate = other.m_scoreBasedViolationRate;
    m_countryCode = other.m_countryCode;
    m_scoreBasedGeoIpUsed = other.m_scoreBasedGeoIpUsed;
    m_overrideIpAddress = other.m_overrideIpAddress;
    m_communityId = other.m_communityId;
    m_minParticipantRangeMin = other.m_minParticipantRangeMin;
    m_minParticipantRangeMax = other.m_minParticipantRangeMax;
    m_maxParticipantRangeMin = other.m_maxParticipantRangeMin;
    m_maxParticipantRangeMax = other.m_maxParticipantRangeMax;
    m_openedOnly = other.m_openedOnly;
    m_vacantOnly = other.m_vacantOnly;
    for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
    {
        m_additionalScoreBasedParameterValueKey[i].Copy(other.m_additionalScoreBasedParameterValueKey[i]);
        m_additionalScoreBasedParameterValues[i].m_words[0] = other.m_additionalScoreBasedParameterValues[i].m_words[0];
        m_additionalScoreBasedParameterValues[i].m_words[1] = other.m_additionalScoreBasedParameterValues[i].m_words[1];
        m_additionalScoreBasedParameterValues[i].m_words[2] = other.m_additionalScoreBasedParameterValues[i].m_words[2];
        m_additionalScoreBasedParameterValues[i].m_words[3] = other.m_additionalScoreBasedParameterValues[i].m_words[3];
    }
    m_additionalScoreBasedParameterStringKey.Copy(other.m_additionalScoreBasedParameterStringKey);
    m_additionalScoreBasedParameterString.Copy(other.m_additionalScoreBasedParameterString);
    m_sessionMatchmakeKeyword.Copy(other.m_sessionMatchmakeKeyword);
    m_conditionMask = other.m_conditionMask;
}

void NexSessionSearchCriteria::Trace(unsigned long long) const
{
}

void NexSessionSearchCriteria::Reset()
{
    session::SessionSearchCriteria::Reset();
    m_resultRangeOffset = static_cast<unsigned int>(-1);
    m_resultRangeSize = 100;
    m_gameMode = 0;
    m_sessionType = cSessionType_Normal;
    m_excludeNonHostPrincipalId = true;
    m_excludeUserPasswordSet = true;
    m_randomSessionSelectionMethod = cSelectionMethod_0;
    m_scoreBasedSettingIndex = 0;
    m_scoreBasedRatingValue = 0;
    m_scoreBasedDisconnectionRate = 0;
    m_scoreBasedViolationRate = 0;
    m_countryCode = 0;
    m_scoreBasedGeoIpUsed = false;
    m_overrideIpAddress.m_address = 0;
    m_overrideIpAddress.m_port = 0;
    m_communityId = 0;
    m_minParticipantRangeMin = 0xffff;
    m_minParticipantRangeMax = 0xffff;
    m_maxParticipantRangeMin = 0xffff;
    m_maxParticipantRangeMax = 0xffff;
    m_openedOnly = true;
    m_vacantOnly = true;
    m_conditionMask = 0;
    for (unsigned int i = 0; i < cAttributeNum; ++i)
    {
        m_attributeListSize[i] = 0;
        m_attributeRangeMin[i] = 0;
        m_attributeRangeMax[i] = 0;
        m_attributeRangeUsed[i] = false;
        for (unsigned int j = 0; j < cAttributeListSize; ++j)
            m_attributeList[i][j] = 0;
    }
    for (unsigned int i = 0; i < cAttributeNum / 2; ++i)
    {
        m_additionalScoreBasedParameterValueKey[i].Clear();
        m_additionalScoreBasedParameterValues[i].m_words[0] = 0;
        m_additionalScoreBasedParameterValues[i].m_words[1] = 0;
        m_additionalScoreBasedParameterValues[i].m_words[2] = 0;
    }
    m_additionalScoreBasedParameterStringKey.Clear();
    m_additionalScoreBasedParameterString.Clear();
    m_sessionMatchmakeKeyword.Clear();
}

unsigned int NexSessionSearchCriteria::IsValid() const
{
    return (m_conditionMask >> 4) & 1;
}

}}}
