#include "nn/nex/GlobalVariables.hpp"

#include "nn/nex/ThreadVariableList.hpp"

namespace nn
{
namespace nex
{

ThreadVariableList* g_threadVariableList = 0;
bool g_threadVariableListReady = false;

namespace
{

GlobalVariables g_globalVariablesStorage;
GlobalVariables* g_globalVariables = 0;
std::int32_t g_globalVariablesCount = 0;

}

void GlobalVariables::AcquireInstance()
{
    if (g_globalVariables == 0)
    {
        g_threadVariableList = new ThreadVariableList();
        g_globalVariables = &g_globalVariablesStorage;
        g_globalVariablesCount = 1;
        return;
    }

    ++g_globalVariablesCount;
}

void GlobalVariables::ReleaseInstance()
{
    if (g_globalVariables == 0)
        return;

    if (g_globalVariablesCount > 0)
        --g_globalVariablesCount;

    if (g_globalVariablesCount == 0)
    {
        delete g_threadVariableList;
        g_threadVariableList = 0;
        g_globalVariables = 0;
    }
}

std::int32_t GlobalVariables::GetCount()
{
    return g_globalVariablesCount;
}

void GlobalVariables::ClearCache()
{
}

}
}
