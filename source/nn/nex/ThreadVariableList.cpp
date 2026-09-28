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
    , m_criticalSection(1)
{
    g_threadVariableListReady = true;
}

ThreadVariableList::~ThreadVariableList()
{
    ResetValues();
    g_threadVariableListReady = false;

    while (m_first != 0)
    {
        ThreadVariableRoot* variable = m_first;

        m_first = variable->m_next;
        variable->m_next = 0;
        variable->m_previous = 0;
        --m_count;
    }

    m_last = 0;
}

void ThreadVariableList::AddVariable(ThreadVariableRoot* variable)
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    if (m_first == 0)
    {
        m_first = variable;
    }
    else
    {
        m_last->m_next = variable;
        variable->m_previous = m_last;
    }

    m_last = variable;
    ++m_count;

}

ThreadVariableList* ThreadVariableList::GetInstanceRef()
{
    return g_threadVariableList;
}

void ThreadVariableList::ClearValue(uint32_t value)
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    for (ThreadVariableRoot* variable = m_first; variable != 0; variable = variable->m_next)
        variable->ClearValue(value);

}

void ThreadVariableList::RemoveVariable(ThreadVariableRoot* variable)
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    ThreadVariableRoot* current = m_first;
    while (current != 0)
    {
        if (current != variable)
        {
            current = current->m_next;
            continue;
        }

        ThreadVariableRoot* next = current->m_next;

        if (current->m_previous != 0)
            current->m_previous->m_next = next;
        else
            m_first = next;

        if (next != 0)
            next->m_previous = current->m_previous;
        else
            m_last = current->m_previous;

        current->m_next = 0;
        current->m_previous = 0;
        --m_count;

        current = next;
    }

}

void ThreadVariableList::ResetValues()
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    for (ThreadVariableRoot* variable = m_first; variable != 0; variable = variable->m_next)
        variable->ResetValues();

}

void ThreadVariableList::ClearValue()
{
    if (!g_threadVariableListReady)
        return;

    ScopedCS lock(&m_criticalSection);

    for (ThreadVariableRoot* variable = m_first; variable != 0; variable = variable->m_next)
        variable->ClearValue();

}

}
}
