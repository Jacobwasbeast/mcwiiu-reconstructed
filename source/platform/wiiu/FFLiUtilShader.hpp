#ifndef PLATFORM_WIIU_FFLIUTILSHADER_HPP
#define PLATFORM_WIIU_FFLIUTILSHADER_HPP
struct _GX2ShaderUniformVar { const char* name; unsigned int offset; unsigned int type; int location; unsigned int count; };
struct _GX2ShaderUniformBlock { const char* name; unsigned int location; unsigned int size; };
struct _GX2ShaderSampler { const char* name; int location; unsigned int type; };
struct _GX2PixelShader { unsigned int words[41]; unsigned int cacheSize; unsigned int cachePtr; unsigned int wordsAfterCache; unsigned int uniformBlockCount; _GX2ShaderUniformBlock* uniformBlocks; unsigned int uniformVarCount; _GX2ShaderUniformVar* uniformVars; unsigned int wordsAfterVars[4]; unsigned int samplerCount; _GX2ShaderSampler* samplers; };
struct _GX2VertexShader { unsigned int words[52]; unsigned int cacheSize; unsigned int cachePtr; unsigned int wordsAfterCache; unsigned int uniformBlockCount; _GX2ShaderUniformBlock* uniformBlocks; unsigned int uniformVarCount; _GX2ShaderUniformVar* uniformVars; unsigned int wordsAfterVars[4]; unsigned int samplerCount; _GX2ShaderSampler* samplers; };
class FFLiUtilShaderAllocator;
class FFLiUtilShader
{
public:
    struct FetchShader { unsigned int words[8]; void* program; };
    struct AttributeStreamDesc { const char* name; unsigned int offset; unsigned int buffer; unsigned int format; };
    struct CheckUniformBlockDesc { const char* name; unsigned int offset; unsigned int size; };
    static unsigned int GetProgramBufferSizeForFetch(unsigned int);
    static int GetProgramBufferSize(unsigned int, const void*, unsigned int);
    static unsigned int GetHeaderBufferSizeForFetch(unsigned int);
    static int GetHeaderBufferSize(unsigned int, const void*, unsigned int);
    static int GetBufferSize(unsigned int, const void*, unsigned int);
    static int GetBufferSizeForFetch(unsigned int);
    static bool BuildShader(FetchShader*, const _GX2VertexShader*, FFLiUtilShaderAllocator*, const AttributeStreamDesc*, unsigned int, bool);
    static bool BuildShaderImpl(_GX2PixelShader**, FFLiUtilShaderAllocator*, const void*, unsigned int, bool);
    static bool BuildShader(_GX2PixelShader**, FFLiUtilShaderAllocator*, const void*, unsigned int, bool);
    static bool BuildShaderImpl(_GX2VertexShader**, FFLiUtilShaderAllocator*, const void*, unsigned int, bool);
    static bool BuildShader(_GX2VertexShader**, FFLiUtilShaderAllocator*, const void*, unsigned int, bool);
    static bool GetUniformLocation(unsigned int*, const _GX2PixelShader*, const char*);
    static bool GetUniformLocation(unsigned int*, const _GX2VertexShader*, const char*);
    static bool GetUniformBlockLocation(unsigned int*, const _GX2PixelShader*, const char*);
    static bool GetUniformBlockLocation(unsigned int*, const _GX2VertexShader*, const char*);
    static bool GetSamplerLocation(unsigned int*, const _GX2PixelShader*, const char*);
    static bool GetSamplerLocation(unsigned int*, const _GX2VertexShader*, const char*);
    static const _GX2ShaderUniformBlock* GetUniformBlock(const _GX2PixelShader*, const char*);
    static const _GX2ShaderUniformBlock* GetUniformBlock(const _GX2VertexShader*, const char*);
    static const _GX2ShaderUniformVar* GetUniformVar(const _GX2PixelShader*, const char*);
    static const _GX2ShaderUniformVar* GetUniformVar(const _GX2VertexShader*, const char*);
    static bool CheckUniformBlock(const _GX2PixelShader*, unsigned int, const CheckUniformBlockDesc*, unsigned int);
    static bool CheckUniformBlock(const _GX2VertexShader*, unsigned int, const CheckUniformBlockDesc*, unsigned int);
};
#endif
