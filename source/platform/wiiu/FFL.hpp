#ifndef SOURCE_PLATFORM_WIIU_FFL_HPP
#define SOURCE_PLATFORM_WIIU_FFL_HPP

#include <stdint.h>
#include <stddef.h>
#include "game/client/render/Renderer.hpp"

#include "platform/wiiu/FFLiUtilShader.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "platform/wiiu/FFLiCopySurface.hpp"
#include "vendor/__N_12_FFLiIcon_cpp_65029ad9.hpp"
struct FFLCharModel { FFLiCharModel model; };
struct FFLShaderCallback;
class FFLiShaderCallback;
struct FFLStoreData;
struct FFLiMiiDataOfficial;
struct FFLiMiiDataCore;
struct FFLiMiiDataCoreRFL;
template <class Descriptor> class FFLiCompressorImpl;
struct FFLiCompressorDescBC1;
struct FFLiCompressorDescBC3;
class __N_21_FFLiDefaultShader_cpp_d69dbbf4;
typedef __N_21_FFLiDefaultShader_cpp_d69dbbf4 FFLiDefaultShader;
struct FFLIconSetting
{
    uint32_t width;
    uint32_t height;
    uint32_t backgroundType;
    FFLColor backgroundColor;
    uint32_t surfaceFormat;
    bool useDepth;
};
struct FFLiMiiID { uint8_t value[10]; };
struct FFLiDateTime { int year, month, day, hour, minute, second; };
struct FFLAdditionalInfo
{
    uint16_t name[11];
    uint16_t creatorName[11];
    FFLiMiiID createId;
    FFLColor skinColor;
    uint32_t flags;
    uint8_t faceType;
    uint8_t faceLine;
    uint8_t faceTexture;
    uint8_t hairType;
};

void FFLiAdjustShape(FFLDrawParam* drawParam, FFLBoundingBox* bounds,
                     float scaleX, float scaleY, const FFLVec3* offset,
                     bool flip, const FFLiCoordinate* coordinate,
                     FFLiShapePartsType parts, bool adjust);
bool FFLiIsNullMiiID(const FFLCreateID* id);
bool FFLiIsSameMiiID(const FFLCreateID* lhs, const FFLCreateID* rhs);
void FFLSetExpression(FFLiCharModel& model, FFLExpression expression);
bool FFLiIsValidCRC16(const void* data, unsigned int size);
bool FFLiCheckAlignPtr(const void* pointer, unsigned int alignment);
unsigned int FFLiBugCanSwapSize(unsigned int size);
void FFLiInvalidateShape(FFLDrawParam* drawParam);
bool FFLiiVerifyCharInfo(const FFLiCharInfo* info, int mode);
int FFLGetAdditionalInfo(FFLAdditionalInfo* output, FFLDataSource source,
                         FFLStoreData* data, unsigned short index,
                         int checkFontRegion);
FFLColor* FFLiGetFavoriteColor(int index);
bool FFLiIsTemporaryMiiID(const FFLCreateID* id);
void FFLiInvalidateSurface(_GX2Surface* surface, unsigned int type);
void FFLiInvalidateTexture(_GX2Texture* texture);
void FFLiRenderMaskTextures(
    FFLiMaskTextures* textures, FFLiMaskTexturesTempObject* object,
    const FFLiShaderCallback* callback, FFLiCopySurface* copySurface,
    FFLiCompressorImpl<FFLiCompressorDescBC3>* compressor);
void* FFLiBugVgtFixedIndexPtr(void* pointer);
int FFLiiGetEyeRotateOffset(int index);
int FFLiGetIconTempBufferSize(const FFLIconSetting* setting);
void FFLiInitModulateShapeMask(FFLModulateParam* param,
                               const _GX2Texture& texture);
uint32_t FFLiIsVaildResourceHeader(const FFLiResourceHeader* header);
void FFLiRenderFacelineTexture(
    FFLiRenderTexture* texture, const FFLiCharInfo* info, unsigned int resolution,
    FFLiFacelineTextureTempObject* object, const FFLiShaderCallback* callback,
    FFLiCopySurface* copySurface,
    FFLiCompressorImpl<FFLiCompressorDescBC1>* compressor);
void FFLiInitModulateShapeBeard(FFLModulateParam* param, int index);
void FFLiInitModulateShapeGlass(FFLModulateParam* param, int index,
                                const _GX2Texture& texture);
void FFLiInvalidateRenderTexture(FFLiRenderTexture* texture);
void FFLiMiiDataOfficial2CharInfo(FFLiCharInfo* destination,
                                  const FFLiMiiDataOfficial& source);
int FFLiGetBufferSizeMaskTextures(unsigned int expressionMask,
                                  unsigned int resolution, bool mipmap,
                                  bool useMipmap);
void FFLiInitModulateShapeForehead(FFLModulateParam* param, int index);
void FFLiInitModulateShapeNoseline(FFLModulateParam* param,
                                   const _GX2Texture& texture);
unsigned int FFLiConvertFSStatusToFFLResult(int status);
int FFLiMiiDataCoreRFL2MiiDataCore(FFLiMiiDataCore* destination,
                                   const FFLiMiiDataCoreRFL& source,
                                   bool replaceName);
void FFLiClearCreatorNameFromOfficial(FFLiMiiDataOfficial* data);

#endif
