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

uint64_t LanMatchRandomMatchmakeJob::NoticeOwnerChangeEvent(int32_t a1, uint32_t a2, uint32_t a3)
{
    return 0;
}

void LanMatchRandomMatchmakeJob::Cleanup()
{
}

void LanMatchRandomMatchmakeJob::Trace(uint64_t a1)
{
}

uint64_t LanMatchRandomMatchmakeJob::StartupImpl(uint32_t a1, uint32_t a2)
{
    return 0;
}

void LanMatchRandomMatchmakeJob::SetNextStepAtWaitCreateMesh()
{
}

void LanMatchRandomMatchmakeJob::SetNextStepAtWaitJoinMesh()
{
}

uint32_t LanMatchRandomMatchmakeJob::SetNextStepAtFailedCreateMesh(void* a1)
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::SetNextStepAtFailedJoinMesh(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4)
{
    return 0;
}

uint32_t LanMatchRandomMatchmakeJob::SetNextStepAtWaitLeaveMesh()
{
    return 0;
}

}}}
