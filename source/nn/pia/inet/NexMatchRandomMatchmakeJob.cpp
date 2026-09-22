#include "nn/pia/inet/NexMatchRandomMatchmakeJob.hpp"

namespace nn { namespace pia { namespace inet {

uint32_t NexMatchRandomMatchmakeJob::LeaveJoinedMatchmakeSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitLeaveJoinedMatchmakeSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::RandomMatchmake()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitRandomMatchmake()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitNotification()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::StartNatSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitStartNatSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::LeaveMatchmakeSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitLeaveCurrentMatchmakeSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitLeaveBufferMatchmakeSession()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::ChangeParticipation()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitChangeParticipation()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::CleanupForRetryJoinMesh()
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::WaitForRetryJoinMesh()
{
    return static_cast<uint32_t>(state_);
}

uint64_t NexMatchRandomMatchmakeJob::NoticeOwnerChangeEvent(int32_t a1, uint32_t a2, uint32_t a3)
{
    return static_cast<uint64_t>(state_);
}

void NexMatchRandomMatchmakeJob::NoticeUnregisterEvent(uint32_t a1)
{
    state_ += 1;
}

void NexMatchRandomMatchmakeJob::Trace(uint64_t a1)
{
    state_ += 1;
}

uint64_t NexMatchRandomMatchmakeJob::StartupImpl(uint32_t a1, uint32_t a2)
{
    return static_cast<uint64_t>(state_);
}

void NexMatchRandomMatchmakeJob::CleanupImpl()
{
    state_ += 1;
}

void NexMatchRandomMatchmakeJob::SetNextStepAtWaitCreateMesh()
{
    state_ += 1;
}

void NexMatchRandomMatchmakeJob::SetNextStepAtWaitJoinMesh()
{
    state_ += 1;
}

uint32_t NexMatchRandomMatchmakeJob::SetNextStepAtFailedCreateMesh(uint32_t a1, uint32_t a2, uint32_t a3)
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::SetNextStepAtFailedJoinMesh(nn_pia_Result* a1)
{
    return static_cast<uint32_t>(state_);
}

uint32_t NexMatchRandomMatchmakeJob::SetNextStepAtWaitLeaveMesh()
{
    return static_cast<uint32_t>(state_);
}

}}}
