#ifndef SOURCE_VENDOR___N_20_FFLIRAWMASKPARTS_CPP_7C9651F3_HPP
#define SOURCE_VENDOR___N_20_FFLIRAWMASKPARTS_CPP_7C9651F3_HPP

#include <stdint.h>

#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"

struct FFLVec2
{
    float x;
    float y;
};

struct FFLiRawMaskPartsDesc
{
    float x;
    float y;
    float scaleX;
    float scaleY;
    float rotation;
};

enum FFLiOriginPosition
{
    FFLiOriginPositionTopLeft = 0,
    FFLiOriginPositionTopRight = 1,
    FFLiOriginPositionBottomLeft = 2
};

class __N_20_FFLiRawMaskParts_cpp_7c9651f3
{
public:
    static void CalcMVMatrix(_Mat34* matrix, const FFLiRawMaskPartsDesc* description);
    static uint32_t Allocate(FFLiBufferAllocator* allocator, uint32_t size, uint32_t alignment);
    static void EndianSwap(void* data, uint32_t size);
    static void InitPrimitive(FFLPrimitiveParam* primitive, FFLiBufferAllocator* allocator);
    static void CalcAttribute(FFLVec4* positions, FFLVec2* texCoords, FFLiOriginPosition origin, const _Mat44* matrix);
    static void InitAttributes(FFLAttributeBufferParam* attributes, FFLiOriginPosition origin, FFLiBufferAllocator* allocator, const _Mat44* matrix);
    static void InvalidatePrimitive(FFLPrimitiveParam* primitive);
    static void InvalidateAttributes(FFLAttributeBufferParam* attributes);
    static void InitAttributesForFill(FFLAttributeBufferParam* attributes, FFLiBufferAllocator* allocator);
};

#endif
