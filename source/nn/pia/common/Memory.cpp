#include "nn/pia/common/Memory.hpp"

#include <cstring>

#include "nn/pia/HeapManager.hpp"
#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace common {

template <>
unsigned char* NewArray<unsigned char>(unsigned int count)
{
    unsigned char* array = static_cast<unsigned char*>(
        pead::allocateFromSeadHeap(count, HeapManager::GetHeap()));
    if (array != 0)
        std::memset(array, 0, count);
    return array;
}

template <>
void DeleteArray<unsigned char>(unsigned char* array)
{
    if (array != 0)
    {
        pead::getArraySize(array);
        pead::freeToSeadHeap(array);
    }
}

}}}
