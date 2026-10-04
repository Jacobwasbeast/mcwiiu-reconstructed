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

void LanMatchJoinSessionJob::NoticeOwnerChangeEvent(unsigned int, unsigned long long)
{
}

void LanMatchJoinSessionJob::Trace(unsigned long long) const
{
}

void LanMatchJoinSessionJob::SetNextStepAtWaitJoinMesh()
{
}

uint32_t LanMatchJoinSessionJob::SetNextStepAtFailedJoinMesh(Result)
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

Result LanMatchJoinSessionJob::StartupImpl(const session::JoinSessionSetting*)
{
    return 0;
}

}}}
