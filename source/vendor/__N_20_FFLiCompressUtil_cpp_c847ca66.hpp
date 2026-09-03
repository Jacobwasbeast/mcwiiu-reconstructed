#ifndef SOURCE_VENDOR___N_20_FFLICOMPRESSUTIL_CPP_C847CA66_HPP
#define SOURCE_VENDOR___N_20_FFLICOMPRESSUTIL_CPP_C847CA66_HPP

#include "game/client/render/Renderer.hpp"

enum _GX2SurfaceFormat
{
    GX2_SURFACE_FORMAT_UNORM_D24_S8 = 0x011,
    GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8 = 0x01A,
    GX2_SURFACE_FORMAT_FLOAT_R16_G16_B16_A16 = 0x820
};

enum _GX2AAMode
{
    GX2_AA_MODE1X
};

class __N_20_FFLiCompressUtil_cpp_c847ca66
{
public:
    static void SetupColorBuffer(_GX2ColorBuffer* colorBuffer, _GX2Texture* texture,
                                 unsigned int mipLevel, _GX2SurfaceFormat format);
};

#endif
