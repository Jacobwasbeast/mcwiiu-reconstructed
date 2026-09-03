#ifndef SOURCE_VENDOR_N_13_FFLISHAPE_CPP_FE662C08_HPP
#define SOURCE_VENDOR_N_13_FFLISHAPE_CPP_FE662C08_HPP

#include <stdint.h>
#include "platform/wiiu/FFLiCoordinate.hpp"

typedef uint32_t _GX2AttribFormat;

enum FFLAttributeBufferType
{
    FFLAttributeBufferTypePosition = 0,
    FFLAttributeBufferTypeNormal = 1,
    FFLAttributeBufferTypeColor = 2,
    FFLAttributeBufferTypeTexCoord = 3,
    FFLAttributeBufferTypeTangent = 4
};

struct FFLAttributeBufferParam
{
    void* data;
    uint32_t format;
    uint32_t componentCount;
    uint32_t stride;
    uint32_t offset;
    uint32_t size;
};

struct FFLPrimitiveParam
{
    void* data;
    uint32_t count;
    uint32_t indexSize;
};

class __N_13_FFLiShape_cpp_fe662c08
{
public:
    static uint8_t GetElementType(FFLAttributeBufferType type);
    static uint32_t FormatToStride(_GX2AttribFormat format);
    static uint32_t GetStride(FFLAttributeBufferType type, unsigned int stride);
    static void EndianSwap(void* data, unsigned int size);
    static void AdjustAttribute(FFLVec4* value, unsigned int count, float scaleX, float scaleY, float scaleZ, const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate);
    static void AdjustAttributeWithoutScale(FFLiSnorm10_10_10_2* value, unsigned int count, bool flip, const FFLiCoordinate* coordinate);
    static void AdjustAttributeWithoutScale(FFLiSnorm8_8_8_8* value, unsigned int count, bool flip, const FFLiCoordinate* coordinate);
    static void AdjustAttribute(FFLVec3* value, unsigned int count, float scaleX, float scaleY, float scaleZ, const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate);
    static bool CheckForAttribute(FFLAttributeBufferType type, const FFLAttributeBufferParam& param);
    static bool CheckForPrimitive(const FFLPrimitiveParam& param);
};

#endif
