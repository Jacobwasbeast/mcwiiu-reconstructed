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

    arrayWithLength()
    {
        data = NULL;
        length = 0;
    }

    static void zero(T* values, unsigned int count)
    {
        for (unsigned int i = 0; i < count; i++)
        {
            values[i] = T();
        }
    }

    arrayWithLength(unsigned int count, bool clear)
    {
        T* newData = new T[count];

        if (clear)
        {
            zero(newData, count);
        }

        length = count;
        data = newData;
    }

    arrayWithLength(T* values, unsigned int count)
    {
        data = values;
        length = count;
    }

    arrayWithLength(std::vector<T>& values)
    {
        length = values.size();
        data = new T[length];

        for (unsigned int i = 0; i < length; i++)
        {
            data[i] = values[i];
        }
    }

    void resize(unsigned int newLength)
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

    void resizeNoDelete(unsigned int newLength, void** oldData)
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
};

#endif
