#include "platform/wiiu/FFLiCompressorParam.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiCompressUniform.hpp"
#include "game/client/render/Renderer.hpp"

#include <new>

unsigned int FFLiGetMipMapLevelSize(unsigned int value, unsigned int level);

FFLiCompressorParam::FFLiCompressorParam()
{
    mUniformCount = 0;
    mUniforms = 0;
}

FFLiCompressorParam::~FFLiCompressorParam()
{
}

int FFLiCompressorParam::GetBufferSize(unsigned int count)
{
    return (int)(count * 0x10U + FFLiCompressUniform::GetBufferSize() * count);
}

void FFLiCompressorParam::Setup(FFLiBufferAllocator* allocator, unsigned int count, bool initialized)
{
    mUniformCount = count;
    mUniforms = (FFLiCompressUniform**)allocator->Allocate(count * 4U, 4U);
    unsigned int index;
    for (index = 0; index < count; ++index)
    {
        unsigned int address = allocator->Allocate(0xcU, 4U);
        FFLiCompressUniform* uniform = 0;
        if (address != 0)
            uniform = new ((void*)address) FFLiCompressUniform();
        mUniforms[index] = uniform;
    }
    for (index = 0; index < count; ++index)
        mUniforms[index]->Setup(allocator, initialized ? 1U : 0U);
}

void FFLiCompressorParam::SetTexture(const _GX2Texture* texture)
{
    unsigned int level;
    for (level = 0; level < texture->surface.numMips; ++level)
    {
        unsigned int width = FFLiGetMipMapLevelSize(texture->surface.width, level);
        unsigned int height = FFLiGetMipMapLevelSize(texture->surface.height, level);
        mUniforms[level]->SetResolution(width, height);
        mUniforms[level]->SetMipLevel(level);
    }
}

FFLiCompressUniform* FFLiCompressorParam::GetUniform(unsigned int index) const
{
    return mUniforms[index];
}

FFLiCompressorParam* FFLiCompressorParam::Create(unsigned int count, bool initialized, FFLiBufferAllocator* allocator)
{
    FFLiCompressorParam* param = (FFLiCompressorParam*)allocator->Allocate(8U, 4U);
    if (param != 0)
        param = new ((void*)param) FFLiCompressorParam();
    param->Setup(allocator, count, initialized);
    return param;
}
