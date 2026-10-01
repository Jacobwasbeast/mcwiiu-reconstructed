#ifndef GAME_ITEM_NONNULLLIST_HPP
#define GAME_ITEM_NONNULLLIST_HPP

#include <cstddef>

template <class T>
class NonNullList
{
public:
    NonNullList()
        : items(NULL),
          length(0)
    {
    }

    ~NonNullList();

    static NonNullList<T> withSize(unsigned int size, const T& defaultValue);

    unsigned int size() const
    {
        return length;
    }

    T& operator[](unsigned int index)
    {
        return items[index];
    }

    const T& operator[](unsigned int index) const
    {
        return items[index];
    }

    T* items;
    unsigned int length;
    T defaultValue;

private:
    NonNullList(T* items, unsigned int length, const T& defaultValue);
};

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

#endif
