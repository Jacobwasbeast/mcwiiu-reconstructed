#include "platform/wiiu/FFLiCompressorImpl.hpp"
#include "platform/wiiu/FFLiCompressorParam.hpp"

#include <cstring>

extern void FFLiTempSetShaderMode(int shaderMode);
extern void FFLiTempSetContextState(const _GX2ContextState* state);
extern void FFLiInvalidateColorBuffer(_GX2ColorBuffer* buffer);
extern void FFLiInvalidateTexture(_GX2Texture* texture);
extern void FFLiSetupColorBufferForBC1(_GX2ColorBuffer* buffer, _GX2Texture* texture, unsigned int level);
extern void FFLiSetupColorBufferForBC3(_GX2ColorBuffer* buffer, _GX2Texture* texture, unsigned int level);
extern unsigned int FFLiCalculateUnitTextureResolution(unsigned int resolution, unsigned int level);
extern "C" void GX2SetTargetChannelMasks(unsigned int, unsigned int, unsigned int, unsigned int,
                                     unsigned int, unsigned int, unsigned int, unsigned int);
extern "C" void GX2SetAlphaTest(int, unsigned int, float);
extern "C" void GX2SetColorControl(unsigned int, unsigned char, int, int);
extern "C" void GX2SetDepthOnlyControl(int, int, unsigned int);
extern "C" void GX2SetCullOnlyControl(unsigned int, int, int);
extern "C" void GX2SetBlendControl(unsigned int, unsigned int, unsigned int, unsigned int, int,
                               unsigned int, unsigned int, unsigned int);
extern "C" void GX2SetViewport(float, float, float, float, float, float);
extern "C" void GX2SetScissor(unsigned int, unsigned int, unsigned int, unsigned int);
extern "C" void GX2SetColorBuffer(_GX2ColorBuffer*, unsigned int);
extern "C" void GX2CalcSurfaceSizeAndAlignment(_GX2Surface*);
extern void GX2InitTextureRegs(_GX2Texture*);
extern void instructionSynchronize();

template <class Descriptor>
unsigned int FFLiCompressorImpl<Descriptor>::GetBufferSize(const void* shaderData, unsigned int shaderIndex)
{
    return FFLiContextState::GetBufferSize() + FFLiCompressShader::GetBufferSize(shaderData, shaderIndex) +
           FFLiCompressDrawer::GetBufferSize();
}

template <class Descriptor>
FFLiCompressorImpl<Descriptor>::FFLiCompressorImpl(FFLiBufferAllocator* allocator, const void* shaderData,
                                                  unsigned int shaderIndex)
    : m_contextState(), m_shader(), m_drawer()
{
    unsigned int size = GetBufferSize(shaderData, shaderIndex);
    unsigned int buffer = allocator->Allocate(size, 8);
    m_allocator.Init((void*)buffer, size);
}

template <class Descriptor>
FFLiCompressorImpl<Descriptor>::~FFLiCompressorImpl()
{
}

template <class Descriptor>
bool FFLiCompressorImpl<Descriptor>::SetupCPU(const void* shaderData, unsigned int shaderIndex)
{
    m_contextState.Initialize(&m_allocator);
    bool result = m_shader.SetupCPU(shaderData, shaderIndex, &m_allocator);
    if (result)
        m_drawer.SetupCPU(&m_allocator);
    return result;
}

template <class Descriptor>
bool FFLiCompressorImpl<Descriptor>::UseUB() const
{
    return m_shader.UseUB();
}

template <class Descriptor>
void FFLiCompressorImpl<Descriptor>::SetupGPU()
{
    m_shader.SetupGPU();
    m_drawer.SetupGPU();
    m_contextState.Setup();
    m_contextState.Bind(true);
    FFLiTempSetShaderMode(UseUB());
    GX2SetTargetChannelMasks(0xf, 0, 0, 0, 0, 0, 0, 0);
    GX2SetAlphaTest(0, 7, 0.0f);
    GX2SetColorControl(0xcc, 0xff, 0, 1);
    GX2SetDepthOnlyControl(0, 0, 1);
    GX2SetCullOnlyControl(0, 0, 1);
    GX2SetBlendControl(0, 1, 0, 0, 0, 0, 0, 0);
    m_shader.Bind();
    FFLiTempSetContextState(0);
}

template <class Descriptor>
unsigned int FFLiCompressorImpl<Descriptor>::GetTextureBufferSize(unsigned int width, unsigned int height)
{
    _GX2Texture texture;
    std::memset(&texture, 0, sizeof(texture));
    texture.surface.dim = 1;
    texture.surface.width = width;
    texture.surface.height = height;
    texture.surface.depth = 1;
    texture.surface.numMips = 1;
    texture.surface.format = Descriptor::ColorFormat;
    texture.surface.aaMode = 0;
    texture.surface.use = 1;
    GX2CalcSurfaceSizeAndAlignment(&texture.surface);
    GX2InitTextureRegs(&texture);
    return ((texture.surface.alignment << 1) & 0xfffffffeU) + texture.surface.imageSize + texture.surface.mipSize;
}

template <class Descriptor>
void FFLiCompressorImpl<Descriptor>::CompressImpl(_GX2Texture* destination, const _GX2Texture* source,
                                                  const FFLiCompressorParam* param)
{
    m_contextState.Bind(true);
    m_shader.SetTexture(source);
    m_drawer.SetAttributeBuffer(0, 1);
    for (unsigned int level = 0; level < source->surface.numMips; ++level)
    {
        if (Descriptor::ColorFormat == FFLiCompressorDescBC1::ColorFormat)
            FFLiSetupColorBufferForBC1(&m_colorBuffer, destination, level);
        else
            FFLiSetupColorBufferForBC3(&m_colorBuffer, destination, level);
        GX2SetColorBuffer(&m_colorBuffer, 0);
        unsigned int width = FFLiCalculateUnitTextureResolution(source->surface.width, level);
        unsigned int height = FFLiCalculateUnitTextureResolution(source->surface.height, level);
        GX2SetViewport(0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f);
        GX2SetScissor(0, 0, width, height);
        m_shader.SetUniform(param->GetUniform(level));
        m_drawer.Draw();
    }
    FFLiInvalidateColorBuffer(&m_colorBuffer);
    FFLiInvalidateTexture(destination);
    instructionSynchronize();
}

template class FFLiCompressorImpl<FFLiCompressorDescBC1>;
template class FFLiCompressorImpl<FFLiCompressorDescBC3>;
