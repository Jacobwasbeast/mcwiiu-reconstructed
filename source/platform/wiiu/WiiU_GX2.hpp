#ifndef SOURCE_PLATFORM_WIIU_WIIU_GX2_HPP
#define SOURCE_PLATFORM_WIIU_WIIU_GX2_HPP

#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/FFLiUtilShader.hpp"

void GX2InitColorBufferFTV__FP15_GX2ColorBufferUiT217_GX2SurfaceFormat10_GX2AAMode(
    _GX2ColorBuffer*, unsigned int, unsigned int, unsigned int, unsigned int);
void GX2InitDepthBuffer__FP15_GX2DepthBufferUiT217_GX2SurfaceFormat10_GX2AAMode(
    _GX2DepthBuffer*, unsigned int, unsigned int, unsigned int, unsigned int);
void GX2InitTexture__FP11_GX2TextureUiN3217_GX2SurfaceFormat14_GX2SurfaceDim(
    _GX2Texture*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
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
