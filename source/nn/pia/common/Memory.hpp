#ifndef SOURCE_NN_PIA_COMMON_MEMORY_HPP
#define SOURCE_NN_PIA_COMMON_MEMORY_HPP

#include "platform/wiiu/pead.hpp"

namespace nn { namespace pia { namespace common {

template <typename T> void DeleteObj(T* object)
{
    if (object != 0)
        pead::freeToSeadHeap(object);
}

template <typename T> T* NewArray(unsigned int count);
template <typename T> void DeleteArray(T* array);

template <> unsigned char* NewArray<unsigned char>(unsigned int count);
template <> void DeleteArray<unsigned char>(unsigned char* array);

}}}

#endif
