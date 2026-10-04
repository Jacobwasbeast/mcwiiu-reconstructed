#include "nn/pia/inet/NexCommunitySearchCriteriaCommunityId.hpp"

namespace nn { namespace pia { namespace inet {

NexCommunitySearchCriteriaCommunityId::~NexCommunitySearchCriteriaCommunityId()
{
}

Result NexCommunitySearchCriteriaCommunityId::SetResultRange(unsigned int offset, unsigned int size)
{
    return size <= 100 ? session::CommunitySearchCriteria::SetResultRange(offset, size) : Result(0x10c07);
}

const unsigned int* NexCommunitySearchCriteriaCommunityId::GetCommunityIdList() const
{
    return m_communityIds;
}

unsigned int NexCommunitySearchCriteriaCommunityId::GetCommunityIdListSize() const
{
    return m_communityIdCount;
}

void NexCommunitySearchCriteriaCommunityId::Reset()
{
    session::CommunitySearchCriteria::Reset();
    m_communityIdCount = 0;
    for (unsigned int index = 0; index < 100; ++index)
        m_communityIds[index] = 0;
}

}}}
