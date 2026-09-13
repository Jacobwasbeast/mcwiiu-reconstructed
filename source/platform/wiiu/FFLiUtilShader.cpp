#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include "vendor/__N_21_FFLiDefaultShader_cpp_d69dbbf4.hpp"
#include "game/client/render/Renderer.hpp"
#include <cstring>
#include <cstdlib>
extern unsigned int FFLiRoundUp(unsigned int,unsigned int);
extern "C" unsigned int GX2CalcFetchShaderSizeEx(unsigned int,unsigned int,unsigned int);
extern "C" void GX2InitFetchShaderEx(FFLiUtilShader::FetchShader*,void*,unsigned int,const _GX2AttribStream*,unsigned int,unsigned int);
extern unsigned int GFDGetPixelShaderCount(const void*); extern unsigned int GFDGetPixelShaderHeaderSize(unsigned int,const void*); extern unsigned int GFDGetPixelShaderProgramSize(unsigned int,const void*); extern int GFDGetPixelShader(_GX2PixelShader*,void*,unsigned int,const void*);
extern unsigned int GFDGetVertexShaderCount(const void*); extern unsigned int GFDGetVertexShaderHeaderSize(unsigned int,const void*); extern unsigned int GFDGetVertexShaderProgramSize(unsigned int,const void*); extern int GFDGetVertexShader(_GX2VertexShader*,void*,unsigned int,const void*);
extern void FFLiBugEndianSwap(void*,unsigned int); extern void FFLiInvalidateShader__FPvUi(void*,unsigned int);
extern const unsigned char gFFLDefaultShaderData[];
extern int FFLGetBufferSizeDefaultShader(const void*);
extern int FFLInitDefaultShader(void*, void*, const void*);
extern void FFLSetDefaultShader(void*);
extern void FFLSetShaderCallback(void*);
extern void FFLSetupInitCharModelDefaultShader(void*);
extern void FFLGetIconMatrix(_Mat44*, _Mat34*, unsigned short, unsigned short);
extern void FFLSetMatrixDefaultShader(void*, const _Mat44*, const _Mat34*);
extern void* FFLGetContextStateDefaultShader(void*);
extern void FFLSetupDrawDefaultShader(void*);
extern void GetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);
extern void ASM_MTX34To44(const _Mat34*, _Mat44*);
extern void ASM_MTX44Concat(const _Mat44*, const _Mat44*, _Mat44*);
extern void ASM_MTX44Transpose(const _Mat44*, _Mat44*);
extern "C" void GX2SetContextState(void*);
extern "C" void GX2SetFetchShader(const FFLiUtilShader::FetchShader*);
extern "C" void GX2SetVertexShader(const _GX2VertexShader*);
extern "C" void GX2SetPixelShader(const _GX2PixelShader*);
extern "C" void GX2SetupContextStateEx(void*, unsigned int);
extern "C" void GX2InitSampler(void*, unsigned int, unsigned int);
extern "C" void GX2SetPixelSampler(const void*, unsigned int);
extern "C" void GX2SetVertexSampler(const void*, unsigned int);
extern "C" void GX2SetPixelUniformReg(unsigned int, unsigned int, const void*);
extern "C" void GX2SetVertexUniformReg(unsigned int, unsigned int, const void*);
extern "C" void OSPanic(const char*, unsigned int, const char*, ...);
extern unsigned char ModulateModeToValue(unsigned int);
unsigned int setRGBAOrder__FUi(unsigned int value) { return value; }

int InitDefaultShader__Fv()
{
    if (sIconDefaultShaderBuffer == 0)
    {
        sIconDefaultShaderBuffer = std::malloc((size_t)FFLGetBufferSizeDefaultShader(gFFLDefaultShaderData) + 0x100);
        if (sIconDefaultShaderBuffer == 0) OSPanic("FFLiUtilShader.cpp", 0x54, "Invalid pointer: %x", 0);
    }
    void* aligned = (void*)(((uintptr_t)sIconDefaultShaderBuffer + 0xffu) & ~((uintptr_t)0xffu));
    if (aligned == 0) OSPanic("FFLiUtilShader.cpp", 0x5a, "Invalid pointer: %x", 0);
    int result = FFLInitDefaultShader(&sIconDefaultShader, aligned, gFFLDefaultShaderData);
    if (result != 0) OSPanic("FFLiUtilShader.cpp", 0x5d, "A default shader cannot be set up");
    FFLSetDefaultShader(&sIconDefaultShader);
    sIconShaderType = 0;
    return result;
}

