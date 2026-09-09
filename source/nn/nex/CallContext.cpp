#include "nn/nex/CallContext.hpp"
#include "nn/nex/Scheduler.hpp"
#include "nn/nex/SpinTest.hpp"

namespace nn
{
namespace nex
{

CallContext::CallContext()
    : m_state(Ready)
    , m_flags(0)
    , m_result()
{
}

CallContext::~CallContext()
{
}

std::uint8_t CallContext::Wait(std::uint32_t timeout)
{
    if (m_state == CallInProgress)
    {
        bool result = false;
        if (Scheduler::PreventBlockCall(timeout != 0, timeout, false,
                &CallContext::PreventBlockCallCallback, this, &result) != 0)
            return result ? 1 : 0;

        SpinTest spinTest(10, timeout);
        spinTest.LeaveOnTimeout();
        while (m_state == CallInProgress)
        {
            Scheduler::GlobalSingleThreadDispatch(spinTest.GetRemainingTime());
            if (spinTest.SpinOnce(L"D:/home/jenkins/workspace/Master/Pack/d4f027b8/OnlineCore/src/Core/CallContext.cpp", 0x1a4,
                    L"GetState()==CallContext::CallInProgress") == 0)
                break;
        }
    }
    return static_cast<std::uint8_t>(m_state >= CallSucceeded && m_state <= CallCanceled);
}

bool CallContext::PreventBlockCallCallback(RootObject* object, std::uint32_t value)
{
    CallContext* context = static_cast<CallContext*>(object);
    return context->Wait(value) != 0;
}

CallContext::State CallContext::GetState() const
{
    return m_state;
}

const qResult& CallContext::GetResult() const
{
    return m_result;
}

void CallContext::BeginTransition(State state, const qResult& result, bool publishResult)
{
    if (publishResult)
        m_result = result;
    m_state = state;
}

}
}
