#include "vendor/__N_13_FFLiShape_cpp_fe662c08.hpp"
#include <string.h>

uint8_t __N_13_FFLiShape_cpp_fe662c08::GetElementType(FFLAttributeBufferType type)
{
    static const uint8_t types[5] = { 0, 2, 1, 3, 4 };
    if (type > FFLAttributeBufferTypeTangent)
        return 0;
    return types[(unsigned int)type];
}

uint32_t __N_13_FFLiShape_cpp_fe662c08::FormatToStride(_GX2AttribFormat format)
{
    if (format > 0x080cU)
    {
        if (format == 0x080dU)
            return 8;
        if (format == 0x0811U || format == 0x0813U)
            return 16;
        return 0;
    }
    if (format != 10U && (format < 0x020aU || format > 0x020bU))
        return 0;
    return 4;
}

uint32_t __N_13_FFLiShape_cpp_fe662c08::GetStride(FFLAttributeBufferType type, unsigned int stride)
{
    _GX2AttribFormat format;
    if (type == FFLAttributeBufferTypePosition)
        format = 0x0811U;
    else if (type == FFLAttributeBufferTypeNormal)
        format = 0x080dU;
    else if (type == FFLAttributeBufferTypeColor)
        format = 0x020bU;
    else if (type == FFLAttributeBufferTypeTexCoord)
        format = 0x020aU;
    else if (type == FFLAttributeBufferTypeTangent)
        format = 10U;
    else
        return 0;
    {
        const uint32_t required = FormatToStride(format);
        return required < stride ? required : 0;
    }
}

void __N_13_FFLiShape_cpp_fe662c08::EndianSwap(void* data, unsigned int size)
{
    (void)data;
    (void)size;
}

void __N_13_FFLiShape_cpp_fe662c08::AdjustAttribute(FFLVec4* value, unsigned int count, float scaleX, float scaleY, float scaleZ, const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate)
{
    unsigned int i;
    const bool transform = !coordinate->IsDefault();
    for (i = 0; i < count; ++i)
    {
        FFLVec4* current = value + i;
        if (flip)
            current->x = FFLiCoordinate::FlipValue(current, current->x);
        if (scaleX != 1.0f) current->x *= scaleX;
        if (scaleY != 1.0f) current->y *= scaleY;
        if (scaleZ != 1.0f) current->z *= scaleZ;
        if (offset != 0)
        {
            current->x += offset->x;
            current->y += offset->y;
            current->z += offset->z;
        }
        if (transform) coordinate->Transform(current);
    }
}

void __N_13_FFLiShape_cpp_fe662c08::AdjustAttributeWithoutScale(FFLiSnorm10_10_10_2* value, unsigned int count, bool flip, const FFLiCoordinate* coordinate)
{
    unsigned int i;
    const bool transform = !coordinate->IsDefault();
    for (i = 0; i < count; ++i)
    {
        FFLiSnorm10_10_10_2* current = value + i;
        if (flip)
            current->value = (current->value & 0xfffffc00U) | (FFLiCoordinate::FlipValue(current, current->value & 0x3ffU) & 0x3ffU);
        if (transform) coordinate->TransformWithoutScale(current);
    }
}

void __N_13_FFLiShape_cpp_fe662c08::AdjustAttributeWithoutScale(FFLiSnorm8_8_8_8* value, unsigned int count, bool flip, const FFLiCoordinate* coordinate)
{
    unsigned int i;
    const bool transform = !coordinate->IsDefault();
    for (i = 0; i < count; ++i)
    {
        FFLiSnorm8_8_8_8* current = value + i;
        if (flip) current->x = (int8_t)FFLiCoordinate::FlipValue(current, current->x);
        if (transform) coordinate->TransformWithoutScale(current);
    }
}

void __N_13_FFLiShape_cpp_fe662c08::AdjustAttribute(FFLVec3* value, unsigned int count, float scaleX, float scaleY, float scaleZ, const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate)
{
    unsigned int i;
    const bool transform = !coordinate->IsDefault();
    for (i = 0; i < count; ++i)
    {
        FFLVec3* current = value + i;
        if (flip) current->x = FFLiCoordinate::FlipValue(current, current->x);
        if (scaleX != 1.0f) current->x *= scaleX;
        if (scaleY != 1.0f) current->y *= scaleY;
        if (scaleZ != 1.0f) current->z *= scaleZ;
        if (offset != 0)
        {
            current->x += offset->x;
            current->y += offset->y;
            current->z += offset->z;
        }
        if (transform) coordinate->Transform(current);
    }
}

bool __N_13_FFLiShape_cpp_fe662c08::CheckForAttribute(FFLAttributeBufferType type, const FFLAttributeBufferParam& param)
{
    const uint32_t elementSize = GetStride(type, param.stride);
    if (GetElementType(type) == 0 || param.data == 0 || param.componentCount == 0)
        return false;
    if (elementSize == 0 || param.stride < elementSize)
        return false;
    if (param.offset > param.size)
        return false;
    return param.size - param.offset >= elementSize;
}

bool __N_13_FFLiShape_cpp_fe662c08::CheckForPrimitive(const FFLPrimitiveParam& param)
{
    return param.data != 0 && param.count != 0 && (param.indexSize == 2 || param.indexSize == 4);
}
