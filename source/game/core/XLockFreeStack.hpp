#ifndef GAME_CORE_XLOCKFREESTACK_HPP
#define GAME_CORE_XLOCKFREESTACK_HPP

#include "game/core/CriticalSection.hpp"

#include <cstddef>
#include <vector>

template <class T>
class XLockFreeStack
{
public:
    XLockFreeStack();
    ~XLockFreeStack();

    void Push(T* pItem);
    T* Pop();

private:
    std::vector<T*> m_items;
    OSFastMutex m_lock;
};

template <class T>
XLockFreeStack<T>::XLockFreeStack()
{
    InitializeCriticalSectionAndSpinCount(&m_lock, 5120);
}

template <class T>
XLockFreeStack<T>::~XLockFreeStack()
{
    DeleteCriticalSection(&m_lock);
}

template <class T>
void XLockFreeStack<T>::Push(T* pItem)
{
    EnterCriticalSection(&m_lock);

    m_items.push_back(pItem);

    LeaveCriticalSection(&m_lock);
}

template <class T>
T* XLockFreeStack<T>::Pop()
{
    EnterCriticalSection(&m_lock);

    if (!m_items.empty())
    {
        T* pItem = m_items.back();

        m_items.pop_back();

        LeaveCriticalSection(&m_lock);

        return pItem;
    }

    LeaveCriticalSection(&m_lock);

    return NULL;
}

#endif
