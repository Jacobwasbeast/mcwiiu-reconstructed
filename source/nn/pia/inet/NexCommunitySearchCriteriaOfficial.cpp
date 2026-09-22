#include "nn/pia/inet/NexCommunitySearchCriteriaOfficial.hpp"

namespace nn { namespace pia { namespace inet {

NexCommunitySearchCriteriaOfficial::~NexCommunitySearchCriteriaOfficial()
{
}

Result NexCommunitySearchCriteriaOfficial::SetResultRange(unsigned int offset, unsigned int size)
{
    return size <= 100 ? session::CommunitySearchCriteria::SetResultRange(offset, size) : 0x10c07;
}

bool NexCommunitySearchCriteriaOfficial::IsAvailableCommunityOnly() const
{
    return m_availableCommunityOnly;
}

void NexCommunitySearchCriteriaOfficial::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_availableCommunityOnly = true;
}

}}}
