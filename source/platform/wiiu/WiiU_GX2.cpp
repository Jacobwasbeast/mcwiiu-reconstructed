#include "platform/wiiu/WiiU_GX2.hpp"

#include <cmath>
#include <cstring>
#include <stdint.h>

extern void _GX2InitColorBuffer(_GX2ColorBuffer*, unsigned int, unsigned int,
                                _GX2SurfaceFormat, _GX2AAMode);
extern "C" void GX2CalcSurfaceSizeAndAlignment(_GX2Surface*);
extern "C" void GX2InitDepthBufferRegs(_GX2DepthBuffer*);
extern "C" void GX2InitTextureRegs(_GX2Texture*);
extern "C" unsigned int GX2TempGetGPUVersion();
extern "C" void OSPanic(const char*, int, const char*);
extern void C_MTXPerspective(float (*)[4], float, float, float, float);
extern void C_MTXLookAt(_Mat34*, const float*, const float*, const float*);
extern void instructionSynchronize();
unsigned int GFDCleanTag(unsigned int value);

void GX2InitColorBufferFTV(
    _GX2ColorBuffer* buffer, unsigned int width, unsigned int height, _GX2SurfaceFormat format, _GX2AAMode aaMode)
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

void GX2InitDepthBuffer(
    _GX2DepthBuffer* buffer, unsigned int width, unsigned int height, _GX2SurfaceFormat format, _GX2AAMode aaMode)
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

void GX2InitTexture(
    _GX2Texture* texture, unsigned int width, unsigned int height, unsigned int depth, unsigned int numMips,
    _GX2SurfaceFormat format, _GX2SurfaceDim dim)
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

static uint32_t GFDReadWord(const void* data, unsigned int offset)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data) + offset;
    return (static_cast<uint32_t>(bytes[0]) << 24) |
           (static_cast<uint32_t>(bytes[1]) << 16) |
           (static_cast<uint32_t>(bytes[2]) << 8) | bytes[3];
}

#ifdef RECON_NATIVE
static bool GFDNativeShaderHeader(void* data, unsigned int size)
{
    if (size < 0x28 || (size & 3) != 0)
        return false;
    uint8_t* bytes = static_cast<uint8_t*>(data);
    const unsigned int descriptor = size - 0x28;
    if (GFDReadWord(bytes, descriptor) != 0x7d424c4b ||
        GFDReadWord(bytes, descriptor + 4) != 0x28)
        return false;
    const unsigned int textSize = GFDReadWord(bytes, descriptor + 0x14);
    const unsigned int textBegin = GFDCleanTag(GFDReadWord(bytes, descriptor + 0x18));
    const unsigned int patchCount = GFDReadWord(bytes, descriptor + 0x20);
    const unsigned int patchBegin = GFDCleanTag(GFDReadWord(bytes, descriptor + 0x24));
    if ((textBegin & 3) != 0 || (textSize & 3) != 0 ||
        textBegin > descriptor || textSize > descriptor - textBegin ||
        (patchBegin & 3) != 0 || patchBegin > descriptor ||
        patchCount > (descriptor - patchBegin) / 4 ||
        (patchBegin < textBegin + textSize && patchBegin + patchCount * 4 > textBegin))
        return false;
    for (unsigned int index = 0; index < patchCount; ++index)
    {
        const uint32_t patch = GFDReadWord(bytes, patchBegin + index * 4);
        if (patch == 0)
            continue;
        const unsigned int target = GFDCleanTag(patch);
        if ((target & 3) != 0 || target >= descriptor ||
            (target >= textBegin && target < textBegin + textSize) ||
            GFDCleanTag(GFDReadWord(bytes, target)) >= descriptor)
            return false;
    }
    for (unsigned int offset = 0; offset < size; offset += 4)
    {
        if (offset >= textBegin && offset < textBegin + textSize)
            continue;
        const uint32_t value = GFDReadWord(bytes, offset);
        std::memcpy(bytes + offset, &value, sizeof(value));
    }
    return true;
}
#endif

int _GFDCheckBlockHeaderMagicVersions(const void* data)
{
    const uint32_t* block = static_cast<const uint32_t*>(data);
    if (block == 0 || GFDReadWord(block, 0) != 0x424C4B7B)
        return 0;
    return GFDReadWord(block, 8) == 1 ? 1 : 0;
}

int _GFDCheckHeaderVersions(const void* data)
{
    if (data == 0)
        return 0;
    const uint32_t* header = static_cast<const uint32_t*>(data);
    if (GFDReadWord(header, 0) != 0x47667832 || GFDReadWord(header, 8) != 7 || GFDReadWord(header, 12) > 1)
        return 0;
    return GFDReadWord(header, 16) == GX2TempGetGPUVersion() ? 1 : 0;
}

