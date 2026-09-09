#include "nn/nex/ThreadVariableList.hpp"

#include "nn/nex/ThreadVariableRoot.hpp"

namespace nn
{
namespace nex
{

extern ThreadVariableList* g_threadVariableList;
extern bool g_threadVariableListReady;

ThreadVariableList::ThreadVariableList()
    : m_first(0)
    , m_last(0)
    , m_count(0)
    , m_criticalSection(0)
    , m_initialized(true)
{
    g_threadVariableListReady = true;
}

ThreadVariableList::~ThreadVariableList()
{
    ResetValues();
    m_first = 0;
    m_last = 0;
    m_count = 0;
    m_initialized = false;
    if (g_threadVariableList == this)
        g_threadVariableList = 0;
    g_threadVariableListReady = false;
}

void ThreadVariableList::AddVariable(ThreadVariableRoot* variable)
{
    if (!m_initialized || variable == 0)
        return;

    variable->m_previous = m_last;
    variable->m_next = 0;
    if (m_last != 0)
        m_last->m_next = variable;
    else
        m_first = variable;
    m_last = variable;
    ++m_count;
}

ThreadVariableList* ThreadVariableList::GetInstanceRef()
{
    return g_threadVariableList;
}

void ThreadVariableList::ClearValue(std::uint32_t value)
{
    for (ThreadVariableRoot* variable = m_first; variable != 0; variable = variable->m_next)
        variable->ClearValue(value);
}

void ThreadVariableList::RemoveVariable(ThreadVariableRoot* variable)
{
    if (!m_initialized || variable == 0)
        return;

    if (variable->m_previous != 0)
        variable->m_previous->m_next = variable->m_next;
    else if (m_first == variable)
        m_first = variable->m_next;
    if (variable->m_next != 0)
        variable->m_next->m_previous = variable->m_previous;
    else if (m_last == variable)
        m_last = variable->m_previous;
    variable->m_next = 0;
    variable->m_previous = 0;
    if (m_count != 0)
        --m_count;
}

void ThreadVariableList::ResetValues()
{
    for (ThreadVariableRoot* variable = m_first; variable != 0; variable = variable->m_next)
        variable->ResetValues();
}

void ThreadVariableList::ClearValue()
{
    for (ThreadVariableRoot* variable = m_first; variable != 0; variable = variable->m_next)
        variable->ClearValue();
}

}
}
