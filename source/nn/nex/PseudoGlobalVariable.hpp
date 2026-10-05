#ifndef LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLE_HPP
#define LEGACY_CONSOLE_WIIU_NN_NEX_PSEUDOGLOBALVARIABLE_HPP

#include <new>

#include "nn/nex/MemoryManager.hpp"
#include "nn/nex/PseudoGlobalVariableList.hpp"
#include "nn/nex/PseudoGlobalVariableRoot.hpp"
#include "nn/nex/PseudoSingleton.hpp"

namespace nn
{
namespace nex
{

template <typename T>
class PseudoGlobalVariable : public PseudoGlobalVariableRoot
{
public:
    PseudoGlobalVariable()
        : PseudoGlobalVariableRoot()
        , m_value()
        , m_initialValue()
    {
        m_value = m_initialValue;
        m_pExtraContexts = 0;
        g_pseudoGlobalVariableList.AddVariable(this);
        if (GetNbOfExtraContexts() > 1)
            AllocateExtraContexts();
    }

    explicit PseudoGlobalVariable(const T& rInitialValue)
        : PseudoGlobalVariableRoot()
        , m_value(rInitialValue)
        , m_initialValue(rInitialValue)
    {
        m_pExtraContexts = 0;
        g_pseudoGlobalVariableList.AddVariable(this);
        if (GetNbOfExtraContexts() > 1)
            AllocateExtraContexts();
    }

    virtual ~PseudoGlobalVariable()
    {
        g_pseudoGlobalVariableList.RemoveVariable(this);
        FreeExtraContexts();
    }

    virtual void AllocateExtraContexts()
    {
        unsigned int uiCount = GetNbOfExtraContexts();
        m_pExtraContexts = static_cast<T*>(MemoryManager::Allocate(uiCount * sizeof(T)));
        for (unsigned int i = 0; i < uiCount; ++i)
            ::new (static_cast<void*>(&m_pExtraContexts[i])) T(m_initialValue);
    }

    virtual void FreeExtraContexts()
    {
        if (m_pExtraContexts != 0)
        {
            for (unsigned int i = 0; i < GetNbOfExtraContexts(); ++i)
                m_pExtraContexts[i].~T();

            MemoryManager::Free(m_pExtraContexts);
            m_pExtraContexts = 0;
        }
    }

    virtual void ResetContext(unsigned int uiContextId)
    {
        m_pExtraContexts[uiContextId] = m_initialValue;
    }

    virtual PseudoGlobalVariableRoot* GetNext()
    {
        return m_pNext;
    }

    virtual void SetNext(PseudoGlobalVariableRoot* pNext)
    {
        m_pNext = pNext;
    }

    T& Get()
    {
        if (g_usingInstantiationContext)
        {
            unsigned int uiContextId = g_instantiationContextVariable->GetRef();
            if (uiContextId != 0)
                return m_pExtraContexts[uiContextId];
        }

        return m_value;
    }

private:
    T* m_pExtraContexts;
    T m_value;
    T m_initialValue;
};

}
}

#endif
