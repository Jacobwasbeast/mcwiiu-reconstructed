#include "nn/pia/inet/NexCommunitySearchCriteriaParticipant.hpp"

namespace nn { namespace pia { namespace inet {

NexCommunitySearchCriteriaParticipant::~NexCommunitySearchCriteriaParticipant()
{
}

unsigned long long NexCommunitySearchCriteriaParticipant::GetParticipantPrincipalId() const
{
    return m_participantPrincipalId;
}

Result NexCommunitySearchCriteriaParticipant::SetResultRange(unsigned int offset, unsigned int size)
{
    return size <= 100 ? session::CommunitySearchCriteria::SetResultRange(offset, size) : Result(0x10c07);
}

void NexCommunitySearchCriteriaParticipant::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_participantPrincipalId = 0;
}

}}}
