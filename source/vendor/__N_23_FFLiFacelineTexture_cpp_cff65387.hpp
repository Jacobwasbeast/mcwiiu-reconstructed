#ifndef VENDOR___N_23_FFLIFACELINETEXTURE_CPP_CFF65387_HPP
#define VENDOR___N_23_FFLIFACELINETEXTURE_CPP_CFF65387_HPP

#include <stdint.h>

class FFLiBufferAllocator;
struct FFLPrimitiveParam;
struct FFLAttributeBufferParam;
struct FFLDrawParam;

class __N_23_FFLiFacelineTexture_cpp_cff65387
{
public:
    static uint32_t GetWidth(uint32_t width);
    static uint32_t GetHeight(uint32_t height);
    static uint32_t GetNumMips(uint32_t width, uint32_t height, bool mipmaps);
    static uint32_t GetTextureBufferSize(uint32_t width, uint32_t height,
                                         uint32_t mipCount, bool compressed);
    static int32_t GetDrawParamBufferSize();
    static uint32_t GetTextureFormat(bool compressed, bool alpha);
    static uint32_t Allocate(FFLiBufferAllocator* allocator, uint32_t size,
                             uint32_t alignment);
    static void EndianSwap(void* data, uint32_t size);
    static void InitPrimitive(FFLPrimitiveParam* primitive, FFLiBufferAllocator* allocator);
    static void InitAttributes(FFLAttributeBufferParam* attributes, uint32_t flags,
                               FFLiBufferAllocator* allocator);
    static void InitDrawParamWithoutModulate(FFLDrawParam* drawParam, uint32_t flags,
                                             FFLiBufferAllocator* allocator);
    static void InvalidatePrimitive(FFLPrimitiveParam* primitive);
    static void InvalidateAttributes(FFLAttributeBufferParam* attributes);
    static void InvalidateDrawParam(FFLDrawParam* drawParam);
};

#endif
