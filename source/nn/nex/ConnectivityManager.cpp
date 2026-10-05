#include "nn/nex/ConnectivityManager.hpp"

#include "nn/nex/InstanceTable.hpp"

#if defined(RECON_NATIVE)
#include "nn/nex/CallContext.hpp"
#include "nn/nex/CallContextRegister.hpp"
#include "nn/nex/Core.hpp"
#include "nn/nex/Scheduler.hpp"
#endif

namespace nn
{
namespace nex
{

ConnectivityManager* ConnectivityManager::GetInstance()
{
    return GetInstanceWithoutNetworkCheck();
}

ConnectivityManager* ConnectivityManager::GetInstanceWithoutNetworkCheck()
{
    unsigned int contextId = g_usingInstantiationContext;
    if (g_usingInstantiationContext)
        contextId = g_instantiationContextVariable->GetRef();

    return static_cast<ConnectivityManager*>(g_instanceTable.FindInstance(11, contextId));
}

bool ConnectivityManager::RegisterConnectivityTester(ConnectivityTester* connectivityTester)
{
    if (m_connectivityTester != 0)
        return false;

    m_connectivityTester = connectivityTester;
    return true;
}

bool ConnectivityManager::UnregisterConnectivityTester()
{
    if (m_connectivityTester == 0)
        return false;

    m_connectivityTester = 0;
    return true;
}

#if defined(RECON_NATIVE)
void ConnectivityManager::OnStartNATSession(CallContext* callContext)
{
    m_natSessionStarted = false;

    Core* core = Core::GetInstance();
    if (core == 0 || core->GetScheduler() == 0 || core->GetCallContextRegister() == 0)
    {
        return;
    }

    CriticalSection& criticalSection = core->GetScheduler()->GetCriticalSection();
    ScopedCS lock(&criticalSection);

    CallContext* pendingContext = core->GetCallContextRegister()->GetCallContext(m_callContextId);
    m_callContextId = 0;
    if (pendingContext != 0 && pendingContext->GetState() == CallContext::CallInProgress)
    {
        if (callContext != 0 && callContext->GetState() == CallContext::CallFailed)
        {
            pendingContext->SignalFailure(callContext->GetResult());
        }
        else
        {
            pendingContext->SignalFailure(qResult(static_cast<int>(0x80010001u)));
        }
    }

}
#endif

}
}
