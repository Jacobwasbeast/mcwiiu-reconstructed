#include "platform/wiiu/WiiU_GX2.hpp"

#include <cmath>
#include <cstring>
#include <stdint.h>

extern void _GX2InitColorBuffer(_GX2ColorBuffer*, unsigned int, unsigned int, unsigned int, unsigned int);
extern void GX2CalcSurfaceSizeAndAlignment(_GX2Surface*);
extern void GX2InitDepthBufferRegs(_GX2DepthBuffer*);
extern void GX2InitTextureRegs(_GX2Texture*);
extern void OSPanic(const char*, int, const char*);
extern void C_MTXPerspective(float (*)[4], float, float, float, float);
extern void C_MTXLookAt(_Mat34*, const float*, const float*, const float*);
extern void instructionSynchronize();
extern unsigned int _GFDGetBlockCount(unsigned int, const void*);
extern unsigned int _GFDGetBlockDataSize(unsigned int, unsigned int, const void*);
extern int _GFDCheckHeaderVersions(const void*);
extern int _GFDCheckBlockHeaderMagicVersions(const void*);
extern int _GFDRelocateBlock(unsigned int, void*);

void GX2InitColorBufferFTV__FP15_GX2ColorBufferUiT217_GX2SurfaceFormat10_GX2AAMode(
    _GX2ColorBuffer* buffer, unsigned int width, unsigned int height, unsigned int format, unsigned int aaMode)
{
    buffer->surface.use = 3;
    _GX2InitColorBuffer(buffer, width, height, format, aaMode);
    if (buffer->surface.aaMode != aaMode)
    {
        _GX2Surface surface = buffer->surface;
        surface.use = 3;
        surface.aaMode = aaMode;
        GX2CalcSurfaceSizeAndAlignment(&surface);
        buffer->surface.imageSize = surface.imageSize;
        buffer->surface.alignment = surface.alignment;
    }
}

void GX2InitDepthBuffer__FP15_GX2DepthBufferUiT217_GX2SurfaceFormat10_GX2AAMode(
    _GX2DepthBuffer* buffer, unsigned int width, unsigned int height, unsigned int format, unsigned int aaMode)
{
    buffer->surface.width = width;
    buffer->surface.height = height;
    buffer->surface.format = format;
    buffer->surface.aaMode = aaMode;
    buffer->surface.dim = 1;
    buffer->surface.depth = 1;
    buffer->surface.numMips = 1;
    buffer->surface.use = (format == 0x11 || format == 0x811) ? 4 : 5;
    buffer->hiZPtr = 0;
    buffer->hiZSize = 0;
    buffer->surface.imagePtr = 0;
    buffer->surface.mipPtr = 0;
    buffer->depthClear = 1.0f;
    buffer->stencilClear = 0;
    GX2CalcSurfaceSizeAndAlignment(&buffer->surface);
    GX2InitDepthBufferRegs(buffer);
}

void GX2InitTexture__FP11_GX2TextureUiN3217_GX2SurfaceFormat14_GX2SurfaceDim(
    _GX2Texture* texture, unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips,
    unsigned int format, unsigned int dim)
{
    std::memset(texture, 0, sizeof(*texture));
    texture->surface.dim = dim;
    texture->surface.width = width;
    texture->surface.height = height;
    texture->surface.depth = depth;
    texture->surface.numMips = numMips;
    texture->surface.format = format;
    texture->surface.aaMode = 0;
    texture->surface.use = 1;
    texture->viewNumMips = numMips;
    texture->viewNumSlices = depth;
    texture->compMap = 0;
    GX2CalcSurfaceSizeAndAlignment(&texture->surface);
    GX2InitTextureRegs(texture);
}

void GetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44* projection, _Mat34* view,
                                           unsigned short width, unsigned short height)
{
    if (width == 0)
        OSPanic(__FILE__, 0x38, "Width must not be zero.");
    if (height == 0)
        OSPanic(__FILE__, 0x39, "Height must not be zero.");
    if (projection == 0)
        OSPanic(__FILE__, 0x3a, "Invalid Pointer: %x");
    if (view == 0)
        OSPanic(__FILE__, 0x3b, "Invalid Pointer: %x");

    float aspect = static_cast<float>(width) / static_cast<float>(height);
    float angle = width < height
        ? std::atan2(43.2f / aspect, 500.0f)
        : std::atan2(43.2f, 1.0f);
    C_MTXPerspective(projection->value, angle * 2.0f * 180.0f / 3.141590118408203125f,
                     aspect, 500.0f, 700.0f);
    const float eye[3] = {0.0f, 34.5f, 600.0f};
    const float target[3] = {0.0f, 0.0f, 0.0f};
    const float up[3] = {0.0f, 1.0f, 0.0f};
    C_MTXLookAt(view, eye, target, up);
    instructionSynchronize();
}

