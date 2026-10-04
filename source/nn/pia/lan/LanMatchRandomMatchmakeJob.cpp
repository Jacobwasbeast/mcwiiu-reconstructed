#include "nn/pia/lan/LanMatchRandomMatchmakeJob.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchRandomMatchmakeJob::LanMatchRandomMatchmakeJob()
{
}

LanMatchRandomMatchmakeJob::~LanMatchRandomMatchmakeJob()
{
}

uint32_t LanMatchRandomMatchmakeJob::RandomMatchmake()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::WaitRandomMatchmake()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::UnregisterGathering()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::WaitUnregisterGathering()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::LeaveMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::WaitLeaveMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::WaitForRetryRandomMatchmake()
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::WaitRequestSessionInfo()
{
    return 0;
}

void LanMatchRandomMatchmakeJob::NoticeOwnerChangeEvent(uint32_t ownerId, uint64_t principalId)
{
}

void LanMatchRandomMatchmakeJob::Cleanup()
{
}

void LanMatchRandomMatchmakeJob::Trace(uint64_t value) const
{
}

Result LanMatchRandomMatchmakeJob::StartupImpl(const session::JoinRandomSessionSetting* setting)
{
    return 0;
}

void LanMatchRandomMatchmakeJob::SetNextStepAtWaitCreateMesh()
{
}

void LanMatchRandomMatchmakeJob::SetNextStepAtWaitJoinMesh()
{
}

uint32_t LanMatchRandomMatchmakeJob::SetNextStepAtFailedCreateMesh(Result result)
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::SetNextStepAtFailedJoinMesh(Result result)
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::SetNextStepAtWaitLeaveMesh()
{
    return 0;
}

}}}
