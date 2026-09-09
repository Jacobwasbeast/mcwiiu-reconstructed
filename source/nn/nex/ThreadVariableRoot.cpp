#include "nn/nex/ThreadVariableRoot.hpp"

namespace nn
{
namespace nex
{

class ObjectThreadRoot
{
public:
    static std::uint32_t GetCurrentThreadID();
};

ThreadVariableRoot::ThreadVariableRoot()
    : m_threadVariableState(0)
    , m_threadVariableValue(0)
    , m_variableOwner(0)
    , m_next(0)
    , m_previous(0)
{
}

ThreadVariableRoot::~ThreadVariableRoot()
{
}

void ThreadVariableRoot::ResetValues()
{
}

void ThreadVariableRoot::ClearValue()
{
}

void ThreadVariableRoot::ClearValue(std::uint32_t)
{
}

std::uint32_t ThreadVariableRoot::GetThreadID()
{
    return ObjectThreadRoot::GetCurrentThreadID();
}

}
}
