#ifndef PLATFORM_WIIU_FFLIUTILSHADER_HPP
#define PLATFORM_WIIU_FFLIUTILSHADER_HPP
#include <stdint.h>
struct _GX2ShaderUniformVar { const char* name; unsigned int type; unsigned int count; unsigned int offset; int block; };
struct _GX2ShaderUniformBlock { const char* name; unsigned int offset; unsigned int size; };
struct _GX2ShaderSampler { const char* name; unsigned int type; unsigned int location; };
struct _GX2UniformInitialValue { float value[4]; unsigned int offset; };
struct _GX2LoopVar { unsigned int offset; unsigned int value; };
struct _GX2AttribVar { const char* name; unsigned int type; unsigned int count; unsigned int location; };
struct _GX2RBuffer { unsigned int flags; unsigned int elemSize; unsigned int elemCount; void* buffer; };
struct _GX2PixelShader { unsigned int words[41]; unsigned int size; void* program; unsigned int mode; unsigned int uniformBlockCount; _GX2ShaderUniformBlock* uniformBlocks; unsigned int uniformVarCount; _GX2ShaderUniformVar* uniformVars; unsigned int initialValueCount; _GX2UniformInitialValue* initialValues; unsigned int loopVarCount; _GX2LoopVar* loopVars; unsigned int samplerCount; _GX2ShaderSampler* samplers; _GX2RBuffer gx2rBuffer; };
struct _GX2VertexShader { unsigned int words[52]; unsigned int size; void* program; unsigned int mode; unsigned int uniformBlockCount; _GX2ShaderUniformBlock* uniformBlocks; unsigned int uniformVarCount; _GX2ShaderUniformVar* uniformVars; unsigned int initialValueCount; _GX2UniformInitialValue* initialValues; unsigned int loopVarCount; _GX2LoopVar* loopVars; unsigned int samplerCount; _GX2ShaderSampler* samplers; unsigned int attribVarCount; _GX2AttribVar* attribVars; unsigned int ringItemsize; unsigned int hasStreamOut; unsigned int streamOutStride[4]; _GX2RBuffer gx2rBuffer; };
struct _Mat44 { float value[4][4]; };
struct _Mat34 { float value[3][4]; };
struct FFLColor
{
    union
    {
        struct { float r, g, b, a; };
        float value[4];
    };
};
typedef unsigned int FFLModulateMode;
struct FFLModulateParam;
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
struct PlainShader {
    unsigned int self;
    unsigned int contextState;
    unsigned int setMatrix;
    unsigned int setContextState;
    _GX2VertexShader* vertexShader;
    _GX2PixelShader* pixelShader;
    FFLiUtilShader::FetchShader fetchShader;
    unsigned int reserved[17];
    unsigned int vertexMatrix;
    unsigned int pixelMode;
    unsigned int pixelColor1;
    unsigned int pixelColor2;
    unsigned int pixelColor3;
    unsigned int pixelSampler;
};

unsigned int setRGBAOrder__FUi(unsigned int);
int InitDefaultShader__Fv();
void InitExtShader__Fv();
void InitFetchShader__FP11FetchShaderPC16_GX2VertexShader(FFLiUtilShader::FetchShader*, const _GX2VertexShader*);
void InitPixelShader__FPP15_GX2PixelShaderPCv(_GX2PixelShader**, const void*);
void InitShader__FP11PlainShaderPCv(PlainShader*, const void*);
void InitVertexShader__FPP16_GX2VertexShaderPCv(_GX2VertexShader**, const void*);
int InitPlainShader__FP11PlainShaderPCv(PlainShader*, const void*);
void SetupIconShaderForCreateModel__Fv();
void SetupIconShaderForDrawTexture__FUsT1(unsigned short, unsigned short);
void SetupDefaultShaderForCreateModel__Fv();
void SetupDefaultShaderForDrawTexture__FUsT1(unsigned short, unsigned short);
void SetupExtShaderForCreateModel__Fv();
void SetupExtShaderForDrawTexture__FUsT1(unsigned short, unsigned short);
void SetConstColor__FUiPC8FFLColor(unsigned int, const FFLColor*);
void SetModulateMode__FP11PlainShader15FFLModulateMode(PlainShader*, FFLModulateMode);
void BindTexture__FP11PlainShaderPC16FFLModulateParam(PlainShader*, const FFLModulateParam*);
void SetModulate__FP11PlainShaderPC16FFLModulateParam(PlainShader*, const FFLModulateParam*);
void SetMatrix__FPvPC6_Mat44(void*, const _Mat44*);
void SetMatrixPlainShader__FPC11PlainShaderPC6_Mat44(const PlainShader*, const _Mat44*);
#endif
