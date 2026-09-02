#include "game/ui/MethodAction.hpp"

template <class T>
MethodAction<T>::MethodAction(T* pTarget, Func pFunc)
    : m_pTarget(pTarget)
    , m_pFunc(pFunc)
{
}

template <class T>
MethodAction<T>::~MethodAction()
{
}

template <class T>
void MethodAction<T>::Execute()
{
    (m_pTarget->*m_pFunc)();
}
