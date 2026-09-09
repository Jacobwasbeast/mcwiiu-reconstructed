#include "nn/nex/SystemComponent.hpp"

namespace nn
{
namespace nex
{

SystemComponent::SystemComponent(const String& name)
    : m_name(), m_state(STATE_CREATED), m_inUseCount(0), m_parent(0)
{
    (void)name;
}

SystemComponent::~SystemComponent()
{
    SetParent(0);
}

void SystemComponent::StateTransition(State)
{
}

void SystemComponent::OnInitialize()
{
    (void)TestState();
}

void SystemComponent::OnTerminate()
{
    (void)TestState();
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

SystemComponent::State SystemComponent::TestState() const
{
    return static_cast<State>(m_state);
}

void SystemComponent::DoWork()
{
}

bool SystemComponent::ValidTransition(State state) const
{
    if (state == STATE_INITIALIZATION_FAILED || state == STATE_TERMINATION_FAILED)
        return true;

    switch (m_state)
    {
    case STATE_CREATED:
        return state == STATE_INITIALIZING || state == STATE_TERMINATING || state == STATE_TERMINATED;
    case STATE_INITIALIZING:
        return state == STATE_READY;
    case STATE_READY:
        return state == STATE_IN_USE || state == STATE_TERMINATING;
    case STATE_IN_USE:
        return state == STATE_READY || state == STATE_TERMINATING;
    case STATE_TERMINATING:
        return state == STATE_TERMINATED;
    case STATE_TERMINATED:
        return state == STATE_CREATED || state == STATE_INITIALIZING;
    default:
        return false;
    }
}

bool SystemComponent::UseIsAllowed() const
{
    return m_state == STATE_READY || m_state == STATE_IN_USE;
}

void SystemComponent::SetParent(SystemComponent* parent)
{
    m_parent = parent;
}

bool SystemComponent::SetState(State state, bool force)
{
    if (!force && !ValidTransition(state))
        return false;

    StateTransition(state);
    m_state = state;
    return true;
}

SystemComponent::State SystemComponent::Initialize()
{
    OnInitialize();
    if (m_state == STATE_CREATED || m_state == STATE_TERMINATED)
        SetState(STATE_INITIALIZING, false);
    if (m_state == STATE_INITIALIZING && BeginInitialization())
        SetState(STATE_READY, false);
    return TestState();
}

SystemComponent::State SystemComponent::Terminate()
{
    OnTerminate();
    if (m_state == STATE_READY || m_state == STATE_IN_USE)
        SetState(STATE_TERMINATING, m_state == STATE_IN_USE);
    if (m_state == STATE_TERMINATING && BeginTermination())
        SetState(STATE_TERMINATED, false);
    return TestState();
}

bool SystemComponent::IncrementInUseCount(const wchar_t*)
{
    if (!UseIsAllowed())
        return false;
    if (m_state == STATE_READY)
        SetState(STATE_IN_USE, false);
    ++m_inUseCount;
    return true;
}

void SystemComponent::DecrementInUseCount(const wchar_t*)
{
    if (m_inUseCount > 0)
        --m_inUseCount;
    if (m_inUseCount == 0 && m_state == STATE_IN_USE)
        SetState(STATE_READY, false);
}

const wchar_t* SystemComponent::GetType() const
{
    return L"SystemComponent";
}

bool SystemComponent::IsAKindOf(const wchar_t* type) const
{
    if (type == 0)
        return false;
    const wchar_t* lhs = GetType();
    while (*lhs != 0 && *type != 0 && *lhs == *type)
    {
        ++lhs;
        ++type;
    }
    return *lhs == 0 && *type == 0;
}

}
}
