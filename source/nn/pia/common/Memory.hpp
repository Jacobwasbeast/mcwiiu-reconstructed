#ifndef SOURCE_NN_PIA_COMMON_MEMORY_HPP
#define SOURCE_NN_PIA_COMMON_MEMORY_HPP

#include <new>

#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace common {

template <typename T> void DeleteObj(T* object)
{
    if (object != 0)
        pead::freeToSeadHeap(object);
}

template <typename T> T* NewArray(unsigned int count)
{
    void* buffer = pead::allocateFromSeadHeap(sizeof(T) * count, HeapManager::GetHeap());
    if (buffer == 0)
        return 0;
    T* array = static_cast<T*>(buffer);
    for (unsigned int index = 0; index < count; ++index)
        new (&array[index]) T();
    return array;
}

template <typename T> void DeleteArray(T* array)
{
    if (array != 0)
    {
        unsigned int size = pead::getArraySize(array);
        unsigned int count = size / sizeof(T);
        for (unsigned int index = 0; index < count; ++index)
            array[index].~T();
        pead::freeToSeadHeap(array);
    }
}

}}}

#endif
