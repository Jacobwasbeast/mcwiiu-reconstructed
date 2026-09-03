#ifndef PLATFORM_WIIU_FFLICOMPRESSSHADER_HPP
#define PLATFORM_WIIU_FFLICOMPRESSSHADER_HPP

#include "platform/wiiu/FFLiUtilShader.hpp"

class FFLiBufferAllocator;
class FFLiCompressUniform;
struct _GX2Texture;

class FFLiCompressShader
{
public:
    FFLiCompressShader();
    ~FFLiCompressShader();
    bool UseUB() const;
    unsigned int SetupLocationForUB();
    unsigned int SetupLocationForUR();
    bool SetupLocation();
    bool SetupCPU(const void* shaderData, unsigned int shaderIndex, FFLiBufferAllocator* allocator);
    void SetupGPU();
    void Bind() const;
    void SetTexture(const _GX2Texture* texture) const;
    void SetUniformUB(const FFLiCompressUniform* uniform) const;
    void SetUniformUR(const FFLiCompressUniform* uniform) const;
    void SetUniform(const FFLiCompressUniform* uniform) const;
    static unsigned int GetBufferSize(const void* shaderData, unsigned int shaderIndex);
    void Setup(const void* shaderData, unsigned int shaderIndex, FFLiBufferAllocator* allocator);

private:
    FFLiUtilShader::FetchShader mFetchShader;
    _GX2VertexShader* mVertexShader;
    _GX2PixelShader* mPixelShader;
    unsigned int mSampler[3];
    unsigned int mVertexUniformBlock;
    unsigned int mPixelUniformBlock;
    unsigned int mTexture;
};

#endif
