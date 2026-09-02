#include "game/core/NS_JobConnectEndPoint_CPP.hpp"

namespace NS_JobConnectEndPoint_CPP
{

void ConnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult resultConnect, const nn::nex::UserContext* pUserContext)
{
    nn::nex::Scheduler* pScheduler = nn::nex::Scheduler::GetInstance();
    nn::nex::CriticalSection& csScheduler = pScheduler->GetCriticalSection();

    csScheduler.Enter();

    nn::nex::JobConnectEndPoint* pJob = static_cast<nn::nex::JobConnectEndPoint*>(pUserContext->GetPointer());
    if (pJob->GetState() == nn::nex::Job::SUSPENDED)
    {
        pJob->ReportConnectResult(resultConnect);
    }

    csScheduler.Leave();
}

void DisconnectCallback(nn::nex::EndPoint* pEndPoint, nn::nex::qResult resultDisconnect, const nn::nex::UserContext* pUserContext)
{
    nn::nex::Scheduler* pScheduler = nn::nex::Scheduler::GetInstance();
    nn::nex::CriticalSection& csScheduler = pScheduler->GetCriticalSection();

    csScheduler.Enter();

    nn::nex::JobConnectEndPoint* pJob = static_cast<nn::nex::JobConnectEndPoint*>(pUserContext->GetPointer());
    pJob->SetToReady();
    pJob->ReleaseRef();

    csScheduler.Leave();
}

}
