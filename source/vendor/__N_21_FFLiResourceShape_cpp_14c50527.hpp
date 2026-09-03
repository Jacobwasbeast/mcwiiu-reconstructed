#ifndef SOURCE_VENDOR_N_21_FFLIRESOURCESHAPE_CPP_14C50527_HPP
#define SOURCE_VENDOR_N_21_FFLIRESOURCESHAPE_CPP_14C50527_HPP

#include <stdint.h>
#include "platform/wiiu/FFLiCoordinate.hpp"

typedef uint32_t _GX2AttribFormat;

class __N_21_FFLiResourceShape_cpp_14c50527
{
public:
    static const void* GetElement(unsigned int* elementSize, const void* data, unsigned int offset, unsigned int size);
    static void SwapEndianAttribute(void* data, unsigned int format, unsigned int count);
    static void SwapEndianIndex(void* data, unsigned int count, unsigned int indexSize);
    static void SwapEndianVec3(FFLVec3* value);
    static void SwapEndianHairTransform(void* value);
    static void SwapEndianFacelineTransform(void* value);
    static bool IsValid(unsigned int format, unsigned int componentCount, unsigned int stride, unsigned int offset, unsigned int size);
    static unsigned int GetComponentSize(_GX2AttribFormat format);
};

#endif
