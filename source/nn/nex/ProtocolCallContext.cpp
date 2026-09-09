#include "nn/nex/ProtocolCallContext.hpp"
#include "nn/nex/Credentials.hpp"

namespace nn
{
namespace nex
{

ProtocolCallContext::ProtocolCallContext(Credentials* credentials)
    : CallContext()
    , m_returnValues()
    , m_credentials(0)
    , m_completionFlag(0)
    , m_completionResult(0)
{
    SetCredentials(credentials);
}

ProtocolCallContext::~ProtocolCallContext()
{
    SetCredentials(0);
    m_completionFlag = 0;
    m_completionResult = 0;
    m_returnValues.clear();
}

void ProtocolCallContext::SetCredentials(Credentials* credentials)
{
    if (m_credentials != 0)
        m_credentials->ReleaseRef();
    m_credentials = credentials;
    if (m_credentials != 0)
        m_credentials->AddRef();
}

void ProtocolCallContext::AddReturnValuePtr(void* value)
{
    m_returnValues.push_back(value);
}

void* ProtocolCallContext::GetReturnValuePtr(std::size_t index) const
{
    if (index >= m_returnValues.size())
        return 0;
    return m_returnValues[index];
}

void ProtocolCallContext::BeginTransition(State state, const qResult& result, bool publishResult)
{
    if (state == Ready)
        m_completionFlag = 0;
    else if (state == CallFailed && publishResult)
    {
        if (m_completionFlag != 0)
        {
            *m_completionFlag = false;
            m_completionFlag = 0;
        }
        if (m_completionResult != 0)
        {
            *m_completionResult = result;
            m_completionResult = 0;
        }
    }
    CallContext::BeginTransition(state, result, publishResult);
}

}
}
