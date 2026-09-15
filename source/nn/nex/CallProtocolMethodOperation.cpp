#include "nn/nex/CallProtocolMethodOperation.hpp"

namespace nn { namespace nex {

CallProtocolMethodOperation::CallProtocolMethodOperation()
    : Operation(0), m_attemptCount(0), m_postponed(false), m_callUserContext(),
      m_postponementDelay(0), m_result()
{
}

CallProtocolMethodOperation::~CallProtocolMethodOperation()
{
}

unsigned int CallProtocolMethodOperation::GetType() const
{
    return 19;
}

const wchar_t* CallProtocolMethodOperation::GetClassNameString() const
{
    return L"CallProtocolMethod";
}

void CallProtocolMethodOperation::ForceImplOperationCommonMethodsMacro()
{
}

void CallProtocolMethodOperation::PostponeOperation(int delay)
{
    m_postponementDelay = delay;
    m_postponed = true;
}

void CallProtocolMethodOperation::InitializeForNewCallAttempt()
{
    m_postponed = false;
    ++m_attemptCount;
    m_result = qResult(0x10001);
}

unsigned int CallProtocolMethodOperation::GetAttemptCount()
{
    return m_attemptCount;
}

}}
