#include "vendor/__N_12_FFLiIcon_cpp_65029ad9.hpp"
#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#define private public
#include "platform/wiiu/FFLiManager.hpp"
#undef private
#include <gx2/event.h>
#include <gx2/registers.h>

#include <cstdlib>
#include <coreinit/debug.h>

extern "C" void GX2SetContextState(_GX2ContextState* state);
extern int InitIconShader__F14IconShaderType(IconShaderType shaderType);
struct FFLCharModel;
extern int FFLGetBufferSizeCharModel(const FFLCharModelDesc* description);
extern int FFLGetTempBufferSizeCharModel(const FFLCharModelDesc* description);
extern int FFLInitCharModelCPUStep(FFLCharModel* model, const FFLCharModelSource* source,
                                   const FFLCharModelDesc* description, FFLCharModelBuffer* buffer);
extern void FFLInitCharModelGPUStep(FFLCharModel* model);
extern void SetupIconShaderForCreateModel__Fv();
extern void SetupIconShaderForDrawTexture__FUsT1(unsigned short width, unsigned short height);
extern void* CreateModel__FP12FFLCharModel13FFLDataSourceP12FFLStoreDatai(
    void* model, FFLDataSource dataSource, FFLStoreData* storeData, int index);
extern unsigned int FFLGetIconBufferSize(const FFLIconSetting* setting);
extern int FFLGetIconTempBufferSize(const FFLIconSetting* setting);
extern unsigned int FFLMakeIconEx(_GX2Texture* texture, const FFLCharModel* model,
                                  FFLExpression expression, const FFLIconSetting* setting,
                                  void* buffer, void* temporaryBuffer);
extern void* CreateIconTexture__FP11_GX2TexturePC12FFLCharModel(
    _GX2Texture* texture, const void* model);
extern void FFLDeleteCharModel(void* model);
extern void FinalizeIconShader__Fv();

static const FFLIconSetting sDefaultIconSetting = {
    64, 64, 1, {0.0f, 0.0f, 0.0f, 0.0f}, 0x1a, false
};

void* CreateModel__FP12FFLCharModel13FFLDataSourceP12FFLStoreDatai(
    void* model, FFLDataSource dataSource, FFLStoreData* storeData, int index)
{
    if (model == 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x35,
                "Invalid Pointer: %x", 0);

    FFLCharModelDesc description = {static_cast<FFLResolution>(0x100), 1, 1,
                                   static_cast<FFLResourceType>(0), false};
    FFLCharModelSource source = {dataSource, storeData, static_cast<unsigned short>(index)};
    FFLCharModelBuffer buffer;
    buffer.modelBufferSize = static_cast<unsigned int>(FFLGetBufferSizeCharModel(&description));
    buffer.temporaryBufferSize = static_cast<unsigned int>(FFLGetTempBufferSizeCharModel(&description));
    buffer.modelBuffer = std::malloc(buffer.modelBufferSize);
    buffer.temporaryBuffer = std::malloc(buffer.temporaryBufferSize);
    const int result = FFLInitCharModelCPUStep(static_cast<FFLCharModel*>(model), &source, &description, &buffer);
    if (result != 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x52,
                "FFLCharModel cannot be set up. (Result: %d)", result);
    SetupIconShaderForCreateModel__Fv();
    FFLInitCharModelGPUStep(static_cast<FFLCharModel*>(model));
    GX2DrawDone();
    std::free(buffer.temporaryBuffer);
    return buffer.modelBuffer;
}

void* CreateIconTexture__FP11_GX2TexturePC12FFLCharModel(
    _GX2Texture* texture, const void* model)
{
    if (texture == 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x6e,
                "Invalid Pointer: %x", 0);
    if (model == 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x6f,
                "Invalid Pointer: %x", 0);
    void* buffer = std::malloc(FFLGetIconBufferSize(&sDefaultIconSetting));
    void* temporaryBuffer = std::malloc(FFLGetIconTempBufferSize(&sDefaultIconSetting));
    SetupIconShaderForDrawTexture__FUsT1(64, 64);
    unsigned int result = FFLMakeIconEx(texture, static_cast<const FFLCharModel*>(model),
                                        static_cast<FFLExpression>(0), &sDefaultIconSetting,
                                        buffer, temporaryBuffer);
    if (result != 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x7d,
                "FFLInitCharModel Failed (FFLResult: %d)", result);
    GX2DrawDone();
    std::free(temporaryBuffer);
    return buffer;
}

void* MakeIcon(_GX2Texture* texture, IconShaderType shaderType, FFLDataSource dataSource,
             FFLStoreData* storeData, int index)
{
    if (texture == 0)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x93,
                "Invalid Pointer: %x", 0);

    if (shaderType >= ICON_SHADER_TYPE_END)
        OSPanic("C:\\Users\\build\\Documents\\TFS\\StoriesPark\\Minecraft\\MinecraftConsoles-Maintenance-", 0x94,
                "Assertion Failed: %s", "ICON_SHADER_TYPE_BEGIN <= type && type < ICON_SHADER_TYPE_END");

    InitIconShader__F14IconShaderType(shaderType);
    unsigned char model[2016];
    void* modelBuffer = CreateModel__FP12FFLCharModel13FFLDataSourceP12FFLStoreDatai(
        model, dataSource, storeData, index);
    GX2SetContextState(0);
    void* iconBuffer = CreateIconTexture__FP11_GX2TexturePC12FFLCharModel(texture, model);
    FFLDeleteCharModel(model);
    std::free(modelBuffer);
    (void)iconBuffer;
    FinalizeIconShader__Fv();
    return iconBuffer;
}

