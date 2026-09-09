#include "platform/wiiu/FFLiCompressUniform.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"

void FFLiSwapEndianArrayImpl__tm__2_f__FPZ1ZUi_v(float* values, unsigned int count);
extern "C" void GX2Invalidate(unsigned int invalidateType, void* address, unsigned int size);

static unsigned int FFLiCompressUniformGetBlockAlignedSize(unsigned int value)
{
    return (value + 3U) & 0xfffffffcU;
}

static void FFLiCompressUniformSetupUniformBlock(float* values, unsigned int count, unsigned int size)
{
    FFLiSwapEndianArrayImpl__tm__2_f__FPZ1ZUi_v(values, count);
    GX2Invalidate(0x44U, values, size);
}

FFLiCompressUniform::FFLiCompressUniform()
{
    mInitialized = 0;
    mResolutionUniform = 0;
    mMipUniform = 0;
}

FFLiCompressUniform::~FFLiCompressUniform()
{
}

void FFLiCompressUniform::Setup(FFLiBufferAllocator* allocator, unsigned int bufferCount)
{
    mInitialized = bufferCount != 0;
    mResolutionUniform = allocator->Allocate(0x100U, 0x100U);
    mMipUniform = allocator->Allocate(0x100U, 0x100U);
}

void FFLiCompressUniform::SetResolution(unsigned int width, unsigned int height)
{
    const unsigned int alignedWidth = FFLiCompressUniformGetBlockAlignedSize(width);
    const unsigned int alignedHeight = FFLiCompressUniformGetBlockAlignedSize(height);
    float* values = (float*)mResolutionUniform;
    values[0] = 1.0f / (float)width;
    values[1] = 1.0f / (float)height;
    values[2] = (float)alignedWidth / (float)width;
    values[3] = (float)alignedHeight / (float)height;
    if (mInitialized)
        FFLiCompressUniformSetupUniformBlock(values, 4U, 0x100U);
}

void FFLiCompressUniform::SetMipLevel(unsigned int level)
{
    float* values = (float*)mMipUniform;
    values[0] = (float)level;
    values[1] = 0.0f;
    values[2] = 0.0f;
    values[3] = 0.0f;
    if (mInitialized)
        FFLiCompressUniformSetupUniformBlock(values, 4U, 0x100U);
}

unsigned int FFLiCompressUniform::GetBufferSize()
{
    return 0x300U;
}
