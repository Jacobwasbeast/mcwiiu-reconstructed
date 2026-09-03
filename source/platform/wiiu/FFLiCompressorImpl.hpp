#ifndef PLATFORM_WIIU_FFLICOMPRESSORIMPL_HPP
#define PLATFORM_WIIU_FFLICOMPRESSORIMPL_HPP

#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiContextState.hpp"
#include "platform/wiiu/FFLiCompressShader.hpp"
#include "platform/wiiu/FFLiCompressDrawer.hpp"

class FFLiCompressorParam;

struct FFLiCompressorDescBC1
{
    static const unsigned int ColorFormat = 0x11f;
};

struct FFLiCompressorDescBC3
{
    static const unsigned int ColorFormat = 0x122;
};

template <class Descriptor>
class FFLiCompressorImpl
{
public:
    FFLiCompressorImpl(FFLiBufferAllocator* allocator, const void* shaderData, unsigned int shaderIndex);
    ~FFLiCompressorImpl();
    static unsigned int GetBufferSize(const void* shaderData, unsigned int shaderIndex);
    bool SetupCPU(const void* shaderData, unsigned int shaderIndex);
    bool UseUB() const;
    void SetupGPU();
    static unsigned int GetTextureBufferSize(unsigned int width, unsigned int height);
    void CompressImpl(_GX2Texture* destination, const _GX2Texture* source, const FFLiCompressorParam* param);

private:
    FFLiBufferAllocator m_allocator;
    FFLiContextState m_contextState;
    FFLiCompressShader m_shader;
    FFLiCompressDrawer m_drawer;
    _GX2ColorBuffer m_colorBuffer;
};

#endif
