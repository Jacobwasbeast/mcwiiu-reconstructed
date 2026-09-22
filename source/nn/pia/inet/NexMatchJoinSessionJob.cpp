#include "nn/pia/inet/NexMatchJoinSessionJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchJoinSessionJob::NexMatchJoinSessionJob() : m_state(0)
{}

NexMatchJoinSessionJob::~NexMatchJoinSessionJob()
{}

unsigned int NexMatchJoinSessionJob::CleanupForRetryJoinMesh()
{
    return m_state;

}
void NexMatchJoinSessionJob::CleanupImpl()
{
    m_state = 0;

}
unsigned int NexMatchJoinSessionJob::GetStationConnectionInfo()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::JoinMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::LeaveJoinedMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::LeaveMatchmakeSession()
{
    return m_state;

}
unsigned long long NexMatchJoinSessionJob::NoticeOwnerChangeEvent(unsigned long long p0, unsigned long long p1, unsigned long long p2)
{
    return m_state;

}
void NexMatchJoinSessionJob::NoticeUnregisterEvent(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchJoinSessionJob::SetNextStepAtFailedJoinMesh(unsigned long long p0)
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::SetNextStepAtMeshCleanup()
{
    return m_state;

}
void NexMatchJoinSessionJob::SetNextStepAtWaitJoinMesh()
{
    m_state = 0;

}
unsigned int NexMatchJoinSessionJob::SetNextStepAtWaitLeaveMesh()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::StartNatSession()
{
    return m_state;

}
unsigned long long NexMatchJoinSessionJob::StartupImpl(unsigned long long p0, unsigned long long p1)
{
    return m_state;

}
void NexMatchJoinSessionJob::Trace(unsigned long long p0)
{
    m_state = 0;

}
unsigned int NexMatchJoinSessionJob::WaitForRetryJoinMesh()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitGetStationConnectionInfo()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitJoinMatchmake()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitLeaveBufferMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitLeaveCurrentMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitLeaveJoinedMatchmakeSession()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitNotification()
{
    return m_state;

}
unsigned int NexMatchJoinSessionJob::WaitStartNatSession()
{
    return m_state;

}

}}}
