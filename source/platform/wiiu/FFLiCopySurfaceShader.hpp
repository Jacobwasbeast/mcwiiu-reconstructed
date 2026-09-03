#ifndef PLATFORM_WIIU_FFLICOPYSURFACESHADER_HPP
#define PLATFORM_WIIU_FFLICOPYSURFACESHADER_HPP

#include "platform/wiiu/FFLiUtilShader.hpp"

class FFLiBufferAllocator;
struct _GX2Texture;

class FFLiCopySurfaceShader
{
public:
    FFLiCopySurfaceShader();
    ~FFLiCopySurfaceShader();
    bool SetupCPU(FFLiBufferAllocator* allocator);
    void SetupGPU();
    void Bind() const;
    void SetTexture(const _GX2Texture* texture, unsigned int level);
    static unsigned int GetBufferSize();

private:
    FFLiUtilShader::FetchShader mFetchShader;
    _GX2VertexShader* mVertexShader;
    _GX2PixelShader* mPixelShader;
    unsigned int mSampler[3];
    unsigned int mSamplerLocation;
};

#endif
