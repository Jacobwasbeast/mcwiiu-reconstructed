#include "platform/wiiu/FFLiCopySurface.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/WiiU_GX2.hpp"

#include <cstring>

extern void FFLiTempSetContextState(const _GX2ContextState* state);
extern "C" void GX2SetDefaultState();
extern "C" void GX2SetDepthOnlyControl(int enable, int func, unsigned int write);
extern "C" void GX2SetTargetChannelMasks(unsigned int, unsigned int, unsigned int, unsigned int,
                                     unsigned int, unsigned int, unsigned int, unsigned int);
extern "C" void GX2CalcSurfaceSizeAndAlignment(_GX2Surface* surface);
extern "C" void GX2InitTextureRegs(_GX2Texture* texture);
extern "C" void GX2InitColorBufferRegs(_GX2ColorBuffer* buffer);
extern "C" void GX2SetColorBuffer(_GX2ColorBuffer* buffer, unsigned int target);
extern "C" void GX2SetViewport(float x, float y, float width, float height, float nearZ, float farZ);
extern "C" void GX2SetScissor(unsigned int x, unsigned int y, unsigned int width, unsigned int height);
extern unsigned int FFLiGetMipMapLevelSize(unsigned int value, unsigned int level);
extern void FFLiInvalidateSurface(_GX2Surface* surface, unsigned int type);
extern void instructionSynchronize();
extern void _GX2InitColorBuffer(_GX2ColorBuffer* buffer, unsigned int width, unsigned int height,
                                _GX2SurfaceFormat format, _GX2AAMode aaMode);

unsigned int FFLiCopySurface::GetBufferSize()
{
    return FFLiCopySurfaceShader::GetBufferSize() + FFLiCopySurfaceDrawer::GetBufferSize();
}

FFLiCopySurface::FFLiCopySurface(FFLiBufferAllocator* allocator)
{
    unsigned int size = GetBufferSize();
    unsigned int buffer = allocator->Allocate(size, 1U);
    FFLiBufferAllocator localAllocator;
    localAllocator.Init((void*)buffer, size);
    SetupCPU(&localAllocator);
    mDrawer.SetupCPU(&localAllocator);
}

FFLiCopySurface::~FFLiCopySurface()
{
}

void FFLiCopySurface::SetupGPU()
{
    FFLiCopySurfaceShader::SetupGPU();
    mDrawer.SetupGPU();
}

void FFLiCopySurface::Begin()
{
    FFLiTempSetContextState(0);
    GX2SetDefaultState();
    GX2SetDepthOnlyControl(0, 0, 0);
    GX2SetTargetChannelMasks(0xf, 0, 0, 0, 0, 0, 0, 0);
    FFLiCopySurfaceShader::Bind();
    mDrawer.SetAttributeBuffer(0, 1);
}

void FFLiCopySurface::SetupSrcSurface(const _GX2Surface* surface, unsigned int level)
{
    _GX2Texture texture;
    std::memset(&texture, 0, sizeof(texture));
    texture.surface.dim = 1;
    texture.surface.width = surface->width;
    texture.surface.height = surface->height;
    texture.surface.depth = 1;
    texture.surface.numMips = surface->numMips;
    texture.surface.format = surface->format;
    texture.surface.aaMode = 0;
    texture.surface.use = 1;
    texture.surface.imageSize = surface->imageSize;
    texture.surface.imagePtr = surface->imagePtr;
    texture.surface.mipSize = surface->mipSize;
    texture.surface.mipPtr = surface->mipPtr;
    texture.surface.tileMode = surface->tileMode;
    texture.surface.swizzle = surface->swizzle;
    texture.surface.alignment = surface->alignment;
    texture.surface.pitch = surface->pitch;
    std::memcpy(texture.surface.mipOffset, surface->mipOffset, sizeof(texture.surface.mipOffset));
    GX2CalcSurfaceSizeAndAlignment(&texture.surface);
    GX2InitTextureRegs(&texture);
    SetTexture(&texture, level);
}

void FFLiCopySurface::SetupDstSurface(_GX2Surface* surface, unsigned int level)
{
    _GX2ColorBuffer colorBuffer;
    std::memset(&colorBuffer, 0, sizeof(colorBuffer));
    _GX2InitColorBuffer(&colorBuffer, surface->width, surface->height,
                        (_GX2SurfaceFormat)surface->format, GX2_AA_MODE1X);
    colorBuffer.surface.imagePtr = surface->imagePtr;
    colorBuffer.surface.mipPtr = surface->mipPtr;
    colorBuffer.surface.imageSize = surface->imageSize;
    colorBuffer.surface.mipSize = surface->mipSize;
    colorBuffer.surface.tileMode = surface->tileMode;
    colorBuffer.surface.swizzle = surface->swizzle;
    GX2InitColorBufferRegs(&colorBuffer);
    GX2SetColorBuffer(&colorBuffer, 0);
    unsigned int width = FFLiGetMipMapLevelSize(surface->width, level);
    unsigned int height = FFLiGetMipMapLevelSize(surface->height, level);
    GX2SetViewport(0.0f, 0.0f, (float)width, (float)height, 0.0f, 1.0f);
    GX2SetScissor(0, 0, width, height);
    instructionSynchronize();
}

void FFLiCopySurface::Execute(_GX2Surface* destination, unsigned int destinationLevel,
                              const _GX2Surface* source, unsigned int sourceLevel)
{
    SetupSrcSurface(source, sourceLevel);
    SetupDstSurface(destination, destinationLevel);
    FFLiInvalidateSurface(destination, 2);
    mDrawer.Draw();
    FFLiInvalidateSurface(destination, 0x10);
}

bool FFLiCopySurface::CanInitCharModel(bool first, bool second) const
{
    return first;
}
