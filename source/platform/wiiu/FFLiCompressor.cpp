#include "platform/wiiu/FFLiCompressor.hpp"

FFLiCompressor::FFLiCompressor(FFLiBufferAllocator* allocator, const void* shaderData)
    : FFLiCompressorImpl<FFLiCompressorDescBC1>(allocator, shaderData, 0),
      mCompressorBC3(allocator, shaderData, 1)
{
}

int FFLiCompressor::GetBufferSize(const void* shaderData)
{
    const int bc1Size = FFLiCompressorImpl<FFLiCompressorDescBC1>::GetBufferSize(shaderData, 0);
    const int bc3Size = FFLiCompressorImpl<FFLiCompressorDescBC3>::GetBufferSize(shaderData, 1);
    if (bc1Size != 0 && bc3Size != 0)
        return bc1Size + bc3Size;
    return 0;
}

bool FFLiCompressor::SetupCPU(const void* shaderData)
{
    if (FFLiCompressorImpl<FFLiCompressorDescBC1>::SetupCPU(shaderData, 0) &&
        mCompressorBC3.SetupCPU(shaderData, 1))
        return true;
    return false;
}

void FFLiCompressor::SetupGPU()
{
    FFLiCompressorImpl<FFLiCompressorDescBC1>::SetupGPU();
    mCompressorBC3.SetupGPU();
}

bool FFLiCompressor::UseUB() const
{
    return FFLiCompressorImpl<FFLiCompressorDescBC1>::UseUB();
}
