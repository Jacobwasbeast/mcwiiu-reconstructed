#include "nn/nex/InstanceControl.hpp"

#include "nn/nex/InstanceTable.hpp"

namespace nn
{
namespace nex
{

CriticalSection g_instanceTableCriticalSection(2);

InstanceControl::InstanceControl(unsigned int classId, unsigned int contextId)
{
    ScopedCS lock(&g_instanceTableCriticalSection);

    m_registered = false;
    if (g_instanceTable.AddInstance(this, classId, contextId))
    {
        m_classId = classId;
        m_contextId = contextId;
        m_delegatorInstance = 0;
        m_registered = true;
    }

}

InstanceControl::~InstanceControl()
{
    ScopedCS lock(&g_instanceTableCriticalSection);

    if (m_registered)
        g_instanceTable.DelInstance(this, m_classId, m_contextId);

}

void InstanceControl::SetDelegatorInstance(void* delegatorInstance)
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    m_delegatorInstance = delegatorInstance;
}

int InstanceControl::GetHighestID()
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    int highestId = g_instanceTable.GetHighestID();
    return highestId;
}

bool InstanceControl::ContextIsValid(unsigned int contextId)
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    bool valid = g_instanceTable.ContextIsValid(contextId);
    return valid;
}

void InstanceControl::AllocateExtraContexts(unsigned int contextCount)
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    g_instanceTable.AllocateExtraContexts(contextCount);
}

void InstanceControl::FreeExtraContexts()
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    g_instanceTable.FreeExtraContexts();
}

bool InstanceControl::DeleteContext(unsigned int contextId)
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    bool deleted = g_instanceTable.DeleteContext(contextId);
    return deleted;
}

unsigned int InstanceControl::FindInstanceContext(InstanceControl* instance, unsigned int classId)
{
    ScopedCS lock(&g_instanceTableCriticalSection);
    unsigned int contextId = g_instanceTable.FindInstanceContext(instance, classId);
    return contextId;
}

}
}
