#include "nn/pia/inet/NexCommunitySearchCriteriaOwner.hpp"

namespace nn { namespace pia { namespace inet {

NexCommunitySearchCriteriaOwner::~NexCommunitySearchCriteriaOwner()
{
}

unsigned long long NexCommunitySearchCriteriaOwner::GetOwnerPrincipalId() const
{
    return m_ownerPrincipalId;
}

Result NexCommunitySearchCriteriaOwner::SetResultRange(unsigned int offset, unsigned int size)
{
    return size <= 100 ? session::CommunitySearchCriteria::SetResultRange(offset, size) : 0x10c07;
}

void NexCommunitySearchCriteriaOwner::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_ownerPrincipalId = 0;
}

}}}
