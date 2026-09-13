#include "vendor/__N_23_FFLiFacelineTexture_cpp_cff65387.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiCompressorImpl.hpp"

#include <string.h>

struct FFLPrimitiveParam
{
    uint32_t primitiveType;
    uint32_t indexCount;
    uint32_t vertexCount;
    void* indexBuffer;
};

struct FFLAttributeBuffer
{
    uint32_t size;
    uint32_t stride;
    void* buffer;
};

struct FFLAttributeBufferParam
{
    FFLAttributeBuffer attributes[5];
};

struct FFLDrawParam
{
    FFLAttributeBufferParam attributes;
    uint32_t state[6];
    uint32_t primitiveMode;
    FFLPrimitiveParam primitive;
};

extern int FFLiGetMipMapNum(int width, int height);
extern int FFLiGetBufferRenderTexture(uint32_t width, uint32_t height,
                                      uint32_t format, uint32_t mipCount);
extern uint32_t FFLiBugCanSwapSize(uint32_t size);
extern uint32_t FFLiBugCanVgtFixedIndexSize(uint32_t size);
extern void* FFLiBugVgtFixedIndexPtr(void* value);
extern uint32_t FFLiRoundUp(uint32_t value, uint32_t alignment);
extern void FFLiInvalidate(uint32_t type, void* address, uint32_t size);
uint32_t __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(uint32_t width)
{
    return width >> 1;
}

uint32_t __N_23_FFLiFacelineTexture_cpp_cff65387::GetHeight(uint32_t height)
{
    return height >> 1;
}

uint32_t __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(uint32_t width, uint32_t height,
                                                              bool mipmaps)
{
    if (mipmaps)
        return FFLiGetMipMapNum(width, height);
    return 1;
}

uint32_t __N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureBufferSize(
    uint32_t width, uint32_t height, uint32_t mipCount, bool compressed)
{
    if (compressed)
        return FFLiCompressorImpl<FFLiCompressorDescBC1>::GetTextureBufferSize(width, height, mipCount);
    return FFLiGetBufferRenderTexture(width, height, 0x1a, mipCount);
}

int32_t __N_23_FFLiFacelineTexture_cpp_cff65387::GetDrawParamBufferSize()
{
    FFLiBugCanSwapSize(8);
    uint32_t indexSize = FFLiRoundUp(FFLiBugCanVgtFixedIndexSize(FFLiBugCanSwapSize(8)), 0x20);
    uint32_t attributeSize = FFLiRoundUp(FFLiBugCanSwapSize(0x40), 0x40);
    uint32_t primitiveSize = FFLiRoundUp(FFLiBugCanSwapSize(0x20), 0x40);
    return (int32_t)(indexSize + 0x40 + attributeSize + primitiveSize);
}

uint32_t __N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureFormat(bool compressed, bool alpha)
{
    if (compressed)
        return alpha ? 0x431 : 0x31;
    return alpha ? 0x41a : 0x1a;
}

uint32_t __N_23_FFLiFacelineTexture_cpp_cff65387::Allocate(FFLiBufferAllocator* allocator,
                                                            uint32_t size, uint32_t alignment)
{
    return allocator->Allocate(size, alignment);
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::EndianSwap(void* data, uint32_t size)
{
    (void)data;
    (void)size;
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::InitPrimitive(
    FFLPrimitiveParam* primitive, FFLiBufferAllocator* allocator)
{
    primitive->primitiveType = 6;
    primitive->indexCount = 4;
    primitive->vertexCount = 4;
    FFLiBugCanVgtFixedIndexSize(8);
    primitive->indexBuffer = FFLiBugVgtFixedIndexPtr(
        (void*)Allocate(allocator, FFLiBugCanSwapSize(FFLiBugCanVgtFixedIndexSize(8)), 0x20));
    static const uint16_t indices[4] = { 0, 1, 2, 3 };
    memcpy(primitive->indexBuffer, indices, 8);
    EndianSwap(primitive->indexBuffer, 8);
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::InitAttributes(
    FFLAttributeBufferParam* attributes, uint32_t flags, FFLiBufferAllocator* allocator)
{
    (void)flags;
    static const float positions[16] = {
        -1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f,
        1.0f, -1.0f, 0.0f, 0.0f
    };
    static const float texcoords[8] = {
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 1.0f
    };
    attributes->attributes[0].size = 0x40;
    attributes->attributes[0].stride = 0x10;
    attributes->attributes[0].buffer = (void*)Allocate(allocator, FFLiBugCanSwapSize(0x40), 0x40);
    memcpy(attributes->attributes[0].buffer, positions, 0x40);
    EndianSwap(attributes->attributes[0].buffer, 0x40);
    attributes->attributes[1].size = 0x20;
    attributes->attributes[1].stride = 8;
    attributes->attributes[1].buffer = (void*)Allocate(allocator, FFLiBugCanSwapSize(0x20), 0x40);
    memcpy(attributes->attributes[1].buffer, texcoords, 0x20);
    EndianSwap(attributes->attributes[1].buffer, 0x20);
    attributes->attributes[2].size = 0;
    attributes->attributes[2].stride = 0;
    attributes->attributes[2].buffer = 0;
    attributes->attributes[3].size = 0;
    attributes->attributes[3].stride = 0;
    attributes->attributes[3].buffer = 0;
    attributes->attributes[4].size = 0;
    attributes->attributes[4].stride = 0;
    attributes->attributes[4].buffer = 0;
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::InitDrawParamWithoutModulate(
    FFLDrawParam* drawParam, uint32_t flags, FFLiBufferAllocator* allocator)
{
    drawParam->primitiveMode = 3;
    InitPrimitive(&drawParam->primitive, allocator);
    InitAttributes(&drawParam->attributes, flags, allocator);
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::InvalidatePrimitive(FFLPrimitiveParam* primitive)
{
    FFLiInvalidate(0x41, primitive->indexBuffer, primitive->indexCount << 1);
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::InvalidateAttributes(
    FFLAttributeBufferParam* attributes)
{
    unsigned int index;
    for (index = 0; index < 5; ++index)
    {
        if (attributes->attributes[index].buffer != 0)
            FFLiInvalidate(0x41, attributes->attributes[index].buffer,
                           attributes->attributes[index].size);
    }
}

void __N_23_FFLiFacelineTexture_cpp_cff65387::InvalidateDrawParam(FFLDrawParam* drawParam)
{
    InvalidatePrimitive(&drawParam->primitive);
    InvalidateAttributes(&drawParam->attributes);
}
