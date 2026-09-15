#ifndef SOURCE_VENDOR___N_21_FFLIDEFAULTSHADER_CPP_D69DBBF4_HPP
#define SOURCE_VENDOR___N_21_FFLIDEFAULTSHADER_CPP_D69DBBF4_HPP

#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "game/client/render/Renderer.hpp"
#include <stdint.h>
#include <stddef.h>

typedef uint32_t _GX2FrontFaceMode;
typedef uint32_t _GX2Boolean;
typedef uint32_t FFLCullMode;
typedef uint32_t FFLModulateMode;
typedef uint32_t FFLDrawType;
typedef uint32_t FFLSpecularMode;

struct FFLiDefaultShaderLight { uint32_t value[16]; };
struct FFLiDefaultShaderMaterial { uint32_t value[14]; };
struct FFLiDefaultShaderParam
{
    uint32_t matrix[16];
    uint32_t matrixView[16];
    uint32_t matrixNormal[12];
    FFLiDefaultShaderLight light;
    FFLiDefaultShaderMaterial material[9];
    uint32_t rim[6];
};

struct _GX2ContextState;

class __N_21_FFLiDefaultShader_cpp_d69dbbf4
{
public:
    static void SetCullingModeWraper(_GX2FrontFaceMode mode, _GX2Boolean cullFront, _GX2Boolean cullBack);
    static void SetupCulling(FFLCullMode mode, const FFLModulateParam& param, int enabled);
    static void BindTexture(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLModulateParam& param);
    static void SetupConstColor(unsigned int location, const FFLColor& color);
    static uint8_t ModulateModeToValue(FFLModulateMode mode);
    static void SetupModulateMode(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, FFLModulateMode mode);
    static void SetupModulate(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLModulateParam& param);
    static void SetupMaterial(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLDrawParam& param);
    static void ShaderCallbackDraw(void* shader, const FFLDrawParam* param);
    static void ShaderCallbackSetMatrix(void* shader, const _Mat44* matrix);
    static void Identity(float (*matrix)[4]);
    static void InitLightParam(FFLiDefaultShaderLight* param);
    static void InitMaterialParam(FFLiDefaultShaderMaterial* param, FFLDrawType type);
    static void InitParam(FFLiDefaultShaderParam* param);
    static void SetUniformLightDisable(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader);
    static void SetupUniformLight(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader);
    static void SetupUniformParam(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader);
    static void Mat44to34(_Mat34* destination, const _Mat44* source);
    static void StoreColumnMajor(_Mat44* destination, const _Mat44& source);
    static void StoreColumnMajor(float (*destination)[4], const _Mat44& source);
    static FFLiDefaultShaderLight* GetLightParam(FFLiDefaultShaderParam* param);
    static FFLiDefaultShaderMaterial* GetMaterialParam(FFLiDefaultShaderParam* param, FFLDrawType type);

    void* m_callbackData;
    _GX2ContextState* m_contextState;
    void (*m_drawCallback)(void*, const FFLDrawParam*);
    void (*m_matrixCallback)(void*, const _Mat44*);
    _GX2VertexShader* m_vertexShader;
    _GX2PixelShader* m_pixelShader;
    FFLiUtilShader::FetchShader m_fetchShader;
    _GX2Sampler m_sampler;
    _GX2Sampler m_samplerAlt;
    uint8_t m_enabled;
    uint8_t m_alignment[3];
    uint32_t m_vertexUniformMatrix;
    uint32_t m_vertexUniformView;
    uint32_t m_vertexUniformNormal;
    uint32_t m_pixelSamplerLocation;
    uint32_t m_mode;
    uint32_t m_pixelConst1;
    uint32_t m_pixelConst2;
    uint32_t m_pixelConst3;
    uint32_t m_lightEnable;
    uint32_t m_lightDirection;
    uint32_t m_lightAmbient;
    uint32_t m_lightDiffuse;
    uint32_t m_lightSpecular;
    uint32_t m_materialAmbient;
    uint32_t m_materialDiffuse;
    uint32_t m_materialSpecular;
    uint32_t m_materialSpecularPower;
    uint32_t m_materialSpecularMode;
    uint32_t m_rimColor;
    uint32_t m_rimPower;
    uint32_t m_parameterStorage[0xC0];
};

#if UINTPTR_MAX == 0xffffffffU
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_vertexShader) == 0x10, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_pixelShader) == 0x14, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_fetchShader) == 0x18, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_sampler) == 0x3c, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_samplerAlt) == 0x48, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_vertexUniformMatrix) == 0x58, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_pixelSamplerLocation) == 0x64, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_mode) == 0x68, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_pixelConst1) == 0x6c, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_pixelConst2) == 0x70, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_pixelConst3) == 0x74, "layout");
static_assert(offsetof(__N_21_FFLiDefaultShader_cpp_d69dbbf4, m_parameterStorage) == 0xa8, "layout");
static_assert(sizeof(__N_21_FFLiDefaultShader_cpp_d69dbbf4) == 0x3a8, "layout");
static_assert(offsetof(FFLiDefaultShaderParam, matrixNormal) == 0x80, "layout");
static_assert(offsetof(FFLiDefaultShaderParam, light) == 0xb0, "layout");
static_assert(offsetof(FFLiDefaultShaderParam, material) == 0xf0, "layout");
static_assert(offsetof(FFLiDefaultShaderParam, rim) == 0x2e8, "layout");
static_assert(sizeof(FFLiDefaultShaderParam) == 0x300, "layout");
#endif

typedef __N_21_FFLiDefaultShader_cpp_d69dbbf4 FFLiDefaultShader;

void DestroyShader__FP11PlainShader(PlainShader* shader);
unsigned char ModulateModeToValue(unsigned int mode);
void DestroyPlainShader__FP11PlainShader(PlainShader* shader);

#endif
