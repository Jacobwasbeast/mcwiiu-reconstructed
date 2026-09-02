#ifndef GAME_CORE_PODHANDLEARRAY_HPP
#define GAME_CORE_PODHANDLEARRAY_HPP

#include <string.h>

#include "game/core/Handle.hpp"

Handle Handle_AllocateInfo(int size, const char* file, int line);
void* Handle_Get(Handle handle);
void Handle_Free(Handle handle);

template <class T>
class PODHandleArray
{
public:
    void Construct();
    void FreeMem();
    void Reacquire();

    int VerifyRoom(int count);
    int Push(const T& value);
    int PushCount(const T* values, int count);
    int PushEmpty();
    int Insert(int index, const T& value);

    void Pop();
    void RemoveUnsorted(int index);
    void RemoveSortedCount(int index, int count);

    T& operator[](int index);
    T& Top();
    T* Data();
    int Count() const;

    Handle m_handle;
    T* m_data;
    int m_count;
    int m_capacity;
    int m_locked;
};

template <class T>
void PODHandleArray<T>::Construct()
{
    m_handle = Handle();
    m_data = NULL;
    m_count = 0;
    m_capacity = 0;
    m_locked = 0;
}

template <class T>
void PODHandleArray<T>::FreeMem()
{
    m_count = 0;
    m_locked = 0;

    Handle_Free(m_handle);

    m_handle = Handle();
    m_data = NULL;
    m_capacity = 0;
}

template <class T>
void PODHandleArray<T>::Reacquire()
{
    m_data = (T*)Handle_Get(m_handle);
}

template <class T>
int PODHandleArray<T>::VerifyRoom(int count)
{
    if (count <= m_capacity)
    {
        return 1;
    }

    if (count < 4)
    {
        count = 4;
    }

    int capacity = m_capacity * 2;
    if (capacity < count)
    {
        capacity = count;
    }

    if (m_locked != 0)
    {
        return 0;
    }

    Handle handle = Handle_AllocateInfo(capacity * sizeof(T), __FILE__, __LINE__);

    T* data = (T*)Handle_Get(handle);
    if (data == NULL)
    {
        return 0;
    }

    memcpy(data, m_data, m_count * sizeof(T));

    Handle_Free(m_handle);

    m_handle = handle;
    m_data = data;
    m_capacity = capacity;
    return 1;
}

template <class T>
int PODHandleArray<T>::Push(const T& value)
{
    if (!VerifyRoom(m_count + 1))
    {
        return 0;
    }

    memcpy(&m_data[m_count], &value, sizeof(T));
    m_count++;
    return 1;
}

template <class T>
int PODHandleArray<T>::PushCount(const T* values, int count)
{
    if (!VerifyRoom(m_count + count))
    {
        return 0;
    }

    memcpy(&m_data[m_count], values, count * sizeof(T));
    m_count += count;
    return 1;
}

template <class T>
int PODHandleArray<T>::PushEmpty()
{
    if (!VerifyRoom(m_count + 1))
    {
        return 0;
    }

    m_count++;
    return 1;
}

template <class T>
int PODHandleArray<T>::Insert(int index, const T& value)
{
    if (!VerifyRoom(m_count + 1))
    {
        return 0;
    }

    int count = m_count;
    m_count = count + 1;

    for (int i = count; i > index; i--)
    {
        m_data[i] = m_data[i - 1];
    }

    m_data[index] = value;
    return 1;
}

template <class T>
void PODHandleArray<T>::Pop()
{
    m_count--;
}

template <class T>
void PODHandleArray<T>::RemoveUnsorted(int index)
{
    m_count--;

    if (m_count != index)
    {
        memcpy(&m_data[index], &m_data[m_count], sizeof(T));
    }
}

template <class T>
void PODHandleArray<T>::RemoveSortedCount(int index, int count)
{
    m_count -= count;

    memmove(&m_data[index], &m_data[index + count], (m_count - index) * sizeof(T));
}

template <class T>
T& PODHandleArray<T>::operator[](int index)
{
    return m_data[index];
}

template <class T>
T& PODHandleArray<T>::Top()
{
    return m_data[m_count - 1];
}

template <class T>
T* PODHandleArray<T>::Data()
{
    return m_data;
}

template <class T>
int PODHandleArray<T>::Count() const
{
    return m_count;
}

#endif
