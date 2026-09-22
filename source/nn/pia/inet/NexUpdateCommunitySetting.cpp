#include "nn/pia/inet/NexUpdateCommunitySetting.hpp"
#include <cstring>

namespace nn { namespace pia { namespace inet {

NexUpdateCommunitySetting::~NexUpdateCommunitySetting() {}

void NexUpdateCommunitySetting::Reset()
{
    communityId = 0;
    minParticipantNum = static_cast<unsigned short>(-1);
    maxParticipantNum = static_cast<unsigned short>(-1);
    for (unsigned int& attribute : attributes) attribute = 0;
    description.Clear();
    communityUserPassword.Clear();
    applicationDataSize = 0;
    std::memset(applicationData, 0, sizeof(applicationData));
    communityIdSet = false;
    minParticipantNumSet = false;
    maxParticipantNumSet = false;
    attributesSet = false;
    descriptionSet = false;
    communityUserPasswordSet = false;
    participationStartDateSet = false;
    participationEndDateSet = false;
}

void NexUpdateCommunitySetting::Trace(unsigned long long) const {}
unsigned int NexUpdateCommunitySetting::GetCommunityId() const { return communityId; }
unsigned short NexUpdateCommunitySetting::GetMinParticipantNum() const { return minParticipantNum; }
unsigned short NexUpdateCommunitySetting::GetMaxParticipantNum() const { return maxParticipantNum; }
unsigned int NexUpdateCommunitySetting::GetAttribute(unsigned int index) const { return index < 6 ? attributes[index] : 0; }
const common::String& NexUpdateCommunitySetting::GetDescription() const { return description; }
const common::String& NexUpdateCommunitySetting::GetCommunityUserPassword() const { return communityUserPassword; }
bool NexUpdateCommunitySetting::IsSetCommunityUserPassword() const { return communityUserPasswordSet; }
const common::DateTime& NexUpdateCommunitySetting::GetParticipaitonStartDate() const { return participationStartDate; }
const common::DateTime& NexUpdateCommunitySetting::GetParticipaitonEndDate() const { return participationEndDate; }
void NexUpdateCommunitySetting::CopyApplicationDataToVector(void* output) const { if (output != 0 && applicationDataSize != 0) std::memcpy(output, applicationData, applicationDataSize); }
bool NexUpdateCommunitySetting::IsValid() const { return true; }

}}}