void InitExtShader__Fv() { sIconShaderType = 1; FFLSetShaderCallback(&sIconExtShader); }

void InitVertexShader__FPP16_GX2VertexShaderPCv(_GX2VertexShader** out, const void* data)
{
    unsigned int header = GFDGetVertexShaderHeaderSize(0, data);
    unsigned int program = GFDGetVertexShaderProgramSize(0, data);
    *out = (_GX2VertexShader*)std::malloc(header);
    void* code = std::malloc(program);
    if (GFDGetVertexShader(*out, code, 0, data) == 0) OSPanic("FFLiUtilShader.cpp", 0x61, "GFDGetVertexShader failed");
    FFLiBugEndianSwap(code, program);
}

void InitPixelShader__FPP15_GX2PixelShaderPCv(_GX2PixelShader** out, const void* data)
{
    unsigned int header = GFDGetPixelShaderHeaderSize(0, data);
    unsigned int program = GFDGetPixelShaderProgramSize(0, data);
    *out = (_GX2PixelShader*)std::malloc(header);
    void* code = std::malloc(program);
    if (GFDGetPixelShader(*out, code, 0, data) == 0) OSPanic("FFLiUtilShader.cpp", 0x89, "GFDGetPixelShader failed");
    FFLiBugEndianSwap(code, program);
}

void InitFetchShader__FP11FetchShaderPC16_GX2VertexShader(FFLiUtilShader::FetchShader* fetch, const _GX2VertexShader* vertex)
{
    unsigned int size = GX2CalcFetchShaderSizeEx(2, 0, 0);
    fetch->program = std::malloc(size);
    GX2InitFetchShaderEx(fetch, fetch->program, 2, 0, 0, 0);
    FFLiBugEndianSwap(fetch->program, size);
    (void)vertex;
}

void InitShader__FP11PlainShaderPCv(PlainShader* shader, const void* data)
{
    InitVertexShader__FPP16_GX2VertexShaderPCv(&shader->vertexShader, data);
    InitPixelShader__FPP15_GX2PixelShaderPCv(&shader->pixelShader, data);
    InitFetchShader__FP11FetchShaderPC16_GX2VertexShader(&shader->fetchShader, shader->vertexShader);
}

int InitPlainShader__FP11PlainShaderPCv(PlainShader* shader, const void* data)
{
    shader->self = (unsigned int)(uintptr_t)shader;
    GX2SetupContextStateEx(&shader->contextState, 1);
    GX2SetContextState(&shader->contextState);
    InitShader__FP11PlainShaderPCv(shader, data);
    GX2InitSampler(&shader->pixelSampler, 1, 1);
    return 1;
}

void SetupIconShaderForCreateModel__Fv()
{
    if (sIconShaderType == 0) FFLSetupInitCharModelDefaultShader(&sIconDefaultShader);
    else if (sIconShaderType == 1) SetupExtShaderForCreateModel__Fv();
}

void SetupDefaultShaderForCreateModel__Fv() { FFLSetupInitCharModelDefaultShader(&sIconDefaultShader); }
void SetupExtShaderForCreateModel__Fv()
{
    FFLSetShaderCallback(&sIconExtShader);
    GX2SetFetchShader(&sIconExtShader.fetchShader);
    GX2SetVertexShader(sIconExtShader.vertexShader);
    GX2SetPixelShader(sIconExtShader.pixelShader);
}

void SetupDefaultShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height)
{
    _Mat44 matrix; _Mat34 affine;
    FFLGetIconMatrix(&matrix, &affine, width, height);
    FFLSetMatrixDefaultShader(&sIconDefaultShader, &matrix, &affine);
    void* contextState = FFLGetContextStateDefaultShader(&sIconDefaultShader);
    GX2SetContextState(contextState);
    FFLSetupDrawDefaultShader(&sIconDefaultShader);
}

void SetupExtShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height)
{
    _Mat44 matrix; _Mat34 affine; _Mat44 transformed;
    GetIconMatrix__FP6_Mat44P6_Mat34UsT3(&matrix, &affine, width, height);
    ASM_MTX34To44(&affine, &transformed);
    ASM_MTX44Concat(&matrix, &transformed, &transformed);
    FFLSetShaderCallback(&sIconExtShader);
    SetMatrixPlainShader__FPC11PlainShaderPC6_Mat44(&sIconExtShader, &transformed);
}

void SetupIconShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height)
{
    if (sIconShaderType == 0) SetupDefaultShaderForDrawTexture__FUsT1(width, height);
    else if (sIconShaderType == 1) SetupExtShaderForDrawTexture__FUsT1(width, height);
    else OSPanic("FFLiUtilShader.cpp", 0x61, "Invalid condition");
}

void SetConstColor__FUiPC8FFLColor(unsigned int location, const FFLColor* color)
{
    if (color == 0) OSPanic("FFLiUtilShader.cpp", 0x102, "Invalid pointer: %x", 0);
    GX2SetPixelUniformReg(location, 4, color->value);
}

void SetModulateMode__FP11PlainShader15FFLModulateMode(PlainShader* shader, FFLModulateMode mode)
{
    if (shader == 0) OSPanic("FFLiUtilShader.cpp", 0x10b, "Invalid pointer: %x", 0);
    unsigned int value[4] = { ModulateModeToValue(mode), 0, 0, 0 };
    GX2SetPixelUniformReg(shader->pixelMode, 4, value);
}

void BindTexture__FP11PlainShaderPC16FFLModulateParam(PlainShader* shader, const FFLModulateParam* param)
{
    if (param->texture != 0)
    {
        GX2SetPixelSampler(param->texture, shader->pixelSampler);
        GX2SetVertexSampler(param->texture, shader->pixelSampler);
    }
}

void SetModulate__FP11PlainShaderPC16FFLModulateParam(PlainShader* shader, const FFLModulateParam* param)
{
    if (shader == 0 || param == 0) OSPanic("FFLiUtilShader.cpp", 0x117, "Invalid pointer: %x", 0);
    BindTexture__FP11PlainShaderPC16FFLModulateParam(shader, param);
    if (param->mode == 0 || param->mode == 1 || param->mode == 2) SetConstColor__FUiPC8FFLColor(shader->pixelColor1, param->colorR);
    if (param->mode == 2) {
        SetConstColor__FUiPC8FFLColor(shader->pixelColor2, param->colorG);
        SetConstColor__FUiPC8FFLColor(shader->pixelColor3, param->colorB);
    }
    SetModulateMode__FP11PlainShader15FFLModulateMode(shader, param->mode);
}

void SetMatrix__FPvPC6_Mat44(void* shader, const _Mat44* matrix)
{
    if (shader == 0 || matrix == 0) OSPanic("FFLiUtilShader.cpp", 0x15c, "Invalid pointer: %x", 0);
    _Mat44 transposed; ASM_MTX44Transpose(matrix, &transposed);
    GX2SetVertexUniformReg(((PlainShader*)shader)->vertexMatrix, 0x10, &transposed);
}

void SetMatrixPlainShader__FPC11PlainShaderPC6_Mat44(const PlainShader* shader, const _Mat44* matrix)
{
    SetMatrix__FPvPC6_Mat44((void*)shader, matrix);
}

unsigned int FFLiUtilShader::GetProgramBufferSizeForFetch(unsigned int n)
{
    return FFLiRoundUp(GX2CalcFetchShaderSizeEx(n, 0, 0), 0x100);
}
int FFLiUtilShader::GetProgramBufferSize(unsigned int n,const void* d,unsigned int i)
{
    return GetProgramBufferSizeForFetch(n)
        + FFLiRoundUp(GFDGetPixelShaderProgramSize(i, d), 0x100)
        + FFLiRoundUp(GFDGetVertexShaderProgramSize(i, d), 0x100);
}
unsigned int FFLiUtilShader::GetHeaderBufferSizeForFetch(unsigned int n)
{
    return FFLiRoundUp(n << 5, 4);
}
int FFLiUtilShader::GetHeaderBufferSize(unsigned int n,const void* d,unsigned int i)
{
    return GetHeaderBufferSizeForFetch(n)
        + FFLiRoundUp(GFDGetPixelShaderHeaderSize(i, d), 4)
        + FFLiRoundUp(GFDGetVertexShaderHeaderSize(i, d), 4);
}
int FFLiUtilShader::GetBufferSize(unsigned int n,const void* d,unsigned int i)
{
    return GetProgramBufferSize(n, d, i) + 0x100 + GetHeaderBufferSize(n, d, i);
}
int FFLiUtilShader::GetBufferSizeForFetch(unsigned int n)
{
    return GetProgramBufferSizeForFetch(n) + 0x100 + GetHeaderBufferSizeForFetch(n);
}

