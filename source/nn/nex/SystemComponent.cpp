#include "nn/nex/SystemComponent.hpp"

#include <cwchar>

#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SpinTest.hpp"

bool WaitForTerminatedStatePreventBlockCallCallback(
    nn::nex::RootObject* pObject, unsigned int uiTimeout);

namespace nn
{
namespace nex
{

SystemComponent::SystemComponent(const String& rName)
    : m_Name(rName)
    , m_uiState(STATE_CREATED)
    , m_iInUseCount(0)
    , m_pParent(0)
{
}

SystemComponent::~SystemComponent()
{
    SetParent(0);
}

const wchar_t* SystemComponent::GetType() const
{
    return L"SystemComponent";
}

bool SystemComponent::IsAKindOf(const wchar_t* pType) const
{
    return std::wcscmp(L"SystemComponent", pType) == 0;
}

void SystemComponent::StateTransition(_State)
{
}

void SystemComponent::OnInitialize()
{
    TestState();
}

void SystemComponent::OnTerminate()
{
    TestState();
}

bool SystemComponent::BeginInitialization()
{
    return true;
}

bool SystemComponent::EndInitialization()
{
    return true;
}

bool SystemComponent::BeginTermination()
{
    return true;
}

bool SystemComponent::EndTermination()
{
    return true;
}

bool SystemComponent::ValidTransition(_State state)
{
    if (state == STATE_INITIALIZATION_FAILED || state == STATE_TERMINATION_FAILED)
        return true;

    switch (m_uiState)
    {
    case STATE_CREATED:
        return state == STATE_INITIALIZING || state == STATE_TERMINATING || state == STATE_TERMINATED;
    case STATE_INITIALIZING:
        return state == STATE_READY;
    case STATE_READY:
        return state == STATE_IN_USE || state == STATE_TERMINATING;
    case STATE_IN_USE:
        return state == STATE_READY || state == STATE_TERMINATION_PENDING;
    case STATE_TERMINATING:
        return state == STATE_TERMINATED;
    case STATE_TERMINATION_PENDING:
        return m_iInUseCount == 0 && state == STATE_TERMINATING;
    case STATE_TERMINATED:
        return state == STATE_CREATED || state == STATE_INITIALIZING;
    case STATE_INITIALIZATION_FAILED:
    case STATE_TERMINATION_FAILED:
        return true;
    default:
        return false;
    }
}

bool SystemComponent::UseIsAllowed()
{
    return m_uiState == STATE_READY || m_uiState == STATE_IN_USE;
}

SystemComponent::_State SystemComponent::TestState()
{
    return static_cast<_State>(m_uiState);
}

void SystemComponent::DoWork()
{
}

void SystemComponent::SetParent(SystemComponent* pParent)
{
    if (m_pParent != 0)
        m_pParent = 0;

    if (pParent == 0)
        return;

    m_pParent = pParent;
}

bool SystemComponent::SetState(_State state, bool bForce)
{
    if (!bForce && !ValidTransition(state))
        return false;

    StateTransition(state);
    m_uiState = state;
    return true;
}

SystemComponent::_State SystemComponent::Initialize()
{
    OnInitialize();

    bool bInitialized;
    switch (m_uiState)
    {
    case STATE_CREATED:
    case STATE_TERMINATED:
        SetState(STATE_INITIALIZING, false);
        bInitialized = BeginInitialization();
        break;
    case STATE_INITIALIZING:
        bInitialized = EndInitialization();
        break;
    default:
        return static_cast<_State>(m_uiState);
    }

    if (bInitialized)
        SetState(STATE_READY, false);

    return static_cast<_State>(m_uiState);
}

SystemComponent::_State SystemComponent::Terminate()
{
    OnTerminate();

    bool bTerminated;
    switch (m_uiState)
    {
    case STATE_CREATED:
    case STATE_INITIALIZING:
    case STATE_READY:
    case STATE_INITIALIZATION_FAILED:
    case STATE_TERMINATION_FAILED:
        SetState(STATE_TERMINATING, true);
        bTerminated = BeginTermination();
        break;
    case STATE_IN_USE:
        SetState(STATE_TERMINATION_PENDING, false);
        return static_cast<_State>(m_uiState);
    case STATE_TERMINATION_PENDING:
        if (m_iInUseCount != 0)
            return static_cast<_State>(m_uiState);
        SetState(STATE_TERMINATING, false);
        bTerminated = BeginTermination();
        break;
    case STATE_TERMINATING:
        bTerminated = EndTermination();
        break;
    default:
        return static_cast<_State>(m_uiState);
    }

    if (bTerminated)
        SetState(STATE_TERMINATED, false);

    return static_cast<_State>(m_uiState);
}

void SystemComponent::SetName(const String& rName)
{
    m_Name = rName;
}

bool SystemComponent::IncrementInUseCount(const wchar_t*)
{
    if (!UseIsAllowed())
        return false;

    if (m_uiState == STATE_READY)
        SetState(STATE_IN_USE, false);

    ++m_iInUseCount;
    return true;
}

void SystemComponent::DecrementInUseCount(const wchar_t*)
{
    --m_iInUseCount;
    if (m_iInUseCount != 0)
        return;

    SetState(m_uiState == STATE_IN_USE ? STATE_READY : STATE_TERMINATING, false);
}

qResult SystemComponent::WaitForTerminatedState(unsigned int uiTimeout)
{
    Terminate();

    qResult result;

    if (Scheduler::PreventRegularBlockCall(uiTimeout != 0, uiTimeout,
            &WaitForTerminatedStatePreventBlockCallCallback, this, &result))
        return result;

    SpinTest spinTest(10, uiTimeout);

    spinTest.LeaveOnTimeout();

    while (TestState() != STATE_TERMINATED
        && spinTest.SpinOnce(
               L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/SystemComponent.cpp",
               339, L"TestState()!=Terminated")
            != 0)
    {
    }

    if (TestState() == STATE_TERMINATED)
        return qResult(0x10001);

    return qResult(static_cast<int>(0x8001000B));
}

SystemComponent::Use::Use(SystemComponent* pComponent, const wchar_t* pReason)
    : m_pComponent(pComponent)
    , m_pReason(pReason)
    , m_bInUse(pComponent->IncrementInUseCount(pReason))
{
}

SystemComponent::Use::~Use()
{
    if (m_bInUse)
        m_pComponent->DecrementInUseCount(m_pReason);
}

}
}

bool WaitForTerminatedStatePreventBlockCallCallback(
    nn::nex::RootObject* pObject, unsigned int uiTimeout)
{
    return static_cast<bool>(
        static_cast<nn::nex::SystemComponent*>(pObject)->WaitForTerminatedState(uiTimeout));
}
