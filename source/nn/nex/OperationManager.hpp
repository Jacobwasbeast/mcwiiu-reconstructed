#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_OPERATIONMANAGER_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_OPERATIONMANAGER_HPP

#include <list>

#include "nn/nex/MemAllocator.hpp"
#include "nn/nex/RootObject.hpp"

namespace nn { namespace nex {

class Operation;
class OperationCallback;

class OperationManager : public RootObject
{
public:
    OperationManager();
    virtual ~OperationManager();

    Operation* GetCurrentOperation() const;
    void TraceOperationStack(unsigned long long flags);
    void InvokeCallbacks(int firstPriority, int lastPriority, Operation* operation);
    void OperationBegins(Operation* operation);
    void PopOperation(Operation* operation);
    void OperationEnds(Operation* operation);
    void RegisterCallback(OperationCallback* callback);
    bool UnregisterCallback(OperationCallback* callback);

private:
    typedef std::list<OperationCallback*, MemAllocator<OperationCallback*> > CallbackList;
    typedef std::list<Operation*, MemAllocator<Operation*> > OperationList;

    CallbackList m_callbacks;
    OperationList m_operations;
};

}}

#endif
