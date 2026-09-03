#include "vendor/__N_21_FFLiDefaultShader_cpp_d69dbbf4.hpp"
#include "platform/wiiu/FFLiDefaultParams.hpp"

void GX2SetCullOnlyControl(unsigned int frontFace, int cullFront, int cullBack);
void GX2SetPixelSampler(const void* sampler, unsigned int unit);
void GX2SetVertexSampler(const void* sampler, unsigned int unit);
void GX2SetPixelUniformReg(unsigned int offset, unsigned int count, const void* values);
void GX2SetVertexUniformReg(unsigned int offset, unsigned int count, const void* values);
void GX2SetVertexUniformBlock(unsigned int offset, unsigned int count, const void* values);
void GX2DrawEx(unsigned int mode, unsigned int count, unsigned int offset, unsigned int instances);
void ASM_MTX44Identity(float (*matrix)[4]);
void ASM_MTX44Transpose(const float (*source)[4], float (*destination)[4]);

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetCullingModeWraper(_GX2FrontFaceMode mode, _GX2Boolean cullFront, _GX2Boolean cullBack)
{
    GX2SetCullOnlyControl(mode, cullFront, cullBack);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupCulling(FFLCullMode mode, const FFLModulateParam& param, int enabled)
{
    const bool special = param.type < 9 && enabled != 0;
    if (mode == 0) SetCullingModeWraper(0, 0, 0);
    else if (mode == 1) SetCullingModeWraper(0, special ? 0 : 1, special ? 1 : 0);
    else if (mode == 2) SetCullingModeWraper(0, special ? 0 : 1, special ? 1 : 0);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::BindTexture(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLModulateParam& param)
{
    if (param.texture != 0) {
        const void* sampler = (const void*)(shader->m_uniformSampler);
        if (shader->m_enabled != 0 && param.type == 11) sampler = (const void*)(shader->m_uniformTexture);
        if (param.texture != 0) GX2SetPixelSampler(param.texture, shader->m_uniformAlpha);
        GX2SetVertexSampler(sampler, shader->m_uniformAlpha);
    }
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupConstColor(unsigned int location, const FFLColor& color)
{
    GX2SetPixelUniformReg(location, 4, color.value);
}

uint8_t __N_21_FFLiDefaultShader_cpp_d69dbbf4::ModulateModeToValue(FFLModulateMode mode)
{
    static const uint8_t values[6] = {0, 110, 111, 112, 113, 114};
    return mode < 6 ? values[mode] : 0;
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupModulateMode(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, FFLModulateMode mode)
{
    uint32_t value[4] = {ModulateModeToValue(mode), 0, 0, 0};
    GX2SetPixelUniformReg(shader->m_uniformColor, 4, value);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupModulate(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLModulateParam& param)
{
    BindTexture(shader, param);
    if (param.mode == 0 || param.mode == 1 || param.mode == 2) SetupConstColor((unsigned int)shader->m_textureShader, param.color[0]);
    if (param.mode == 2) {
        SetupConstColor((unsigned int)shader->m_textureShaderAlt, param.color[1]);
        SetupConstColor((unsigned int)shader->m_textureShaderThird, param.color[2]);
    }
    SetupModulateMode(shader, param.mode);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupMaterial(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLDrawParam& param)
{
    const unsigned int type = param.value[0x10];
    if (type >= 9 && type <= 17) {
        const uint32_t* material = shader->m_prefix + type * 14;
        GX2SetPixelUniformReg(shader->m_uniformLight, 4, material + 6);
        GX2SetPixelUniformReg(shader->m_uniformMatrix, 4, material + 10);
        GX2SetPixelUniformReg(shader->m_uniformNormal, 4, material + 2);
        GX2SetPixelUniformReg(shader->m_uniformColor, 4, material + 3);
        if (param.value[0xb] == 0) GX2SetPixelUniformReg(shader->m_uniformRim, 4, material + 11);
        else GX2SetPixelUniformReg(shader->m_uniformRim, 4, material + 11);
    }
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::ShaderCallbackDraw(void* shader, const FFLDrawParam* param)
{
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* object = (__N_21_FFLiDefaultShader_cpp_d69dbbf4*)shader;
    SetupCulling(param->value[0x15], *(const FFLModulateParam*)(&param->value[0xf]), object->m_modulateParam);
    SetupModulate(object, *(const FFLModulateParam*)(&param->value[0xf]));
    SetupMaterial(object, *param);
    if (param->value[0x19] != 0) {
        GX2SetVertexUniformReg(0, 4, param->value);
        GX2SetVertexUniformReg(4, 4, param->value + 3);
        GX2SetVertexUniformReg(8, 4, param->value + 6);
        GX2SetVertexUniformReg(12, 4, param->value + 9);
        GX2SetVertexUniformReg(16, 4, param->value + 12);
        GX2DrawEx(param->value[0x16], param->value[0x17], param->value[0x18], param->value[0x19]);
    }
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::ShaderCallbackSetMatrix(void* shader, const _Mat44* matrix)
{
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* object = (__N_21_FFLiDefaultShader_cpp_d69dbbf4*)shader;
    float identity[4][4];
    ASM_MTX44Identity(identity);
    GX2SetVertexUniformReg(object->m_shaderValue[1], 16, identity);
    GX2SetVertexUniformReg(object->m_shaderValue[2], 12, identity);
    ASM_MTX44Transpose(matrix->value, identity);
    GX2SetVertexUniformReg(object->m_shaderValue[0], 16, identity);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::Identity(float (*matrix)[4])
{
    uint32_t i;
    for (i = 0; i < 12; ++i) matrix[0][i] = 0.0f;
    matrix[0][0] = 1.0f;
    matrix[1][1] = 1.0f;
    matrix[2][2] = 1.0f;
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::InitLightParam(FFLiDefaultShaderLight* param)
{
    const uint32_t address = FFLiDefaultParams::GetLight();
    const uint32_t* source = (const uint32_t*)(uintptr_t)address;
    param->value[0] = source[0]; param->value[1] = source[1]; param->value[2] = source[2]; param->value[3] = source[3];
    param->value[4] = source[4]; param->value[5] = source[5]; param->value[6] = source[6]; param->value[7] = source[7];
    param->value[8] = source[8]; param->value[9] = source[9]; param->value[10] = source[10]; param->value[11] = source[11];
    param->value[12] = source[12]; param->value[13] = source[13]; param->value[14] = source[14]; param->value[15] = source[15];
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::InitMaterialParam(FFLiDefaultShaderMaterial* param, FFLDrawType type)
{
    const uint32_t* source = (const uint32_t*)(uintptr_t)FFLiDefaultParams::GetMaterial(type);
    uint32_t i;
    for (i = 0; i < 14; ++i) param->value[i] = source[i];
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::InitParam(FFLiDefaultShaderParam* param)
{
    ASM_MTX44Identity((float (*)[4])param->matrix);
    ASM_MTX44Identity((float (*)[4])param->matrixView);
    Identity((float (*)[4])param->identity);
    InitLightParam(&param->light);
    uint32_t i;
    for (i = 0; i < 9; ++i) InitMaterialParam(&param->material[i], i);
    const uint32_t* rim = (const uint32_t*)(uintptr_t)FFLiDefaultParams::GetRim();
    for (i = 0; i < 5; ++i) param->rim[i] = rim[i];
    param->rim[5] = 0;
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetUniformLightDisable(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader)
{
    uint32_t value[4] = {0, 0, 0, 0};
    GX2SetPixelUniformReg(shader->m_uniformMode, 4, value);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformLight(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader)
{
    uint32_t enabled[4] = {shader->m_lightEnabled != 0, 0, 0, 0};
    uint32_t direction[4] = {shader->m_lightDiffuse, shader->m_lightAmbient, shader->m_lightSpecular, 0};
    GX2SetPixelUniformReg(shader->m_uniformLight, 4, direction);
    GX2SetPixelUniformReg(shader->m_uniformMode, 4, enabled);
    GX2SetPixelUniformReg(shader->m_uniformMatrix, 4, shader->m_lightDirection);
    GX2SetPixelUniformReg(shader->m_uniformNormal, 4, shader->m_lightDirection + 4);
    GX2SetPixelUniformReg(shader->m_uniformColor, 4, shader->m_lightDirection + 8);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformParam(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader)
{
    GX2SetVertexUniformReg(shader->m_shaderValue[0], 16, shader->m_reservedA);
    GX2SetVertexUniformReg(shader->m_shaderValue[1], 16, shader->m_reservedA + 16);
    GX2SetVertexUniformReg(shader->m_shaderValue[2], 12, shader->m_reservedA + 32);
    SetupUniformLight(shader);
    GX2SetPixelUniformReg(shader->m_uniformRimPower, 4, shader->m_reservedD);
    GX2SetPixelUniformReg(shader->m_uniformRimColor, 4, &shader->m_rimPower);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::Mat44to34(_Mat34* destination, const _Mat44* source)
{
    uint32_t row, column;
    for (row = 0; row < 3; ++row) for (column = 0; column < 4; ++column) destination->value[row][column] = source->value[row][column];
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::StoreColumnMajor(_Mat44* destination, const _Mat44& source)
{
    uint32_t row, column;
    for (row = 0; row < 4; ++row) for (column = 0; column < 4; ++column) destination->value[column][row] = source.value[row][column];
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::StoreColumnMajor(float (*destination)[4], const _Mat44& source)
{
    uint32_t row, column;
    for (row = 0; row < 3; ++row) for (column = 0; column < 3; ++column) destination[row][column] = source.value[column][row];
}

FFLiDefaultShaderLight* __N_21_FFLiDefaultShader_cpp_d69dbbf4::GetLightParam(FFLiDefaultShaderParam* param)
{
    return &param->light;
}

FFLiDefaultShaderMaterial* __N_21_FFLiDefaultShader_cpp_d69dbbf4::GetMaterialParam(FFLiDefaultShaderParam* param, FFLDrawType type)
{
    return type < 9 ? &param->material[type] : 0;
}
