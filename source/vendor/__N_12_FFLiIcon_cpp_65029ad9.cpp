#include "vendor/__N_12_FFLiIcon_cpp_65029ad9.hpp"
#include "game/client/render/Renderer.hpp"

#include <cstdlib>

extern void OSPanic(const char* file, unsigned int line, const char* message, ...);
extern void GX2SetContextState(_GX2ContextState* state);
extern int InitIconShader__F14IconShaderType(IconShaderType shaderType);
extern void* CreateModel__FP12FFLCharModel13FFLDataSourceP12FFLStoreDatai(
    void* model, FFLDataSource dataSource, FFLStoreData* storeData, int index);
extern unsigned int CreateIconTexture__FP11_GX2TexturePC12FFLCharModel(
    _GX2Texture* texture, const void* model);
extern void FFLDeleteCharModel(void* model);
extern void FinalizeIconShader__Fv();

int MakeIcon(_GX2Texture* texture, IconShaderType shaderType, FFLDataSource dataSource,
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
    unsigned int result = CreateIconTexture__FP11_GX2TexturePC12FFLCharModel(texture, model);
    FFLDeleteCharModel(model);
    std::free(modelBuffer);
    FinalizeIconShader__Fv();
    return static_cast<int>(result);
}

struct FFLColor
{
    float red;
    float green;
    float blue;
    float alpha;
};

struct FFLiRenderTexture
{
    void CopyTo(_GX2Texture* texture);
    unsigned int GetSurfaceFormat() const;
};

class FFLiBufferAllocator
{
public:
    void Init(void* buffer, unsigned int size);
    FFLiRenderTexture* Allocate(unsigned int size, unsigned int alignment);
    void* Allocate(unsigned int size);
};

struct FFLiCharModel
{
    unsigned int GetExpressionMask() const;
    int GetFavoriteColorIndex() const;
};

struct FFLIconSetting
{
    unsigned int width;
    unsigned int height;
    unsigned int backgroundType;
    FFLColor backgroundColor;
    unsigned int surfaceFormat;
    bool useDepth;
};

class FFLiManager
{
public:
    static bool IsConstruct();
    static FFLiManager* GetInstance();
    const FFLShaderCallback* GetShaderCallback() const;
};

extern void FFLSetExpression(FFLiCharModel& charModel, FFLExpression expression);
extern unsigned int FFLiGetIconBufferSize(const FFLIconSetting* setting);
extern unsigned int FFLiGetIconTempBufferSize(const FFLIconSetting* setting);
extern unsigned int FFLiGetDepthBufferSizeRenderTexture(unsigned int width, unsigned int height);
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
extern void FFLiSetRenderState(double value, int first, int second);
extern void FFLiSetBlendState(int source, int destination, int equation, int colorSource,
                              int colorDestination, int alphaEquation, int alphaSource, int alphaDestination);
extern void FFLiSetDepthState(int enable, int write, int function);
extern void FFLiSetRasterizerState(int cullMode, int frontFace, int polygonMode);
extern void FFLiFlushCaches();

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
    FFLiRenderTexture* renderTexture = static_cast<FFLiRenderTexture*>(temporaryAllocator.Allocate(0x9c, 4));
    FFLiInitRenderTexture(renderTexture, setting->width, setting->height, setting->surfaceFormat, &allocator);
    void* depthBuffer = temporaryAllocator.Allocate(FFLiGetDepthBufferSizeRenderTexture(setting->width, setting->height));

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

    FFLiSetRenderState(0.0, 0, 7);
    FFLiSetBlendState(204, 255, 0, 1, 1, 0, 0, 0);
    FFLiSetDepthState(1, 1, 1);
    FFLiSetRasterizerState(0, 4, 5);
    FFLiInvalidateRenderTexture(renderTexture);
    FFLiSetupRenderTexture(renderTexture, &clearColor, depthBuffer,
                           FFLiGetDepthBufferSizeRenderTexture(setting->width, setting->height),
                           renderTexture->GetSurfaceFormat(), manager->GetShaderCallback());

    if (setting->useDepth == false)
        FFLDrawOpaWithCallback(iconModel, shaderCallback);
    FFLDrawXluWithCallback(iconModel, shaderCallback);
    FFLiFlushRenderTexture(renderTexture);
    if (flush)
        FFLiFlushCaches();
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
