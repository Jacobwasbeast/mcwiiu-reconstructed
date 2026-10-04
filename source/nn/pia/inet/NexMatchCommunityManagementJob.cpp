#include "nn/pia/inet/NexMatchCommunityManagementJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchCommunityManagementJob::NexMatchCommunityManagementJob() : m_state(0)
{}

NexMatchCommunityManagementJob::~NexMatchCommunityManagementJob()
{}

Result NexMatchCommunityManagementJob::BrowseCommunityStartupImpl(const session::CommunitySearchCriteria* searchCriteria)
{
    return Result(m_state);

}
void NexMatchCommunityManagementJob::Cleanup()
{
    m_state = 0;

}
unsigned int NexMatchCommunityManagementJob::CreateCommunity()
{
    return m_state;

}
Result NexMatchCommunityManagementJob::CreateCommunityStartupImpl(const session::CreateCommunitySetting* setting)
{
    return Result(m_state);

}
unsigned int NexMatchCommunityManagementJob::DestroyCommunity()
{
    return m_state;

}
Result NexMatchCommunityManagementJob::DestroyCommunityStartupImpl(unsigned int communityId)
{
    return Result(m_state);

}
unsigned int NexMatchCommunityManagementJob::FindCommunityByCommunityId()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::FindCommunityByOwner()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::FindCommunityByParticipant()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::FindOfficialCommunity()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::JoinCommunity()
{
    return m_state;

}
Result NexMatchCommunityManagementJob::JoinCommunityStartupImpl(const session::JoinCommunitySetting* setting)
{
    return Result(m_state);

}
unsigned int NexMatchCommunityManagementJob::LeaveCommunity()
{
    return m_state;

}
Result NexMatchCommunityManagementJob::LeaveCommunityStartupImpl(unsigned int communityId)
{
    return Result(m_state);

}
void NexMatchCommunityManagementJob::Trace(unsigned long long p0) const
{

}
unsigned int NexMatchCommunityManagementJob::UpdateCommunity()
{
    return m_state;

}
Result NexMatchCommunityManagementJob::UpdateCommunityStartupImpl(const session::UpdateCommunitySetting* setting)
{
    return Result(m_state);

}
unsigned int NexMatchCommunityManagementJob::WaitCreateCommunity()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitDestroyCommunity()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitFindCommunityByCommunityId()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitFindCommunityByOwner()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitFindCommunityByParticipant()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitFindOfficialCommunity()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitJoinCommunity()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitLeaveCommunity()
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::WaitUpdateCommunity()
{
    return m_state;

}

}}}
