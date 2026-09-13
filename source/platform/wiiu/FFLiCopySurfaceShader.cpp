#include "platform/wiiu/FFLiCopySurfaceShader.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include <cstring>

#include "platform/wiiu/FFLiCopySurfaceShader_CopySurfaceVertexProgram.inc"

#include "platform/wiiu/FFLiCopySurfaceShader_CopySurfacePixelProgram.inc"

static const char CopySurfaceTextureName[] = "s_texture";
static const char CopySurfacePositionName[] = "a_position";
static const char CopySurfaceTexCoordName[] = "a_texCoord";

static _GX2AttribVar CopySurfaceVertexAttributes[2] = {
    { CopySurfacePositionName, 9, 0, 0 },
    { CopySurfaceTexCoordName, 9, 0, 1 }
};

static _GX2ShaderSampler CopySurfacePixelSamplers[1] = {
    { CopySurfaceTextureName, 1, 0 }
};

#include "platform/wiiu/FFLiCopySurfaceShader_CopySurfaceVertexShader.inc"

#include "platform/wiiu/FFLiCopySurfaceShader_CopySurfacePixelShader.inc"


struct _GX2Sampler;
extern "C" void GX2SetFetchShader(const FFLiUtilShader::FetchShader* fetchShader);
extern "C" void GX2SetVertexShader(const _GX2VertexShader* shader);
extern "C" void GX2SetPixelShader(const _GX2PixelShader* shader);
extern "C" void GX2InitSampler(_GX2Sampler* sampler, unsigned int clampMode, unsigned int minMagFilter);
extern "C" void GX2InitSamplerLOD(_GX2Sampler* sampler, float minLod, float maxLod, float lodBias);
extern "C" void GX2SetPixelSampler(const _GX2Sampler* sampler, unsigned int unit);
extern "C" void GX2SetPixelTexture(const _GX2Texture* texture, unsigned int unit);
extern void FFLiInvalidateShader__FPvUi(void* buffer, unsigned int size);

FFLiCopySurfaceShader::FFLiCopySurfaceShader()
    : mVertexShader(0), mPixelShader(0), mSamplerLocation(0)
{
    std::memset(&mFetchShader, 0, sizeof(mFetchShader));
    std::memset(mSampler, 0, sizeof(mSampler));
    mVertexShader = &CopySurfaceVertexShader;
    mPixelShader = &CopySurfacePixelShader;
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
    FFLiInvalidateShader__FPvUi(mFetchShader.program, mFetchShader.size);
    FFLiInvalidateShader__FPvUi(mVertexShader->program, mVertexShader->size);
    FFLiInvalidateShader__FPvUi(mPixelShader->program, mPixelShader->size);
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
