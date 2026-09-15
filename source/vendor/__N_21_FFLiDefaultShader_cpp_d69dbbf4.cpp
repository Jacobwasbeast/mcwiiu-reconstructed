#include "vendor/__N_21_FFLiDefaultShader_cpp_d69dbbf4.hpp"
#include "platform/wiiu/FFLiDefaultParams.hpp"

#include <cstdlib>
#include <cstring>
#include <coreinit/debug.h>

#include <coreinit/memdefaultheap.h>

void DestroyShader__FP11PlainShader(PlainShader* shader)
{
    if (shader == 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0xbb,
                "Invalid Pointer: %x", 0);

    if (shader->vertexShader != 0)
    {
        if (shader->vertexShader->program != 0)
            MEMFreeToDefaultHeap(shader->vertexShader->program);
        MEMFreeToDefaultHeap(shader->vertexShader);
    }
    if (shader->pixelShader != 0)
    {
        if (shader->pixelShader->program != 0)
            MEMFreeToDefaultHeap(shader->pixelShader->program);
        MEMFreeToDefaultHeap(shader->pixelShader);
    }
    if (shader->fetchShader.program != 0)
        MEMFreeToDefaultHeap(shader->fetchShader.program);
    if (shader->contextState != 0)
        MEMFreeToDefaultHeap(reinterpret_cast<void*>(static_cast<uintptr_t>(shader->contextState)));
}
unsigned char ModulateModeToValue(unsigned int mode)
{
    static const unsigned char values[6] = { 0, 1, 2, 3, 4, 5 };
    if (mode > 5)
    {
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\"
                "MinecraftConsoles-Maintenance-LegacyPlatforms\\4JLibraries_Source\\"
                "WiiU\\Render\\MiiIcon\\PlainShader.cpp", 0xe9,
                "Invalid Condition: %d");
        return 0;
    }
    return values[mode];
}

void DestroyPlainShader__FP11PlainShader(PlainShader* shader)
{
    if (shader == 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x18c,
                "Invalid Pointer: %x", 0);
    DestroyShader__FP11PlainShader(shader);
}

