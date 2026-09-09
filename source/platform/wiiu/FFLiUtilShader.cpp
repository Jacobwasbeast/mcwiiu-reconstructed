#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include <cstring>
#include <cstdlib>
extern unsigned int FFLiRoundUp(unsigned int,unsigned int);
extern "C" unsigned int GX2CalcFetchShaderSizeEx(unsigned int,unsigned int,unsigned int);
extern "C" void GX2InitFetchShaderEx(FFLiUtilShader::FetchShader*,unsigned int,unsigned int,void*,unsigned int,unsigned int);
extern unsigned int GFDGetPixelShaderCount(const void*); extern unsigned int GFDGetPixelShaderHeaderSize(unsigned int,const void*); extern unsigned int GFDGetPixelShaderProgramSize(unsigned int,const void*); extern int GFDGetPixelShader(_GX2PixelShader*,void*,unsigned int,const void*);
extern unsigned int GFDGetVertexShaderCount(const void*); extern unsigned int GFDGetVertexShaderHeaderSize(unsigned int,const void*); extern unsigned int GFDGetVertexShaderProgramSize(unsigned int,const void*); extern int GFDGetVertexShader(_GX2VertexShader*,void*,unsigned int,const void*);
extern void FFLiBugEndianSwap(void*,unsigned int); extern void FFLiInvalidateShader__FPvUi(void*,unsigned int);
extern int FFLGetBufferSizeDefaultShader(const char*);
extern int FFLInitDefaultShader(void*, void*, const char*);
extern void FFLSetDefaultShader(void*);
extern void FFLSetShaderCallback(void*);
extern void FFLSetupInitCharModelDefaultShader(void*);
extern void FFLGetIconMatrix(_Mat44*, _Mat34*, unsigned short, unsigned short);
extern void FFLSetMatrixDefaultShader(void*, const _Mat44*, const _Mat34*);
extern void FFLGetContextStateDefaultShader(void*);
extern void FFLSetupDrawDefaultShader(void*);
extern void GetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);
extern void ASM_MTX34To44(const _Mat34*, _Mat44*);
extern void ASM_MTX44Concat(const _Mat44*, const _Mat44*, _Mat44*);
extern void ASM_MTX44Transpose(const _Mat44*, _Mat44*);
extern "C" void GX2SetContextState();
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
static PlainShader gDefaultShader;
static PlainShader gExtShader;
static unsigned int gShaderType;

unsigned int setRGBAOrder__FUi(unsigned int value) { return value; }

int InitDefaultShader__Fv()
{
    static void* buffer;
    if (buffer == 0) buffer = std::malloc((size_t)FFLGetBufferSizeDefaultShader("Gfx2") + 0x100);
    void* aligned = (void*)(((uintptr_t)buffer + 0xffu) & ~((uintptr_t)0xffu));
    int result = FFLInitDefaultShader(&gDefaultShader, aligned, "Gfx2");
    if (result != 0) OSPanic("FFLiUtilShader.cpp", 0x5d, "A default shader cannot be set up");
    FFLSetDefaultShader(&gDefaultShader);
    gShaderType = 0;
    return result;
}

void InitExtShader__Fv() { gShaderType = 1; FFLSetShaderCallback(&gExtShader); }

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
    GX2InitFetchShaderEx(fetch, size, 2, fetch->program, 0, 0);
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
    GX2SetContextState();
    InitShader__FP11PlainShaderPCv(shader, data);
    GX2InitSampler(&shader->pixelSampler, 1, 1);
    return 1;
}

void SetupIconShaderForCreateModel__Fv()
{
    if (gShaderType == 0) FFLSetupInitCharModelDefaultShader(&gDefaultShader);
    else if (gShaderType == 1) FFLSetShaderCallback(&gExtShader);
}

void SetupDefaultShaderForCreateModel__Fv() { FFLSetupInitCharModelDefaultShader(&gDefaultShader); }
void SetupExtShaderForCreateModel__Fv() { FFLSetShaderCallback(&gExtShader); }

void SetupDefaultShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height)
{
    _Mat44 matrix; _Mat34 affine;
    FFLGetIconMatrix(&matrix, &affine, width, height);
    FFLSetMatrixDefaultShader(&gDefaultShader, &matrix, &affine);
    FFLGetContextStateDefaultShader(&gDefaultShader);
    GX2SetContextState();
    FFLSetupDrawDefaultShader(&gDefaultShader);
}

void SetupExtShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height)
{
    _Mat44 matrix; _Mat34 affine; _Mat44 transformed;
    GetIconMatrix__FP6_Mat44P6_Mat34UsT3(&matrix, &affine, width, height);
    ASM_MTX34To44(&affine, &transformed);
    ASM_MTX44Concat(&matrix, &transformed, &transformed);
    FFLSetShaderCallback(&gExtShader);
    SetMatrixPlainShader__FPC11PlainShaderPC6_Mat44(&gExtShader, &transformed);
}

void SetupIconShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height)
{
    if (gShaderType == 0) SetupDefaultShaderForDrawTexture__FUsT1(width, height);
    else if (gShaderType == 1) SetupExtShaderForDrawTexture__FUsT1(width, height);
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

bool FFLiUtilShader::BuildShader(FetchShader* f,const _GX2VertexShader* s,FFLiUtilShaderAllocator* a,const AttributeStreamDesc* d,unsigned int n,bool inv)
{
    unsigned int z=GX2CalcFetchShaderSizeEx(n,0,0);
    void* p=(void*)a->mProgramAllocator.Allocate(z,0x100);
    f->program=(void*)a->mHeaderAllocator.Allocate(n<<5,4);
    unsigned int i;
    for(i=0;i<n;++i) {
        unsigned int j;
        for(j=0;j<s->uniformVarCount;++j) if(std::strcmp(s->uniformVars[j].name,d[i].name)==0) break;
        if(j==s->uniformVarCount) return false;
    }
    GX2InitFetchShaderEx(f,z,n,p,0,0);
    FFLiBugEndianSwap(p,z);
    if (inv)
    {
        FFLiInvalidateShader__FPvUi((void*)f->words[3], f->words[2]);
    }
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
