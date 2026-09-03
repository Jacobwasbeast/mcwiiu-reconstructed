#include "platform/wiiu/FFLiCopySurfaceShader.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include <cstring>

struct _GX2Sampler;
extern void GX2SetFetchShader(const FFLiUtilShader::FetchShader* fetchShader);
extern void GX2SetVertexShader(const _GX2VertexShader* shader);
extern void GX2SetPixelShader(const _GX2PixelShader* shader);
extern void GX2InitSampler(_GX2Sampler* sampler, unsigned int clampMode, unsigned int minMagFilter);
extern void GX2InitSamplerLOD(_GX2Sampler* sampler, float minLod, float maxLod, float lodBias);
extern void GX2SetPixelSampler(const _GX2Sampler* sampler, unsigned int unit);
extern void GX2SetPixelTexture(const _GX2Texture* texture, unsigned int unit);
extern void FFLiInvalidateShader(void* buffer, unsigned int size);

FFLiCopySurfaceShader::FFLiCopySurfaceShader()
    : mVertexShader(0), mPixelShader(0), mSamplerLocation(0)
{
    std::memset(&mFetchShader, 0, sizeof(mFetchShader));
    std::memset(mSampler, 0, sizeof(mSampler));
    mVertexShader = (_GX2VertexShader*)0x104D1A74U;
    mPixelShader = (_GX2PixelShader*)0x104D1BA8U;
}

FFLiCopySurfaceShader::~FFLiCopySurfaceShader()
{
}

bool FFLiCopySurfaceShader::SetupCPU(FFLiBufferAllocator* allocator)
{
    FFLiUtilShaderAllocator shaderAllocator(allocator, 2);
    const FFLiUtilShader::AttributeStreamDesc attributes[2] = {
        { "a_position", 0, 0, 0x80dU },
        { "a_texCoord", 0, 1, 0x80dU }
    };
    if (!FFLiUtilShader::BuildShader(&mFetchShader, mVertexShader, &shaderAllocator,
                                     attributes, 2, false))
    {
        return false;
    }
    if (!FFLiUtilShader::GetSamplerLocation(&mSamplerLocation, mPixelShader, "s_texture"))
    {
        return false;
    }
    GX2InitSampler((_GX2Sampler*)mSampler, 1, 1);
    return true;
}

void FFLiCopySurfaceShader::SetupGPU()
{
    FFLiInvalidateShader((void*)mFetchShader.program, mFetchShader.words[2]);
    FFLiInvalidateShader((void*)mVertexShader->cachePtr, mVertexShader->cacheSize);
    FFLiInvalidateShader((void*)mPixelShader->cachePtr, mPixelShader->cacheSize);
}

void FFLiCopySurfaceShader::Bind() const
{
    GX2SetFetchShader(&mFetchShader);
    GX2SetVertexShader(mVertexShader);
    GX2SetPixelShader(mPixelShader);
}

void FFLiCopySurfaceShader::SetTexture(const _GX2Texture* texture, unsigned int level)
{
    const float lod = (float)level;
    GX2InitSamplerLOD((_GX2Sampler*)mSampler, lod, lod, 0.0f);
    GX2SetPixelSampler((const _GX2Sampler*)mSampler, mSamplerLocation);
    GX2SetPixelTexture(texture, mSamplerLocation);
}

unsigned int FFLiCopySurfaceShader::GetBufferSize()
{
    return (unsigned int)FFLiUtilShader::GetBufferSizeForFetch(2);
}
