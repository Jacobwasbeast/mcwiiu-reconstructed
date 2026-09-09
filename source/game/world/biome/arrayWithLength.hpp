#ifndef GAME_WORLD_BIOME_ARRAYWITHLENGTH_HPP
#define GAME_WORLD_BIOME_ARRAYWITHLENGTH_HPP

#include <cstddef>
#include <vector>

template <class T>
class arrayWithLength
{
public:
    T* data;
    unsigned int length;

    arrayWithLength();
    arrayWithLength(unsigned int count, bool clear);
    arrayWithLength(T* values, unsigned int count);
    arrayWithLength(std::vector<T>& values);

    void resize(unsigned int newLength);
    void resizeNoDelete(unsigned int newLength, void** oldData);
};

template <class T>
arrayWithLength<T>::arrayWithLength()
{
    data = NULL;
    length = 0;
}

template <class T>
arrayWithLength<T>::arrayWithLength(unsigned int count, bool clear)
{
    T* newData = new T[count];

    if (clear)
    {
        for (unsigned int i = 0; i < count; i++)
        {
            newData[i] = T();
        }
    }

    data = newData;
    length = count;
}

template <class T>
arrayWithLength<T>::arrayWithLength(T* values, unsigned int count)
{
    data = values;
    length = count;
}

template <class T>
arrayWithLength<T>::arrayWithLength(std::vector<T>& values)
{
    length = values.size();
    data = new T[length];

    for (unsigned int i = 0; i < length; i++)
    {
        data[i] = values[i];
    }
}

template <class T>
void arrayWithLength<T>::resize(unsigned int newLength)
{
    if (newLength != length)
    {
        T* newData = new T[newLength];

        for (unsigned int i = 0; i < newLength; i++)
        {
            newData[i] = T();
        }

        if (data != NULL)
        {
            unsigned int copyLength = newLength < length ? newLength : length;

            for (unsigned int i = 0; i < copyLength; i++)
            {
                newData[i] = data[i];
            }

            delete[] data;
        }

        length = newLength;
        data = newData;
    }
}

template <class T>
void arrayWithLength<T>::resizeNoDelete(unsigned int newLength, void** oldData)
{
    if (newLength != length)
    {
        T* newData = new T[newLength];

        for (unsigned int i = 0; i < newLength; i++)
        {
            newData[i] = T();
        }

        if (data != NULL)
        {
            unsigned int copyLength = newLength < length ? newLength : length;

            for (unsigned int i = 0; i < copyLength; i++)
            {
                newData[i] = data[i];
            }

            *oldData = data;
        }

        length = newLength;
        data = newData;
    }
}

#endif
