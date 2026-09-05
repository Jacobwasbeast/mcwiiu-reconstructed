#ifndef PLATFORM_WIIU_FFLICOMPRESSOR_HPP
#define PLATFORM_WIIU_FFLICOMPRESSOR_HPP

#include "platform/wiiu/FFLiCompressorImpl.hpp"

class FFLiCompressor : public FFLiCompressorImpl<FFLiCompressorDescBC1>
{
public:
    FFLiCompressor(FFLiBufferAllocator* allocator, const void* shaderData);
    ~FFLiCompressor();

    static int GetBufferSize(const void* shaderData);
    bool SetupCPU(const void* shaderData);
    void SetupGPU();
    bool UseUB() const;
    void Setup(const void* shaderData);

private:
    friend class FFLiCharModelCreator;

    FFLiCompressorImpl<FFLiCompressorDescBC3> mCompressorBC3;
};

#endif