bool FFLiUtilShader::BuildShader(FetchShader* fetchShader, const _GX2VertexShader* vertexShader, FFLiUtilShaderAllocator* allocator, const AttributeStreamDesc* attributes, unsigned int attributeCount, bool invalidate)
{
    static const unsigned int formatMasks[20] = {
        0x00040405, 0x00010405, 0x00040405, 0x00040405, 0x00010405,
        0x00040405, 0x00040405, 0x00010405, 0x00010405, 0x00010205,
        0x00010203, 0x00010203, 0x00010405, 0x00010405, 0x00010203,
        0x00010203, 0x00010205, 0x00010205, 0x00010203, 0x00010203
    };
    unsigned int programSize = GX2CalcFetchShaderSizeEx(attributeCount, 0, 0);
    void* program = (void*)allocator->mProgramAllocator.Allocate(programSize, 0x100);
    fetchShader->attributes = (_GX2AttribStream*)allocator->mHeaderAllocator.Allocate(attributeCount * sizeof(_GX2AttribStream), 4);
    unsigned int attributeIndex;
    for (attributeIndex = 0; attributeIndex < attributeCount; ++attributeIndex)
    {
        unsigned int vertexAttributeIndex;
        for (vertexAttributeIndex = 0; vertexAttributeIndex < vertexShader->attribVarCount; ++vertexAttributeIndex)
        {
            if (std::strcmp(vertexShader->attribVars[vertexAttributeIndex].name, attributes[attributeIndex].name) == 0)
                break;
        }
        if (vertexAttributeIndex == vertexShader->attribVarCount || static_cast<int>(vertexShader->attribVars[vertexAttributeIndex].location) < 0)
            return false;
        _GX2AttribStream& stream = fetchShader->attributes[attributeIndex];
        stream.location = vertexShader->attribVars[vertexAttributeIndex].location;
        stream.buffer = attributes[attributeIndex].offset;
        stream.offset = attributes[attributeIndex].buffer;
        stream.format = attributes[attributeIndex].format;
        stream.indexType = 0;
        stream.aluDivisor = 0;
        stream.destinationSelector = formatMasks[attributes[attributeIndex].format & 0xffu];
        stream.endianSwap = 3;
    }
    GX2InitFetchShaderEx(fetchShader, program, attributeCount, fetchShader->attributes, 0, 0);
    FFLiBugEndianSwap(program, programSize);
    if (invalidate)
        FFLiInvalidateShader__FPvUi(fetchShader->program, fetchShader->size);
    return true;
}

bool FFLiUtilShader::BuildShaderImpl(_GX2PixelShader** o,FFLiUtilShaderAllocator* a,const void* d,unsigned int i,bool v)
{
    if(i>=GFDGetPixelShaderCount(d)) return false;
    unsigned int h=GFDGetPixelShaderHeaderSize(i,d),p=GFDGetPixelShaderProgramSize(i,d);
    if(!h||!p) return false;
    _GX2PixelShader* s=(_GX2PixelShader*)a->mHeaderAllocator.Allocate(h,4);
    void* q=(void*)a->mProgramAllocator.Allocate(p,0x100);
    if(!GFDGetPixelShader(s,q,i,d)) return false;
    FFLiBugEndianSwap(q,p); *o=s;
    if(v) FFLiInvalidateShader__FPvUi(s->program,s->size);
    return true;
}
bool FFLiUtilShader::BuildShader(_GX2PixelShader** o, FFLiUtilShaderAllocator* a, const void* d, unsigned int i, bool v)
{
    return BuildShaderImpl(o, a, d, i, v);
}
bool FFLiUtilShader::BuildShaderImpl(_GX2VertexShader** o,FFLiUtilShaderAllocator* a,const void* d,unsigned int i,bool v)
{
    if(i>=GFDGetVertexShaderCount(d)) return false;
    unsigned int h=GFDGetVertexShaderHeaderSize(i,d),p=GFDGetVertexShaderProgramSize(i,d);
    if(!h||!p) return false;
    _GX2VertexShader* s=(_GX2VertexShader*)a->mHeaderAllocator.Allocate(h,4);
    void* q=(void*)a->mProgramAllocator.Allocate(p,0x100);
    if(!GFDGetVertexShader(s,q,i,d)) return false;
    FFLiBugEndianSwap(q,p); *o=s;
    if(v) FFLiInvalidateShader__FPvUi(s->program,s->size);
    return true;
}
bool FFLiUtilShader::BuildShader(_GX2VertexShader** o, FFLiUtilShaderAllocator* a, const void* d, unsigned int i, bool v)
{
    return BuildShaderImpl(o, a, d, i, v);
}

