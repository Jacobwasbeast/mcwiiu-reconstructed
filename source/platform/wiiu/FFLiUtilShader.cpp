#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include <cstring>
extern unsigned int FFLiRoundUp(unsigned int,unsigned int);
extern unsigned int GX2CalcFetchShaderSizeEx(unsigned int,unsigned int,unsigned int);
extern void GX2InitFetchShaderEx(FFLiUtilShader::FetchShader*,unsigned int,unsigned int,void*,unsigned int,unsigned int);
extern unsigned int GFDGetPixelShaderCount(const void*); extern unsigned int GFDGetPixelShaderHeaderSize(unsigned int,const void*); extern unsigned int GFDGetPixelShaderProgramSize(unsigned int,const void*); extern int GFDGetPixelShader(_GX2PixelShader*,void*,unsigned int,const void*);
extern unsigned int GFDGetVertexShaderCount(const void*); extern unsigned int GFDGetVertexShaderHeaderSize(unsigned int,const void*); extern unsigned int GFDGetVertexShaderProgramSize(unsigned int,const void*); extern int GFDGetVertexShader(_GX2VertexShader*,void*,unsigned int,const void*);
extern void FFLiBugEndianSwap(void*,unsigned int); extern void FFLiInvalidateShader(void*,unsigned int);

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
        if(j==s->uniformVarCount||s->uniformVars[j].location<0) return false;
    }
    GX2InitFetchShaderEx(f,z,n,p,0,0);
    FFLiBugEndianSwap(p,z);
    if (inv)
    {
        FFLiInvalidateShader((void*)f->words[3], f->words[2]);
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
    if(v) FFLiInvalidateShader((void*)s->cachePtr,s->cacheSize);
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
    if(v) FFLiInvalidateShader((void*)s->cachePtr,s->cacheSize);
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
            if (s->uniformVars[i].location < 0) return false;
            *o = s->uniformVars[i].location;
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
            if (s->uniformVars[i].location < 0) return false;
            *o = s->uniformVars[i].location;
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
            *o = s->uniformBlocks[i].location;
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
            *o = s->uniformBlocks[i].location;
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
            if (s->samplers[i].location < 0) return false;
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
            if (s->samplers[i].location < 0) return false;
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
        if (b == NULL || b->location != d[i].offset || b->size != d[i].size)
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
        if (b == NULL || b->location != d[i].offset || b->size != d[i].size)
            return false;
    }
    return true;
}
