#ifndef PLATFORM_WIIU_FFLICOMPRESSUNIFORM_HPP
#define PLATFORM_WIIU_FFLICOMPRESSUNIFORM_HPP

class FFLiBufferAllocator;

class FFLiCompressUniform
{
public:
    FFLiCompressUniform();
    ~FFLiCompressUniform();
    void Setup(FFLiBufferAllocator* allocator, unsigned int bufferCount);
    void SetResolution(unsigned int width, unsigned int height);
    void SetMipLevel(unsigned int level);
    static unsigned int GetBufferSize();

private:
    unsigned char mInitialized;
    unsigned char mReserved[3];
    unsigned int mResolutionUniform;
    unsigned int mMipUniform;
};

#endif
