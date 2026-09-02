#ifndef GAME_ITEM_NONNULLLIST_HPP
#define GAME_ITEM_NONNULLLIST_HPP

#include <cstddef>

template <class T>
class NonNullList
{
public:
    NonNullList();
    ~NonNullList();

    static NonNullList<T> withSize(unsigned int size, const T& defaultValue);

    unsigned int size() const;

    T& operator[](unsigned int index);
    const T& operator[](unsigned int index) const;

private:
    NonNullList(T* items, unsigned int length, const T& defaultValue);

    T* items;
    unsigned int length;
    T defaultValue;
};

template <class T>
NonNullList<T>::NonNullList()
    : items(NULL),
      length(0)
{
}

template <class T>
NonNullList<T>::NonNullList(T* items, unsigned int length, const T& defaultValue)
    : items(items),
      length(length),
      defaultValue(defaultValue)
{
}

template <class T>
NonNullList<T>::~NonNullList()
{
}

template <class T>
NonNullList<T> NonNullList<T>::withSize(unsigned int size, const T& defaultValue)
{
    if (size == 0)
    {
        return NonNullList<T>(NULL, 0, defaultValue);
    }

    T* items = new T[size];

    for (unsigned int i = 0; i < size; i++)
    {
        items[i] = defaultValue;
    }

    return NonNullList<T>(items, size, defaultValue);
}

template <class T>
unsigned int NonNullList<T>::size() const
{
    return length;
}

template <class T>
T& NonNullList<T>::operator[](unsigned int index)
{
    return items[index];
}

template <class T>
const T& NonNullList<T>::operator[](unsigned int index) const
{
    return items[index];
}

#endif
