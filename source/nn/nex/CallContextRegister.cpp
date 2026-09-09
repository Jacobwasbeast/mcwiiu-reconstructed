#include "nn/nex/CallContextRegister.hpp"

#include <cwchar>

#include "nn/nex/Core.hpp"
#include "nn/nex/Job.hpp"
#include "nn/nex/Scheduler.hpp"

namespace nn
{
namespace nex
{

CallContextRegister::CallContextRegister()
    : SystemComponent(String(L"CallContextRegister"))
    , m_callContexts()
    , m_associatedIds()
    , m_uiNextCallId(1)
    , m_pCheckExpiredCallsJob(0)
{
}

const wchar_t* CallContextRegister::GetType() const
{
    return L"CallContextRegister";
}

bool CallContextRegister::IsAKindOf(const wchar_t* pType) const
{
    if (std::wcscmp(L"CallContextRegister", pType) == 0)
        return true;

    return SystemComponent::IsAKindOf(pType);
}

void CallContextRegister::EnforceDeclareSysComponentMacro()
{
}

bool CallContextRegister::BeginInitialization()
{
    Start();
    return true;
}

bool CallContextRegister::BeginTermination()
{
    Stop();
    return true;
}

unsigned int CallContextRegister::Register(CallContext* pCallContext)
{
    unsigned int uiCallId = m_uiNextCallId++;
    if (m_uiNextCallId == 0)
        m_uiNextCallId = 1;

    m_callContexts[uiCallId] = pCallContext;
    return uiCallId;
}

void CallContextRegister::Unregister(unsigned int uiCallId)
{
    m_callContexts.erase(m_callContexts.find(uiCallId));
    m_associatedIds.erase(m_associatedIds.lower_bound(uiCallId),
        m_associatedIds.upper_bound(uiCallId));
}

CallContext* CallContextRegister::GetCallContext(unsigned int uiCallId)
{
    CallContextMap::iterator callContext = m_callContexts.find(uiCallId);
    if (callContext == m_callContexts.end())
        return 0;

    return callContext->second;
}

void CallContextRegister::AssociateCallContext(unsigned int uiCallId, unsigned int uiAssociatedId)
{
    m_associatedIds.insert(std::make_pair(uiCallId, uiAssociatedId));
}

void CallContextRegister::GetAssociatedId(unsigned int uiCallId,
    qVector<unsigned int>* pAssociatedIds)
{
    pAssociatedIds->clear();

    AssociatedIdMap::iterator associatedId = m_associatedIds.lower_bound(uiCallId);
    AssociatedIdMap::iterator last = m_associatedIds.upper_bound(uiCallId);
    for (; associatedId != last; ++associatedId)
        pAssociatedIds->push_back(associatedId->second);
}

void CallContextRegister::CheckExpiredCalls(int)
{
    CancelExpiredCalls();
}

void CallContextRegister::Stop()
{
    if (!Core::GetInstance()->GetScheduler()->Cancel(m_pCheckExpiredCallsJob))
        m_pCheckExpiredCallsJob->SetToCancel();

    m_pCheckExpiredCallsJob->ReleaseRef();
    m_pCheckExpiredCallsJob = 0;
}

}
}
