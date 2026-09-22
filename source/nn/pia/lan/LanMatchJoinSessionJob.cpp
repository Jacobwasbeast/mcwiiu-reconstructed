#include "nn/pia/lan/LanMatchJoinSessionJob.hpp"

namespace nn { namespace pia { namespace lan {

LanMatchJoinSessionJob::LanMatchJoinSessionJob()
{
}

LanMatchJoinSessionJob::~LanMatchJoinSessionJob()
{
}

uint32_t LanMatchJoinSessionJob::JoinMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::WaitJoinMatchmake()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::GetStationConnectionInfo()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::WaitGetStationConnectionInfo()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::LeaveMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::WaitLeaveMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::WaitForRetryJoinMatchmakeSession()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::WaitRequestSessionInfo()
{
    return 0;
}

void LanMatchJoinSessionJob::Cleanup()
{
}

uint64_t LanMatchJoinSessionJob::NoticeOwnerChangeEvent(int32_t a1, uint32_t a2, uint32_t a3)
{
    return 0;
}

void LanMatchJoinSessionJob::Trace(uint64_t a1)
{
}

void LanMatchJoinSessionJob::SetNextStepAtWaitJoinMesh()
{
}

uint32_t LanMatchJoinSessionJob::SetNextStepAtFailedJoinMesh(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::SetNextStepAtWaitLeaveMesh()
{
    return 0;
}

uint32_t LanMatchJoinSessionJob::SetNextStepAtMeshCleanup()
{
    return 0;
}

uint64_t LanMatchJoinSessionJob::StartupImpl(uint32_t a1, int32_t a2)
{
    return 0;
}

}}}
