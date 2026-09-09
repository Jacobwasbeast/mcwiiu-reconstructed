#include "platform/wiiu/FFLiCompressShader.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#define private public
#include "platform/wiiu/FFLiCompressUniform.hpp"
#undef private
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include <cstring>

struct _GX2Sampler;
extern "C" void GX2SetFetchShader(const FFLiUtilShader::FetchShader* fetchShader);
extern "C" void GX2SetVertexShader(const _GX2VertexShader* shader);
extern "C" void GX2SetPixelShader(const _GX2PixelShader* shader);
extern "C" void GX2SetPixelSampler(const _GX2Sampler* sampler, unsigned int unit);
extern "C" void GX2SetPixelTexture(const _GX2Texture* texture, unsigned int unit);
extern "C" void GX2SetVertexUniformBlock(unsigned int offset, unsigned int count, const void* values);
extern "C" void GX2SetPixelUniformBlock(unsigned int offset, unsigned int count, const void* values);
extern "C" void GX2SetVertexUniformReg(unsigned int offset, unsigned int count, const void* values);
extern "C" void GX2SetPixelUniformReg(unsigned int offset, unsigned int count, const void* values);
extern void FFLiInvalidateShader__FPvUi(void* buffer, unsigned int size);

FFLiCompressShader::FFLiCompressShader()
    : mVertexShader(0), mPixelShader(0), mVertexUniformBlock(0), mPixelUniformBlock(0), mTexture(0)
{
    std::memset(&mFetchShader, 0, sizeof(mFetchShader));
    std::memset(mSampler, 0, sizeof(mSampler));
}

FFLiCompressShader::~FFLiCompressShader()
{
}

bool FFLiCompressShader::UseUB() const
{
    return mVertexShader->mode == 1;
}

unsigned int FFLiCompressShader::SetupLocationForUB()
{
    if (!FFLiUtilShader::GetUniformBlockLocation(&mVertexUniformBlock, mVertexShader, "cVertexBlock")) return 0;
    if (!FFLiUtilShader::GetUniformBlockLocation(&mPixelUniformBlock, mPixelShader, "cPixelBlock")) return 0;
    return 1;
}

unsigned int FFLiCompressShader::SetupLocationForUR()
{
    FFLiUtilShader::GetUniformLocation(&mVertexUniformBlock, mVertexShader, "cTexSizeRCP");
    FFLiUtilShader::GetUniformLocation(&mPixelUniformBlock, mPixelShader, "cReferLevel");
    return 1;
}

bool FFLiCompressShader::SetupLocation()
{
    if (!FFLiUtilShader::GetSamplerLocation(&mTexture, mPixelShader, "cTexture")) return false;
    if (UseUB()) return SetupLocationForUB() != 0;
    return SetupLocationForUR() != 0;
}

bool FFLiCompressShader::SetupCPU(const void* shaderData, unsigned int shaderIndex, FFLiBufferAllocator* allocator)
{
    FFLiUtilShaderAllocator shaderAllocator(allocator, 2, shaderData, shaderIndex);
    if (!FFLiUtilShader::BuildShader(&mVertexShader, &shaderAllocator, shaderData, shaderIndex, false)) return false;
    if (!FFLiUtilShader::BuildShader(&mPixelShader, &shaderAllocator, shaderData, shaderIndex, false)) return false;
    FFLiUtilShader::AttributeStreamDesc attributes[2] = { { "aPosition", 0, 0, 0x811 }, { 0, 0, 0, 0 } };
    if (!FFLiUtilShader::BuildShader(&mFetchShader, mVertexShader, &shaderAllocator, attributes, 2, false)) return false;
    return SetupLocation();
}

void FFLiCompressShader::SetupGPU()
{
    FFLiInvalidateShader__FPvUi((void*)mFetchShader.program, mFetchShader.words[2]);
    FFLiInvalidateShader__FPvUi(mVertexShader->program, mVertexShader->size);
    FFLiInvalidateShader__FPvUi(mPixelShader->program, mPixelShader->size);
}

void FFLiCompressShader::Bind() const
{
    GX2SetFetchShader(&mFetchShader);
    GX2SetVertexShader(mVertexShader);
    GX2SetPixelShader(mPixelShader);
}

void FFLiCompressShader::SetTexture(const _GX2Texture* texture) const
{
    GX2SetPixelSampler((const _GX2Sampler*)mSampler, mTexture);
    GX2SetPixelTexture(texture, mTexture);
}

void FFLiCompressShader::SetUniformUB(const FFLiCompressUniform* uniform) const
{
    GX2SetVertexUniformBlock(mVertexUniformBlock, 0x10, (const void*)uniform->mResolutionUniform);
    GX2SetPixelUniformBlock(mPixelUniformBlock, 0x10, (const void*)uniform->mMipUniform);
}

void FFLiCompressShader::SetUniformUR(const FFLiCompressUniform* uniform) const
{
    GX2SetVertexUniformReg(mVertexUniformBlock, 4, (const void*)uniform->mResolutionUniform);
    GX2SetPixelUniformReg(mPixelUniformBlock, 4, (const void*)uniform->mMipUniform);
}

void FFLiCompressShader::SetUniform(const FFLiCompressUniform* uniform) const
{
    if (UseUB()) SetUniformUB(uniform);
    else SetUniformUR(uniform);
}

unsigned int FFLiCompressShader::GetBufferSize(const void* shaderData, unsigned int shaderIndex)
{
    return FFLiUtilShader::GetBufferSize(2, shaderData, shaderIndex);
}

void FFLiCompressShader::Setup(const void* shaderData, unsigned int shaderIndex, FFLiBufferAllocator* allocator)
{
    if (SetupCPU(shaderData, shaderIndex, allocator)) SetupGPU();
}
