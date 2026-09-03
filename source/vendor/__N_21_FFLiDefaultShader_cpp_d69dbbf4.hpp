#ifndef SOURCE_VENDOR___N_21_FFLIDEFAULTSHADER_CPP_D69DBBF4_HPP
#define SOURCE_VENDOR___N_21_FFLIDEFAULTSHADER_CPP_D69DBBF4_HPP

#include <stdint.h>

typedef uint32_t _GX2FrontFaceMode;
typedef uint32_t _GX2Boolean;
typedef uint32_t FFLCullMode;
typedef uint32_t FFLModulateMode;
typedef uint32_t FFLDrawType;
typedef uint32_t FFLSpecularMode;

struct FFLColor { float value[4]; };
struct _Mat44 { float value[4][4]; };
struct _Mat34 { float value[3][4]; };
struct FFLModulateParam
{
    FFLModulateMode mode;
    uint32_t type;
    FFLColor color[3];
    const void* texture;
};
struct FFLDrawParam
{
    uint32_t value[32];
};
struct FFLiDefaultShaderLight { uint32_t value[16]; };
struct FFLiDefaultShaderMaterial { uint32_t value[14]; };
struct FFLiDefaultShaderParam
{
    uint32_t matrix[16];
    uint32_t matrixView[16];
    uint32_t matrixNormal[12];
    uint32_t identity[16];
    FFLiDefaultShaderLight light;
    FFLiDefaultShaderMaterial material[9];
    uint32_t rim[6];
};

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

    uint32_t m_prefix[21];
    uint8_t m_enabled;
    uint8_t m_alignment[3];
    uint32_t m_shaderValue[5];
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* m_textureShader;
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* m_textureShaderAlt;
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* m_textureShaderThird;
    uint32_t m_uniformMode;
    uint32_t m_uniformLight;
    uint32_t m_uniformMatrix;
    uint32_t m_uniformNormal;
    uint32_t m_uniformColor;
    uint32_t m_uniformSampler;
    uint32_t m_uniformTexture;
    uint32_t m_uniformModeValue;
    uint32_t m_uniformRim;
    uint32_t m_uniformRimPower;
    uint32_t m_uniformRimColor;
    uint32_t m_uniformAlpha;
    uint32_t m_uniformFlags;
    uint32_t m_uniformExtra;
    uint32_t m_reservedA[44];
    uint32_t m_lightEnabled;
    uint32_t m_lightDiffuse;
    uint32_t m_lightAmbient;
    uint32_t m_lightSpecular;
    uint32_t m_reservedB[20];
    uint32_t m_lightDirection[6];
    uint32_t m_reservedC[70];
    uint32_t m_materialValue[6];
    uint32_t m_reservedD[40];
    uint32_t m_rimPower;
    uint32_t m_modulateParam;
};

typedef __N_21_FFLiDefaultShader_cpp_d69dbbf4 FFLiDefaultShader;

#endif
