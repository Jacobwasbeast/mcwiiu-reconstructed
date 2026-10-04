#include "nn/pia/inet/NexMatchLeaveSessionJob.hpp"

namespace nn { namespace pia { namespace inet {

NexMatchLeaveSessionJob::NexMatchLeaveSessionJob() : m_state(0)
{}

NexMatchLeaveSessionJob::~NexMatchLeaveSessionJob()
{}

void NexMatchLeaveSessionJob::ChooseNextStepByCheckingBufferMatchmakeSessions(const common::StepSequenceJob::Step& step)
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::CleanupImpl()
{
    m_state = 0;

}
unsigned int NexMatchLeaveSessionJob::GetMatchmakeSessionOwners()
{
    return m_state;

}
unsigned int NexMatchLeaveSessionJob::GetWaitHostMigrationTimeout() const
{
    return m_state;

}
bool NexMatchLeaveSessionJob::IsNeedSkipLeaveCurrentMatchmakeSession()
{
    return m_state != 0;

}
unsigned int NexMatchLeaveSessionJob::LeaveMatchmakeSessionCompanions()
{
    return m_state;

}
unsigned int NexMatchLeaveSessionJob::MigrateMatchmakeSessionOwner()
{
    return m_state;

}
void NexMatchLeaveSessionJob::SetNextStepAtLeaveCurrentMatchmakeSessionFailure()
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::SetNextStepAtMeshCleanup()
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::SetNextStepAtWaitForcedTerminatingOfJointSessionJob()
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::SetNextStepAtWaitHostMigratedTimeout()
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::SetNextStepAtWaitLeaveBufferMatchmakeSession()
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::SetNextStepAtWaitLeaveCurrentMatchmakeSession()
{
    m_state = 0;

}
void NexMatchLeaveSessionJob::Trace(unsigned long long p0) const
{
}
unsigned int NexMatchLeaveSessionJob::WaitGetMatchmakeSessionOwners()
{
    return m_state;

}
unsigned int NexMatchLeaveSessionJob::WaitLeaveMatchmakeSessionCompanions()
{
    return m_state;

}
unsigned int NexMatchLeaveSessionJob::WaitMigrateMatchmakeSessionOwner()
{
    return m_state;

}

}}}
