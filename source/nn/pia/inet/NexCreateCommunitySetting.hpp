#ifndef LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_COMMUNITY_SETTING_HPP
#define LEGACY_CONSOLE_WIIU_NN_PIA_INET_NEX_CREATE_COMMUNITY_SETTING_HPP

#include "nn/pia.h"
#include "nn/pia/common/DateTime.hpp"
#include "nn/pia/String.hpp"
#include "nn/pia/session/CreateCommunitySetting.hpp"

namespace nn { namespace pia { namespace inet {

class NexCreateCommunitySetting : public session::CreateCommunitySetting
{
public:
    NexCreateCommunitySetting();
    NexCreateCommunitySetting(const NexCreateCommunitySetting& other);
    ~NexCreateCommunitySetting();
    NexCreateCommunitySetting& operator=(const NexCreateCommunitySetting& other);
    void Copy(const NexCreateCommunitySetting& other);
    const common::DateTime* GetParticipaitonStartDate() const;
    const common::DateTime* GetParticipaitonEndDate() const;
    const common::String* GetDescription() const;
    unsigned int GetAttribute(unsigned int index) const;
    bool IsSetAttribute(unsigned int index) const;
    bool CopyApplicationDataToVector(void* output) const;
    const common::String* GetCommunityUserPassword() const;
    void SetParticipationStartDate(const common::DateTime& value);
    bool IsSetParticipationStartDate() const;
    void SetParticipationEndDate(const common::DateTime& value);
    bool IsSetParticipationEndDate() const;
    void SetDescription(const common::String& value);
    bool IsSetDescription() const;
    void SetAttribute(unsigned int index, unsigned int value);
    bool SetApplicationData(const void* data, unsigned int size);
    bool GetApplicationData(void* data, unsigned int size) const;
    unsigned int GetApplicationDataSize() const;
    bool IsSetApplicationData() const;
    void SetCommunityUserPassword(const common::String& value);
    bool IsSetCommunityUserPassword() const;
    void Reset();

private:
    unsigned char m_startDate[12];
    unsigned char m_endDate[12];
    unsigned int m_attributes[6];
    common::String m_description;
    common::String m_password;
    unsigned char m_applicationData[512];
    unsigned int m_applicationDataSize;
    unsigned int m_attributeFlags;
};

}}}

#endif
