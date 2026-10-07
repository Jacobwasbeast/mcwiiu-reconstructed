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
    , m_nexTokenMap()
    , m_nexPasswordMap()
    , m_nexHostMap()
    , m_nexPortMap()
    , m_eventLog()
    , m_traceLog()
    , m_cacheManager()
    , m_packetBufferParams()
{
    g_criticalSectionDisabled = false;
    m_packetBufferPoolEnabled = true;
    m_broadcastAddress = L"255.255.255.255";
}

GlobalVariables::~GlobalVariables()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);
    g_globalVariables = 0;
}

void GlobalVariables::AcquireInstance()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);

    if (g_globalVariables == 0)
    {
        g_threadVariableList = new ThreadVariableList;
        g_globalVariables = new GlobalVariables;
    }
    else
    {
        g_globalVariables->AddRef();
    }

}

void GlobalVariables::ReleaseInstance()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);

    if (g_globalVariables == 0)
    {
        return;
    }

    const uint32_t refCount = g_globalVariables->GetRefCount();
    g_globalVariables->ReleaseRef();

    if (refCount == 1)
    {
        g_globalVariables = 0;
        delete g_threadVariableList;
        g_threadVariableList = 0;
    }

}

void GlobalVariables::ClearCache()
{
    ScopedCS lock(&g_globalVariablesCriticalSection);
}

}
}
