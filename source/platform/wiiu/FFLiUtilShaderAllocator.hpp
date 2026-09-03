#ifndef PLATFORM_WIIU_FFLIUTILSHADERALLOCATOR_HPP
#define PLATFORM_WIIU_FFLIUTILSHADERALLOCATOR_HPP

#include "platform/wiiu/FFLiBufferAllocator.hpp"

class FFLiUtilShaderAllocator
{
public:
    FFLiUtilShaderAllocator(FFLiBufferAllocator* allocator, unsigned int shaderCount, const void* shaderData, unsigned int shaderIndex);
    FFLiUtilShaderAllocator(FFLiBufferAllocator* allocator, unsigned int shaderCount);

    FFLiBufferAllocator mHeaderAllocator;
    FFLiBufferAllocator mProgramAllocator;
};

#endif
