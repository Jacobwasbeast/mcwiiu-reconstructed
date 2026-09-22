#include "nn/pia/inet/NexCommunityInfo.hpp"
#include <cstring>

namespace nn { namespace pia { namespace inet {

NexCommunityInfo::NexCommunityInfo()
    : m_communityId(0), m_official(false), m_restricted(false), m_participationStartDate(), m_participationEndDate(),
      m_relatedSessionCount(0), m_attributes(), m_ownerPrincipalId(0), m_currentParticipantNum(0), m_maxParticipantNum(0),
      m_minParticipantNum(0), m_description(""), m_applicationData(), m_applicationDataSize(0)
{
    Clear();
}

NexCommunityInfo::~NexCommunityInfo()
{
}

void NexCommunityInfo::Clear()
{
    m_communityId = 0;
    m_official = false;
    m_restricted = false;
    m_relatedSessionCount = 0;
    m_ownerPrincipalId = 0;
    m_currentParticipantNum = 0;
    m_maxParticipantNum = 0;
    m_minParticipantNum = 0;
    m_applicationDataSize = 0;
    for (unsigned int index = 0; index < 6; ++index)
        m_attributes[index] = 0;
    std::memset(m_applicationData, 0, sizeof(m_applicationData));
    m_description.Clear();
}

unsigned int NexCommunityInfo::GetCommunityId() const { return m_communityId; }
bool NexCommunityInfo::IsOfficialCommunity() const { return m_official; }
bool NexCommunityInfo::IsRestrictedByPassword() const { return m_restricted; }
const common::DateTime& NexCommunityInfo::GetParticipationStartDate() const { return m_participationStartDate; }
const common::DateTime& NexCommunityInfo::GetParticipationEndDate() const { return m_participationEndDate; }
unsigned int NexCommunityInfo::GetRelatedSessionCount() const { return m_relatedSessionCount; }

bool NexCommunityInfo::GetAttribute(unsigned char index, unsigned int* value) const
{
    if (index >= 6 || value == 0)
        return false;
    *value = m_attributes[index];
    return true;
}

unsigned long long NexCommunityInfo::GetOwnerPrincipalId() const { return m_ownerPrincipalId; }
unsigned short NexCommunityInfo::GetCurrentParticipantNum() const { return m_currentParticipantNum; }
unsigned short NexCommunityInfo::GetMaxParticipantNum() const { return m_maxParticipantNum; }
unsigned short NexCommunityInfo::GetMinParticipantNum() const { return m_minParticipantNum; }
const common::String& NexCommunityInfo::GetDescription() const { return m_description; }

bool NexCommunityInfo::GetApplicationData(void* buffer, unsigned int size) const
{
    if (buffer == 0 || size < m_applicationDataSize)
        return false;
    std::memcpy(buffer, m_applicationData, m_applicationDataSize);
    return true;
}

bool NexCommunityInfo::GetApplicationDataSize(unsigned int* size) const
{
    if (size == 0)
        return false;
    *size = m_applicationDataSize;
    return true;
}

void NexCommunityInfo::SetCommunityId(unsigned int communityId) { m_communityId = communityId; }
void NexCommunityInfo::SetOfficialCommunity(bool official) { m_official = official; }
void NexCommunityInfo::SetRestrictedByPassword(bool restricted) { m_restricted = restricted; }
void NexCommunityInfo::SetParticipationStartTime(const common::DateTime& time) { std::memcpy(&m_participationStartDate, &time, sizeof(m_participationStartDate)); }
void NexCommunityInfo::SetParticipationEndTime(const common::DateTime& time) { std::memcpy(&m_participationEndDate, &time, sizeof(m_participationEndDate)); }
void NexCommunityInfo::SetRelatedSessionCount(unsigned int count) { m_relatedSessionCount = count; }

void NexCommunityInfo::SetAttribute(unsigned char index, unsigned int value)
{
    m_attributes[index] = value;
}

void NexCommunityInfo::SetOwnerPrincipalId(unsigned long long principalId) { m_ownerPrincipalId = principalId; }
void NexCommunityInfo::SetCurrentParticipantNum(unsigned short count) { m_currentParticipantNum = count; }
void NexCommunityInfo::SetMinParticipantNum(unsigned short count) { m_minParticipantNum = count; }
void NexCommunityInfo::SetMaxParticipantNum(unsigned short count) { m_maxParticipantNum = count; }
bool NexCommunityInfo::SetDescription(const common::String& description) { return m_description.Copy(description) == 0; }

bool NexCommunityInfo::SetApplicationData(const void* data, unsigned int size)
{
    std::memcpy(m_applicationData, data, size);
    m_applicationDataSize = size;
    return true;
}

}}}