static int FindShaderEntry(const uint32_t* shader, unsigned int countOffset, unsigned int entriesOffset,
                           const char* name, unsigned int stride, unsigned int resultOffset)
{
    unsigned int count = shader[countOffset / 4];
    const uint32_t* entries = reinterpret_cast<const uint32_t*>(shader[entriesOffset / 4]);
    for (unsigned int i = 0; i < count; ++i)
    {
        const uint32_t* entry = reinterpret_cast<const uint32_t*>(reinterpret_cast<const uint8_t*>(entries) + i * stride);
        const char* entryName = reinterpret_cast<const char*>(static_cast<uintptr_t>(entry[0]));
        if (entryName != 0 && std::strcmp(entryName, name) == 0)
        {
            int location = static_cast<int>(entry[resultOffset / 4]);
            return location >= 0 ? location : -1;
        }
    }
    return -1;
}

int GetVertexUniformLocation__FPC16_GX2VertexShaderPCc(const _GX2VertexShader* shader, const char* name)
{
    if (shader == 0 || name == 0)
        return -1;
    int location = FindShaderEntry(reinterpret_cast<const uint32_t*>(shader), 0xe4, 0xe8, name, 0x14, 0xc);
    if (location < 0)
        OSPanic(__FILE__, 0x28, "loc >= 0");
    return location;
}

int GetPixelUniformLocation__FPC15_GX2PixelShaderPCc(const _GX2PixelShader* shader, const char* name)
{
    if (shader == 0 || name == 0)
        return -1;
    int location = FindShaderEntry(reinterpret_cast<const uint32_t*>(shader), 0xb8, 0xbc, name, 0x14, 0xc);
    if (location < 0)
        OSPanic(__FILE__, 0x35, "loc >= 0");
    return location;
}

unsigned int GetPixelSamplerLocation__FPC15_GX2PixelShaderPCc(const _GX2PixelShader* shader, const char* name)
{
    if (shader == 0 || name == 0)
        return static_cast<unsigned int>(-1);
    int location = FindShaderEntry(reinterpret_cast<const uint32_t*>(shader), 0xd0, 0xd4, name, 0xc, 8);
    return static_cast<unsigned int>(location);
}

static unsigned int GFDGetBlockCountForType(unsigned int type, const void* data)
{
    return _GFDGetBlockCount(type, data);
}

static unsigned int GFDGetBlockSizeForType(unsigned int type, unsigned int index, const void* data)
{
    return _GFDGetBlockDataSize(type, index, data);
}

unsigned int GFDCleanTag(unsigned int value)
{
    return value & 0xfffffU;
}

unsigned int GFDGetVertexShaderCount(const void* data)
{
    return GFDGetBlockCountForType(3, data);
}

unsigned int GFDGetPixelShaderCount(const void* data)
{
    return GFDGetBlockCountForType(6, data);
}

unsigned int GFDGetVertexShaderHeaderSize(unsigned int index, const void* data)
{
    return GFDGetBlockSizeForType(3, index, data);
}

unsigned int GFDGetPixelShaderHeaderSize(unsigned int index, const void* data)
{
    return GFDGetBlockSizeForType(6, index, data);
}

unsigned int GFDGetVertexShaderProgramSize(unsigned int index, const void* data)
{
    return GFDGetBlockSizeForType(5, index, data);
}

unsigned int GFDGetPixelShaderProgramSize(unsigned int index, const void* data)
{
    return GFDGetBlockSizeForType(7, index, data);
}

static int GFDGetShader(void* shader, void* program, unsigned int index, const void* data,
                        unsigned int shaderType, unsigned int programType, unsigned int programOffset)
{
    if (shader == 0 || program == 0 || data == 0 || !_GFDCheckHeaderVersions(data))
        return 0;
    if ((reinterpret_cast<uintptr_t>(program) & 0xffU) != 0)
        return 0;

    const uint8_t* block = static_cast<const uint8_t*>(data) + *reinterpret_cast<const uint32_t*>(static_cast<const uint8_t*>(data) + 4);
    int valid = _GFDCheckBlockHeaderMagicVersions(block);
    unsigned int shaderIndex = 0;
    unsigned int programIndex = 0;
    while (valid)
    {
        const uint8_t* next = block + *reinterpret_cast<const uint32_t*>(block + 4);
        unsigned int type = *reinterpret_cast<const uint32_t*>(block + 0x10);
        unsigned int size = *reinterpret_cast<const uint32_t*>(block + 0x14);
        if (type == shaderType)
        {
            if (index == shaderIndex)
            {
                std::memcpy(shader, next, size);
                if (!_GFDRelocateBlock(size, shader))
                    return 0;
            }
            ++shaderIndex;
        }
        else if (type == programType)
        {
            if (index == programIndex)
            {
                *reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(shader) + programOffset) =
                    reinterpret_cast<uintptr_t>(program);
                std::memcpy(program, next, size);
            }
            ++programIndex;
        }
        if ((index < shaderIndex && index < programIndex) || type == 1)
            break;
        block = next + size;
        valid = _GFDCheckBlockHeaderMagicVersions(block);
    }
    return valid && index <= shaderIndex && index <= programIndex;
}

int GFDGetVertexShader(_GX2VertexShader* shader, void* program, unsigned int index, const void* data)
{
    return GFDGetShader(shader, program, index, data, 3, 5, 0xd4);
}

int GFDGetPixelShader(_GX2PixelShader* shader, void* program, unsigned int index, const void* data)
{
    return GFDGetShader(shader, program, index, data, 6, 7, 0xa8);
}