int _GFDRelocateBlockEx(void* descriptor, unsigned int unused,
                        intptr_t relocationBase, void* relocationData)
{
    if (descriptor == 0 || relocationData == 0)
        return 0;
    uint8_t* bytes = static_cast<uint8_t*>(descriptor);
    uint32_t* relocationTable = static_cast<uint32_t*>(relocationData);
    const uint32_t tag = GFDCleanTag(*reinterpret_cast<const uint32_t*>(bytes + 0x24));
    const uint32_t count = *reinterpret_cast<const uint32_t*>(bytes + 0x20);
    for (uint32_t index = 0; index < count; ++index)
    {
        uint32_t* patch = relocationTable + (tag / sizeof(uint32_t)) + index;
        if (*patch != 0)
        {
            uint32_t* target = reinterpret_cast<uint32_t*>(static_cast<uint8_t*>(relocationData) + GFDCleanTag(*patch));
            const int32_t value = static_cast<int32_t>(GFDCleanTag(*target));
            *target = static_cast<uint32_t>(value - static_cast<int32_t>(unused) +
                                            static_cast<int32_t>(relocationBase));
        }
    }
    *reinterpret_cast<uint32_t*>(bytes + 0x1c) = static_cast<uint32_t>(relocationBase);
    return 1;
}

int _GFDRelocateBlock(unsigned int size, void* base)
{
    if (base == 0)
        return 0;
    uint8_t* bytes = static_cast<uint8_t*>(base);
    if (*reinterpret_cast<uint32_t*>(bytes + size - 0x0c) != 0)
        return 1;
    uint8_t* descriptor = bytes + size - 0x28;
    return _GFDRelocateBlockEx(descriptor, 0, reinterpret_cast<intptr_t>(base), base);
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

unsigned int _GFDGetBlockCount(unsigned int type, const void* data)
{
    if (data == 0 || _GFDCheckHeaderVersions(data) == 0)
        return 0;

    const uint8_t* block = static_cast<const uint8_t*>(data) +
                           GFDReadWord(data, 4);
    unsigned int count = 0;
    while (_GFDCheckBlockHeaderMagicVersions(block) != 0)
    {
        const uint32_t blockType = GFDReadWord(block, 0x10);
        const uint32_t blockHeaderSize = GFDReadWord(block, 4);
        const uint32_t blockDataSize = GFDReadWord(block, 0x14);
        if (blockType == type)
            ++count;
        if (blockType == 1)
            break;
        block += blockHeaderSize + blockDataSize;
    }
    return count;
}

unsigned int _GFDGetBlockDataSize(unsigned int type, unsigned int index, const void* data)
{
    if (data == 0 || _GFDCheckHeaderVersions(data) == 0)
        return 0;

    const uint8_t* block = static_cast<const uint8_t*>(data) +
                           GFDReadWord(data, 4);
    unsigned int current = 0;
    while (_GFDCheckBlockHeaderMagicVersions(block) != 0)
    {
        const uint32_t blockType = GFDReadWord(block, 0x10);
        const uint32_t blockHeaderSize = GFDReadWord(block, 4);
        const uint32_t blockDataSize = GFDReadWord(block, 0x14);
        if (blockType == type)
        {
            if (current == index)
                return blockDataSize;
            ++current;
        }
        if (blockType == 1)
            break;
        block += blockHeaderSize + blockDataSize;
    }
    return 0;
}

unsigned int GFDCleanTag(unsigned int value)
{
    return value & 0xfffffU;
}

unsigned int GFDGetVertexShaderCount(const void* data)
{
    return _GFDGetBlockCount(3, data);
}

unsigned int GFDGetPixelShaderCount(const void* data)
{
    return _GFDGetBlockCount(6, data);
}

unsigned int GFDGetVertexShaderHeaderSize(unsigned int index, const void* data)
{
    return _GFDGetBlockDataSize(3, index, data);
}

unsigned int GFDGetPixelShaderHeaderSize(unsigned int index, const void* data)
{
    return _GFDGetBlockDataSize(6, index, data);
}

unsigned int GFDGetVertexShaderProgramSize(unsigned int index, const void* data)
{
    return _GFDGetBlockDataSize(5, index, data);
}

unsigned int GFDGetPixelShaderProgramSize(unsigned int index, const void* data)
{
    return _GFDGetBlockDataSize(7, index, data);
}

static int GFDGetShader(void* shader, void* program, unsigned int index, const void* data,
                        unsigned int shaderType, unsigned int programType, unsigned int programOffset)
{
    if (shader == 0 || program == 0 || data == 0 || !_GFDCheckHeaderVersions(data))
        return 0;
    if ((reinterpret_cast<uintptr_t>(program) & 0xffU) != 0)
        return 0;

    const uint8_t* block = static_cast<const uint8_t*>(data) + GFDReadWord(data, 4);
    int valid = _GFDCheckBlockHeaderMagicVersions(block);
    unsigned int shaderIndex = 0;
    unsigned int programIndex = 0;
    while (valid)
    {
        const uint8_t* next = block + GFDReadWord(block, 4);
        unsigned int type = GFDReadWord(block, 0x10);
        unsigned int size = GFDReadWord(block, 0x14);
        if (type == shaderType)
        {
            if (index == shaderIndex)
            {
                std::memcpy(shader, next, size);
#ifdef RECON_NATIVE
                if (!GFDNativeShaderHeader(shader, size))
                    return 0;
#endif
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
