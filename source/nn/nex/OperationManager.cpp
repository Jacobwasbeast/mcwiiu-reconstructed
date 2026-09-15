#include "nn/nex/OperationManager.hpp"

namespace nn { namespace nex {

namespace
{

typedef void (*OperationCallbackMethod)(OperationCallback*, Operation*);

struct OperationCallbackLayout
{
    int priority;
    void** virtualTable;
};

int CallbackPriority(OperationCallback* callback)
{
    return static_cast<OperationCallbackLayout*>(static_cast<void*>(callback))->priority;
}

void InvokeOperationCallback(OperationCallback* callback, Operation* operation)
{
    OperationCallbackLayout* layout =
        static_cast<OperationCallbackLayout*>(static_cast<void*>(callback));
    OperationCallbackMethod method =
        reinterpret_cast<OperationCallbackMethod>(layout->virtualTable[5]);
    method(callback, operation);
}

}

OperationManager::OperationManager()
    : m_callbacks(), m_operations()
{
}

OperationManager::~OperationManager()
{
}

Operation* OperationManager::GetCurrentOperation() const
{
    return m_operations.empty() ? 0 : m_operations.back();
}

void OperationManager::TraceOperationStack(unsigned long long)
{
    for (OperationList::const_iterator it = m_operations.begin(); it != m_operations.end(); ++it)
    {
    }
}

void OperationManager::InvokeCallbacks(int firstPriority, int lastPriority, Operation* operation)
{
    for (CallbackList::const_iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        int priority = CallbackPriority(*it);
        if (firstPriority <= priority)
        {
            if (lastPriority < priority)
                return;

            InvokeOperationCallback(*it, operation);
        }
    }
}

void OperationManager::OperationBegins(Operation* operation)
{
    m_operations.push_back(operation);
    InvokeCallbacks(-1024, -1, operation);
}

void OperationManager::PopOperation(Operation*)
{
    if (!m_operations.empty())
        m_operations.pop_back();
}

void OperationManager::OperationEnds(Operation* operation)
{
    InvokeCallbacks(1, 0x400, operation);
    PopOperation(operation);
}

void OperationManager::RegisterCallback(OperationCallback* callback)
{
    CallbackList::iterator position = m_callbacks.end();
    int priority = CallbackPriority(callback);

    for (CallbackList::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        if (priority < CallbackPriority(*it))
        {
            position = it;
            break;
        }
    }

    m_callbacks.insert(position, callback);
}

bool OperationManager::UnregisterCallback(OperationCallback* callback)
{
    for (CallbackList::iterator it = m_callbacks.begin(); it != m_callbacks.end(); ++it)
    {
        if (*it == callback)
        {
            m_callbacks.erase(it);
            return true;
        }
    }

    return false;
}

}}
