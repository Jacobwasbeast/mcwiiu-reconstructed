#include "nn/nex/GlobalVariables.hpp"

#include "nn/nex/CriticalSection.hpp"
#include "nn/nex/ThreadVariableList.hpp"

namespace nn
{
namespace nex
{

GlobalVariables* g_globalVariables = 0;
ThreadVariableList* g_threadVariableList = 0;
bool g_threadVariableListReady = false;

ForcedCriticalSection g_globalVariablesCriticalSection(1);

GlobalVariables::GlobalVariables()
    : m_broadcastAddress()
    , m_errorCode(0)
    , m_errorDetail(0)
    , m_currentThread(static_cast<ObjectThreadRoot*>(0))
    , m_binaryDataMap()
    , m_protocolNameMap()
    , m_methodNameMap()
    , m_errorNameMap()
    , m_protocolVersionMap()
{
    g_criticalSectionDisabled = false;
    m_initialized = true;
    m_broadcastAddress = L"255.255.255.255";
}

GlobalVariables::~GlobalVariables()
{
    g_globalVariablesCriticalSection.Enter();
    g_globalVariables = 0;
    g_globalVariablesCriticalSection.Leave();
}

void GlobalVariables::AcquireInstance()
{
    g_globalVariablesCriticalSection.Enter();

    if (g_globalVariables == 0)
    {
        g_threadVariableList = new ThreadVariableList();
        g_globalVariables = new GlobalVariables();
    }
    else
    {
        g_globalVariables->AddRef();
    }

    g_globalVariablesCriticalSection.Leave();
}

void GlobalVariables::ReleaseInstance()
{
    g_globalVariablesCriticalSection.Enter();

    if (g_globalVariables == 0)
    {
        g_globalVariablesCriticalSection.Leave();
        return;
    }

    const std::uint32_t refCount = g_globalVariables->GetRefCount();
    g_globalVariables->ReleaseRef();

    if (refCount == 1)
    {
        g_globalVariables = 0;
        delete g_threadVariableList;
        g_threadVariableList = 0;
    }

    g_globalVariablesCriticalSection.Leave();
}

void GlobalVariables::ClearCache()
{
    g_globalVariablesCriticalSection.Enter();
    g_globalVariablesCriticalSection.Leave();
}

}
}
