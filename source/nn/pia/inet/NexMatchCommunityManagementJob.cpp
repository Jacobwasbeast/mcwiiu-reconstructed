#include "nn/pia/inet/NexMatchCommunityManagementJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchCommunityManagementJob::NexMatchCommunityManagementJob() : m_state(0)
{}

NexMatchCommunityManagementJob::~NexMatchCommunityManagementJob()
{}

unsigned long long NexMatchCommunityManagementJob::BrowseCommunityStartupImpl(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
void NexMatchCommunityManagementJob::Cleanup()
{
    m_state = 0;

}
unsigned int NexMatchCommunityManagementJob::CreateCommunity()
{
    return m_state;

}
unsigned long long NexMatchCommunityManagementJob::CreateCommunityStartupImpl(unsigned long long p0)
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::DestroyCommunity()
{
    return m_state;

}
unsigned long long NexMatchCommunityManagementJob::DestroyCommunityStartupImpl(unsigned long long p0, unsigned long long p1)
{
    return m_state;

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
unsigned long long NexMatchCommunityManagementJob::JoinCommunityStartupImpl(unsigned long long p0)
{
    return m_state;

}
unsigned int NexMatchCommunityManagementJob::LeaveCommunity()
{
    return m_state;

}
unsigned long long NexMatchCommunityManagementJob::LeaveCommunityStartupImpl(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
void NexMatchCommunityManagementJob::Trace(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchCommunityManagementJob::UpdateCommunity()
{
    return m_state;

}
unsigned long long NexMatchCommunityManagementJob::UpdateCommunityStartupImpl(unsigned long long p0)
{
    return m_state;

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