extern "C" void GX2SetCullOnlyControl(unsigned int frontFace, int cullFront, int cullBack);
extern "C" void GX2SetPixelTexture(const _GX2Texture* texture, unsigned int unit);
extern "C" void GX2SetPixelSampler(const void* sampler, unsigned int unit);
extern "C" void GX2SetVertexSampler(const void* sampler, unsigned int unit);
extern "C" void GX2SetPixelUniformReg(unsigned int offset, unsigned int count, const void* values);
extern "C" void GX2SetVertexUniformReg(unsigned int offset, unsigned int count, const void* values);
extern "C" void GX2SetVertexUniformBlock(unsigned int offset, unsigned int count, const void* values);
extern "C" void GX2SetAttribBuffer(unsigned int index, unsigned int size, unsigned int stride, const void* buffer);
extern "C" void GX2DrawEx(unsigned int mode, unsigned int count, unsigned int offset, unsigned int instances);
extern "C" void GX2DrawIndexedEx(unsigned int mode, unsigned int count, unsigned int indexType, const void* indexBuffer, unsigned int baseVertex, unsigned int instances);
extern "C" void GX2InitSampler(void* sampler, unsigned int clamp, unsigned int filter);
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
    else if (mode == 2) SetCullingModeWraper(0, special ? 1 : 0, special ? 0 : 1);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::BindTexture(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLModulateParam& param)
{
    if (param.texture != 0) {
        const _GX2Sampler* sampler = &shader->m_sampler;
        if (shader->m_enabled != 0 && param.type == 11) sampler = &shader->m_samplerAlt;
        GX2SetPixelTexture(param.texture, shader->m_pixelSamplerLocation);
        GX2SetPixelSampler(sampler, shader->m_pixelSamplerLocation);
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
    GX2SetPixelUniformReg(shader->m_mode, 4, value);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupModulate(__N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLModulateParam& param)
{
    BindTexture(shader, param);
    if (param.mode == 0 || param.mode == 2 || (param.mode >= 3 && param.mode <= 5)) SetupConstColor(shader->m_pixelConst1, *param.colorR);
    if (param.mode == 2) {
        SetupConstColor(shader->m_pixelConst2, *param.colorG);
        SetupConstColor(shader->m_pixelConst3, *param.colorB);
    }
    SetupModulateMode(shader, param.mode);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupMaterial(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader, const FFLDrawParam& param)
{
    const unsigned int type = param.flags;
    if (type < 9 || type > 17) {
        const uint32_t* material = shader->m_parameterStorage + 68 + type * 14;
        uint32_t specularPower[4] = {material[12], 0, 0, 0};
        uint32_t specularMode[4] = {param.attributes.attributes[3].buffer != 0 ? material[13] : 0, 0, 0, 0};
        GX2SetPixelUniformReg(shader->m_materialAmbient, 4, material);
        GX2SetPixelUniformReg(shader->m_materialDiffuse, 4, material + 4);
        GX2SetPixelUniformReg(shader->m_materialSpecular, 4, material + 8);
        GX2SetPixelUniformReg(shader->m_materialSpecularPower, 4, specularPower);
        GX2SetPixelUniformReg(shader->m_materialSpecularMode, 4, specularMode);
    }
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::ShaderCallbackDraw(void* shader, const FFLDrawParam* param)
{
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* object = (__N_21_FFLiDefaultShader_cpp_d69dbbf4*)shader;
    SetupCulling(param->primitive.primitiveType, param->modulate, object->m_parameterStorage[191]);
    SetupModulate(object, param->modulate);
    SetupMaterial(object, *param);
    if (param->flags != 0) {
        for (unsigned int index = 0; index < 5; ++index)
        {
            const FFLAttributeBuffer& attribute = param->attributes.attributes[index];
            GX2SetAttribBuffer(index, attribute.size, attribute.stride, attribute.buffer);
        }
        GX2DrawIndexedEx(param->primitive.primitiveType, param->primitive.indexCount, param->primitive.vertexCount,
                         param->primitive.indexBuffer, 0, 1);
    }
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::ShaderCallbackSetMatrix(void* shader, const _Mat44* matrix)
{
    __N_21_FFLiDefaultShader_cpp_d69dbbf4* object = (__N_21_FFLiDefaultShader_cpp_d69dbbf4*)shader;
    float identity[4][4];
    ASM_MTX44Identity(identity);
    GX2SetVertexUniformReg(object->m_vertexUniformView, 16, identity);
    GX2SetVertexUniformReg(object->m_vertexUniformNormal, 12, identity);
    ASM_MTX44Transpose(matrix->value, identity);
    GX2SetVertexUniformReg(object->m_vertexUniformMatrix, 16, identity);
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
    const unsigned char* source = FFLiDefaultParams::GetLight();
    param->value[0] = source[0];
    memcpy(param->value + 1, source + 4, 60);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::InitMaterialParam(FFLiDefaultShaderMaterial* param, FFLDrawType type)
{
    const unsigned char* source = FFLiDefaultParams::GetMaterial(type);
    memcpy(param->value, source, sizeof(param->value));
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::InitParam(FFLiDefaultShaderParam* param)
{
    ASM_MTX44Identity((float (*)[4])param->matrix);
    ASM_MTX44Identity((float (*)[4])param->matrixView);
    Identity((float (*)[4])param->matrixNormal);
    InitLightParam(&param->light);
    uint32_t i;
    for (i = 0; i < 9; ++i) InitMaterialParam(&param->material[i], i);
    const unsigned char* rim = FFLiDefaultParams::GetRim();
    memcpy(param->rim, rim, 20);
    param->rim[5] = 0;
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetUniformLightDisable(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader)
{
    uint32_t value[4] = {0, 0, 0, 0};
    GX2SetPixelUniformReg(shader->m_lightEnable, 4, value);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformLight(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader)
{
    uint32_t enabled[4] = {shader->m_parameterStorage[44] != 0, 0, 0, 0};
    uint32_t direction[4] = {shader->m_parameterStorage[45], shader->m_parameterStorage[46], shader->m_parameterStorage[47], 0};
    GX2SetPixelUniformReg(shader->m_lightDirection, 4, direction);
    GX2SetPixelUniformReg(shader->m_lightEnable, 4, enabled);
    GX2SetPixelUniformReg(shader->m_lightAmbient, 4, shader->m_parameterStorage + 48);
    GX2SetPixelUniformReg(shader->m_lightDiffuse, 4, shader->m_parameterStorage + 52);
    GX2SetPixelUniformReg(shader->m_lightSpecular, 4, shader->m_parameterStorage + 56);
}

void __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformParam(const __N_21_FFLiDefaultShader_cpp_d69dbbf4* shader)
{
    GX2SetVertexUniformReg(shader->m_vertexUniformMatrix, 16, shader->m_parameterStorage);
    GX2SetVertexUniformReg(shader->m_vertexUniformView, 16, shader->m_parameterStorage + 16);
    GX2SetVertexUniformReg(shader->m_vertexUniformNormal, 12, shader->m_parameterStorage + 32);
    SetupUniformLight(shader);
    GX2SetPixelUniformReg(shader->m_rimColor, 4, shader->m_parameterStorage + 186);
    GX2SetPixelUniformReg(shader->m_rimPower, 4, shader->m_parameterStorage + 190);
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