bool FFLiUtilShader::GetUniformLocation(unsigned int* o, const _GX2PixelShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformVarCount; ++i)
    {
        if (std::strcmp(s->uniformVars[i].name, n) == 0)
        {
            if (static_cast<int>(s->uniformVars[i].offset) < 0)
                return false;
            *o = s->uniformVars[i].offset;
            return true;
        }
    }
    return false;
}
bool FFLiUtilShader::GetUniformLocation(unsigned int* o, const _GX2VertexShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformVarCount; ++i)
    {
        if (std::strcmp(s->uniformVars[i].name, n) == 0)
        {
            if (static_cast<int>(s->uniformVars[i].offset) < 0)
                return false;
            *o = s->uniformVars[i].offset;
            return true;
        }
    }
    return false;
}
bool FFLiUtilShader::GetUniformBlockLocation(unsigned int* o, const _GX2PixelShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformBlockCount; ++i)
    {
        if (std::strcmp(s->uniformBlocks[i].name, n) == 0)
        {
            *o = s->uniformBlocks[i].offset;
            return true;
        }
    }
    return false;
}
bool FFLiUtilShader::GetUniformBlockLocation(unsigned int* o, const _GX2VertexShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformBlockCount; ++i)
    {
        if (std::strcmp(s->uniformBlocks[i].name, n) == 0)
        {
            *o = s->uniformBlocks[i].offset;
            return true;
        }
    }
    return false;
}
bool FFLiUtilShader::GetSamplerLocation(unsigned int* o, const _GX2PixelShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->samplerCount; ++i)
    {
        if (std::strcmp(s->samplers[i].name, n) == 0)
        {
            if (static_cast<int>(s->samplers[i].location) < 0)
                return false;
            *o = s->samplers[i].location;
            return true;
        }
    }
    return false;
}
bool FFLiUtilShader::GetSamplerLocation(unsigned int* o, const _GX2VertexShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->samplerCount; ++i)
    {
        if (std::strcmp(s->samplers[i].name, n) == 0)
        {
            *o = s->samplers[i].location;
            return true;
        }
    }
    return false;
}

const _GX2ShaderUniformBlock* FFLiUtilShader::GetUniformBlock(const _GX2PixelShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformBlockCount; ++i)
    {
        if (std::strcmp(s->uniformBlocks[i].name, n) == 0)
            return s->uniformBlocks + i;
    }
    return NULL;
}

const _GX2ShaderUniformBlock* FFLiUtilShader::GetUniformBlock(const _GX2VertexShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformBlockCount; ++i)
    {
        if (std::strcmp(s->uniformBlocks[i].name, n) == 0)
            return s->uniformBlocks + i;
    }
    return NULL;
}

const _GX2ShaderUniformVar* FFLiUtilShader::GetUniformVar(const _GX2PixelShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformVarCount; ++i)
    {
        if (std::strcmp(s->uniformVars[i].name, n) == 0)
            return s->uniformVars + i;
    }
    return NULL;
}

const _GX2ShaderUniformVar* FFLiUtilShader::GetUniformVar(const _GX2VertexShader* s, const char* n)
{
    unsigned int i;
    for (i = 0; i < s->uniformVarCount; ++i)
    {
        if (std::strcmp(s->uniformVars[i].name, n) == 0)
            return s->uniformVars + i;
    }
    return NULL;
}

bool FFLiUtilShader::CheckUniformBlock(const _GX2PixelShader* s, unsigned int, const CheckUniformBlockDesc* d, unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; ++i)
    {
        const _GX2ShaderUniformBlock* b = GetUniformBlock(s, d[i].name);
        if (b == NULL || b->offset != d[i].offset || b->size != d[i].size)
            return false;
    }
    return true;
}

bool FFLiUtilShader::CheckUniformBlock(const _GX2VertexShader* s, unsigned int, const CheckUniformBlockDesc* d, unsigned int n)
{
    unsigned int i;
    for (i = 0; i < n; ++i)
    {
        const _GX2ShaderUniformBlock* b = GetUniformBlock(s, d[i].name);
        if (b == NULL || b->offset != d[i].offset || b->size != d[i].size)
            return false;
    }
    return true;
}
