#include "nn/nex/InstanceControl.hpp"

#include "nn/nex/InstanceTable.hpp"

namespace nn
{
namespace nex
{

CriticalSection g_instanceTableCriticalSection(2);

InstanceControl::InstanceControl(unsigned int classId, unsigned int contextId)
{
    g_instanceTableCriticalSection.Enter();

    m_registered = false;
    if (g_instanceTable.AddInstance(this, classId, contextId))
    {
        m_classId = classId;
        m_contextId = contextId;
        m_delegatorInstance = 0;
        m_registered = true;
    }

    g_instanceTableCriticalSection.Leave();
}

InstanceControl::~InstanceControl()
{
    g_instanceTableCriticalSection.Enter();

    if (m_registered)
        g_instanceTable.DelInstance(this, m_classId, m_contextId);

    g_instanceTableCriticalSection.Leave();
}

void InstanceControl::SetDelegatorInstance(void* delegatorInstance)
{
    g_instanceTableCriticalSection.Enter();
    m_delegatorInstance = delegatorInstance;
    g_instanceTableCriticalSection.Leave();
}

int InstanceControl::GetHighestID()
{
    g_instanceTableCriticalSection.Enter();
    int highestId = g_instanceTable.GetHighestID();
    g_instanceTableCriticalSection.Leave();
    return highestId;
}

bool InstanceControl::ContextIsValid(unsigned int contextId)
{
    g_instanceTableCriticalSection.Enter();
    bool valid = g_instanceTable.ContextIsValid(contextId);
    g_instanceTableCriticalSection.Leave();
    return valid;
}

void InstanceControl::AllocateExtraContexts(unsigned int contextCount)
{
    g_instanceTableCriticalSection.Enter();
    g_instanceTable.AllocateExtraContexts(contextCount);
    g_instanceTableCriticalSection.Leave();
}

void InstanceControl::FreeExtraContexts()
{
    g_instanceTableCriticalSection.Enter();
    g_instanceTable.FreeExtraContexts();
    g_instanceTableCriticalSection.Leave();
}

bool InstanceControl::DeleteContext(unsigned int contextId)
{
    g_instanceTableCriticalSection.Enter();
    bool deleted = g_instanceTable.DeleteContext(contextId);
    g_instanceTableCriticalSection.Leave();
    return deleted;
}

unsigned int InstanceControl::FindInstanceContext(InstanceControl* instance, unsigned int classId)
{
    g_instanceTableCriticalSection.Enter();
    unsigned int contextId = g_instanceTable.FindInstanceContext(instance, classId);
    g_instanceTableCriticalSection.Leave();
    return contextId;
}

}
}
