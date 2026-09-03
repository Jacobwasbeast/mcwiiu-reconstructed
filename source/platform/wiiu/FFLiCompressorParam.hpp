#ifndef PLATFORM_WIIU_FFLICOMPRESSORPARAM_HPP
#define PLATFORM_WIIU_FFLICOMPRESSORPARAM_HPP

class FFLiBufferAllocator;
class FFLiCompressUniform;
struct _GX2Texture;

class FFLiCompressorParam
{
public:
    FFLiCompressorParam();
    ~FFLiCompressorParam();

    static int GetBufferSize(unsigned int count);
    void Setup(FFLiBufferAllocator* allocator, unsigned int count, bool initialized);
    void SetTexture(const _GX2Texture* texture);
    FFLiCompressUniform* GetUniform(unsigned int index) const;
    static FFLiCompressorParam* Create(unsigned int count, bool initialized, FFLiBufferAllocator* allocator);

private:
    unsigned int mUniformCount;
    FFLiCompressUniform** mUniforms;
};

#endif
