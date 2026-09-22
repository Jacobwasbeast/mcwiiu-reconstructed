#include "nn/pia/inet/NexCreateCommunitySetting.hpp"
#include <cstring>
#include <vector>

namespace nn { namespace pia { namespace inet {

NexCreateCommunitySetting::~NexCreateCommunitySetting() {}
const common::DateTime* NexCreateCommunitySetting::GetParticipaitonStartDate() const { return reinterpret_cast<const common::DateTime*>(m_startDate); }
const common::DateTime* NexCreateCommunitySetting::GetParticipaitonEndDate() const { return reinterpret_cast<const common::DateTime*>(m_endDate); }
const common::String* NexCreateCommunitySetting::GetDescription() const { return &m_description; }
unsigned int NexCreateCommunitySetting::GetAttribute(unsigned int index) const { return index < 6 ? m_attributes[index] : 0; }
bool NexCreateCommunitySetting::IsSetAttribute(unsigned int index) const { return index < 6 && ((m_attributeFlags >> (index + 3)) & 1u) != 0; }
bool NexCreateCommunitySetting::CopyApplicationDataToVector(void* output) const
{
    if (output == 0)
        return false;
    static_cast<std::vector<unsigned char>*>(output)->assign(m_applicationData, m_applicationData + m_applicationDataSize);
    return true;
}
const common::String* NexCreateCommunitySetting::GetCommunityUserPassword() const { return &m_password; }

void NexCreateCommunitySetting::Reset()
{
    std::memset(m_startDate, 0, sizeof(m_startDate)); std::memset(m_endDate, 0, sizeof(m_endDate)); m_applicationDataSize = 0; m_attributeFlags = 0; std::memset(m_attributes, 0, sizeof(m_attributes)); std::memset(m_applicationData, 0, sizeof(m_applicationData)); m_description.Clear(); m_password.Clear();
}

}}}
