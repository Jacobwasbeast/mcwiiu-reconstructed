#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_COMMUNITY_INFO_HPP

#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/String.hpp"

namespace nn { namespace pia { namespace inet {

class NexCommunityInfo
{
public:
    NexCommunityInfo();
    virtual ~NexCommunityInfo();
    void Clear();
    unsigned int GetCommunityId() const;
    bool IsOfficialCommunity() const;
    bool IsRestrictedByPassword() const;
    const common::DateTime& GetParticipationStartDate();
    const common::DateTime& GetParticipationEndDate();
    unsigned int GetRelatedSessionCount() const;
    bool GetAttribute(unsigned char index, unsigned int* value) const;
    unsigned long long GetOwnerPrincipalId() const;
    unsigned short GetCurrentParticipantNum() const;
    unsigned short GetMaxParticipantNum() const;
    unsigned short GetMinParticipantNum() const;
    const common::String& GetDescription() const;
    bool GetApplicationData(void* buffer, unsigned int size);
    bool GetApplicationDataSize(unsigned int* size);
    void SetCommunityId(unsigned int communityId);
    virtual void SetOfficialCommunity(bool official);
    virtual void SetRestrictedByPassword(bool restricted);
    virtual void SetParticipationStartTime(const common::DateTime& time);
    virtual void SetParticipationEndTime(const common::DateTime& time);
    void SetRelatedSessionCount(unsigned int count);
    virtual void SetAttribute(unsigned char index, unsigned int value);
    void SetOwnerPrincipalId(unsigned long long principalId);
    void SetCurrentParticipantNum(unsigned short count);
    void SetMinParticipantNum(unsigned short count);
    void SetMaxParticipantNum(unsigned short count);
    bool SetDescription(const common::String& description);
    bool SetApplicationData(const void* data, unsigned int size);

private:
    unsigned int m_communityId;
    bool m_official;
    bool m_restricted;
    common::DateTime m_participationStartDate;
    common::DateTime m_participationEndDate;
    unsigned int m_relatedSessionCount;
    unsigned int m_attributes[6];
    unsigned long long m_ownerPrincipalId;
    unsigned short m_currentParticipantNum;
    unsigned short m_maxParticipantNum;
    unsigned short m_minParticipantNum;
    common::String m_description;
    unsigned char m_applicationData[512];
    unsigned int m_applicationDataSize;
};

}}}

#endif
