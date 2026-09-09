#ifndef SOURCE_PLATFORM_WIIU_WIIU_GX2_HPP
#define SOURCE_PLATFORM_WIIU_WIIU_GX2_HPP

#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/FFLiUtilShader.hpp"

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

enum _GX2SurfaceDim
{
    GX2_SURFACE_DIM_TEXTURE_1D,
    GX2_SURFACE_DIM_TEXTURE_2D
};

void GX2InitColorBufferFTV(
    _GX2ColorBuffer*, unsigned int, unsigned int, _GX2SurfaceFormat, _GX2AAMode);
void GX2InitDepthBuffer(_GX2DepthBuffer*, unsigned int, unsigned int,
                        _GX2SurfaceFormat, _GX2AAMode);
void GX2InitTexture(_GX2Texture*, unsigned int, unsigned int, unsigned int,
                    unsigned int, _GX2SurfaceFormat, _GX2SurfaceDim);
void GetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);
unsigned int GetPixelSamplerLocation__FPC15_GX2PixelShaderPCc(const _GX2PixelShader*, const char*);
int GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(const _GX2PixelShader*, const char*);
int GetVertexUniformLocation__FPC16_GX2VertexShaderPCc(const _GX2VertexShader*, const char*);
unsigned int GFDCleanTag(unsigned int);
unsigned int GFDGetVertexShaderCount(const void*);
unsigned int GFDGetPixelShaderCount(const void*);
unsigned int GFDGetVertexShaderHeaderSize(unsigned int, const void*);
unsigned int GFDGetPixelShaderHeaderSize(unsigned int, const void*);
unsigned int GFDGetVertexShaderProgramSize(unsigned int, const void*);
unsigned int GFDGetPixelShaderProgramSize(unsigned int, const void*);
int GFDGetVertexShader(_GX2VertexShader*, void*, unsigned int, const void*);
int GFDGetPixelShader(_GX2PixelShader*, void*, unsigned int, const void*);

#endif