extern void FFLSetExpression(FFLiCharModel& charModel, FFLExpression expression);
extern unsigned int FFLiGetIconBufferSize(const FFLIconSetting* setting);
extern int FFLiGetIconTempBufferSize(const FFLIconSetting* setting);
extern int FFLiGetDepthBufferSizeRenderTexture(unsigned int width, unsigned int height);
extern FFLColor* FFLiGetFavoriteColor(int index);
extern void FFLiInitRenderTexture(FFLiRenderTexture* texture, unsigned int width,
                                  unsigned int height, unsigned int format,
                                  FFLiBufferAllocator* allocator);
extern void FFLiInvalidateRenderTexture(FFLiRenderTexture* texture);
extern void FFLiSetupRenderTexture(FFLiRenderTexture* texture, const FFLColor* clearColor,
                                   void* depthBuffer, unsigned int depthBufferSize,
                                   unsigned int format, const FFLShaderCallback* callback);
extern void FFLDrawOpaWithCallback(const FFLiCharModel& charModel, const FFLShaderCallback* callback);
extern void FFLDrawXluWithCallback(const FFLiCharModel& charModel, const FFLShaderCallback* callback);
extern void FFLiFlushRenderTexture(FFLiRenderTexture* texture);

bool __N_12_FFLiIcon_cpp_65029ad9::IsExpressionContained(const FFLiCharModel* charModel,
                                                          FFLExpression expression)
{
    return (charModel->GetExpressionMask() & (1u << (static_cast<unsigned int>(expression) & 0x3fu))) != 0;
}

unsigned int __N_12_FFLiIcon_cpp_65029ad9::MakeIconImpl(_GX2Texture* texture,
                                                        const FFLiCharModel* charModel,
                                                        FFLExpression expression,
                                                        const FFLIconSetting* setting,
                                                        void* buffer, void* temporaryBuffer,
                                                        const FFLShaderCallback* shaderCallback,
                                                        bool flush)
{
    if (!FFLiManager::IsConstruct())
        return 4;

    FFLiManager* manager = FFLiManager::GetInstance();
    FFLiCharModel iconModel = *charModel;
    if (!IsExpressionContained(charModel, expression))
        return 1;

    FFLSetExpression(iconModel, expression);
    FFLiBufferAllocator allocator;
    allocator.Init(buffer, FFLiGetIconBufferSize(setting));
    FFLiBufferAllocator temporaryAllocator;
    temporaryAllocator.Init(temporaryBuffer, FFLiGetIconTempBufferSize(setting));
    FFLiRenderTexture* renderTexture = reinterpret_cast<FFLiRenderTexture*>(
        static_cast<uintptr_t>(temporaryAllocator.Allocate(0x9c, 4)));
    FFLiInitRenderTexture(renderTexture, setting->width, setting->height, setting->surfaceFormat, &allocator);
    void* depthBuffer = reinterpret_cast<void*>(static_cast<uintptr_t>(
        temporaryAllocator.Allocate(FFLiGetDepthBufferSizeRenderTexture(setting->width, setting->height))));

    FFLColor clearColor = {0.0f, 0.0f, 180.0f, 50.12353515625f};
    if (setting->backgroundType == 0)
    {
        FFLColor* favoriteColor = FFLiGetFavoriteColor(iconModel.GetFavoriteColorIndex());
        clearColor = *favoriteColor;
    }
    else if (setting->backgroundType == 1)
    {
        clearColor = setting->backgroundColor;
    }

    GX2SetAlphaTest(FALSE, GX2_COMPARE_FUNC_ALWAYS, 0.0f);
    GX2SetBlendControl(GX2_RENDER_TARGET_0, GX2_BLEND_MODE_SRC_ALPHA,
                       GX2_BLEND_MODE_INV_SRC_ALPHA, GX2_BLEND_COMBINE_MODE_ADD,
                       TRUE, GX2_BLEND_MODE_SRC_ALPHA, GX2_BLEND_MODE_DST_ALPHA,
                       GX2_BLEND_COMBINE_MODE_ADD);
    GX2SetDepthOnlyControl(TRUE, TRUE, GX2_COMPARE_FUNC_LESS);
    GX2SetCullOnlyControl(GX2_FRONT_FACE_CCW, FALSE, FALSE);
    FFLiInvalidateRenderTexture(renderTexture);
    FFLiSetupRenderTexture(renderTexture, &clearColor, depthBuffer,
                           FFLiGetDepthBufferSizeRenderTexture(setting->width, setting->height),
                           renderTexture->GetSurfaceFormat(), manager->GetShaderCallback());

    if (setting->useDepth == false)
        FFLDrawOpaWithCallback(iconModel, shaderCallback);
    FFLDrawXluWithCallback(iconModel, shaderCallback);
    FFLiFlushRenderTexture(renderTexture);
    if (flush)
        GX2DrawDone();
    renderTexture->CopyTo(texture);
    return 0;
}

double __N_12_FFLiIcon_cpp_65029ad9::RadianToDegree(float radians)
{
    return static_cast<double>(radians * 180.0f / 3.141590118408203125f);
}

bool __N_12_FFLiIcon_cpp_65029ad9::IsTextureSizeValid(unsigned short size)
{
    return size == 32 || size == 64 || size == 128 || size == 256 || size == 512 || size == 1024;
}
