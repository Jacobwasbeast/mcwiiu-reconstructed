#include "vendor/__N_20_FFLiCompressUtil_cpp_c847ca66.hpp"

#include <cstring>

unsigned int FFLiCalculateUnitTextureResolution(unsigned int resolution, unsigned int level);
void _GX2InitColorBuffer(_GX2ColorBuffer* buffer, unsigned int width, unsigned int height,
                         _GX2SurfaceFormat format, _GX2AAMode aaMode);
void GX2InitColorBufferRegs(_GX2ColorBuffer* buffer);

void __N_20_FFLiCompressUtil_cpp_c847ca66::SetupColorBuffer(_GX2ColorBuffer* colorBuffer,
                                                            _GX2Texture* texture,
                                                            unsigned int mipLevel,
                                                            _GX2SurfaceFormat format)
{
    unsigned int width = FFLiCalculateUnitTextureResolution(texture->surface.width, mipLevel);
    unsigned int height = FFLiCalculateUnitTextureResolution(texture->surface.height, mipLevel);
    std::memset(colorBuffer, 0, sizeof(_GX2ColorBuffer));
    colorBuffer->surface.use = 3U;
    _GX2InitColorBuffer(colorBuffer, width, height, format, GX2_AA_MODE1X);
    colorBuffer->surface.numMips = texture->surface.numMips;
    colorBuffer->surface.imageSize = texture->surface.imageSize;
    colorBuffer->surface.mipSize = texture->surface.mipSize;
    std::memcpy(colorBuffer->surface.mipOffset, texture->surface.mipOffset,
                sizeof(colorBuffer->surface.mipOffset));
    colorBuffer->surface.imagePtr = texture->surface.imagePtr;
    colorBuffer->viewMip = mipLevel;
    colorBuffer->surface.mipPtr = texture->surface.mipPtr;
    GX2InitColorBufferRegs(colorBuffer);
}
