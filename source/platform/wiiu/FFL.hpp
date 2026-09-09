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
struct FFLResourceDesc;
struct FFLShaderCallback;
class FFLiShaderCallback;
struct FFLStoreData;
struct FFLiStoreDataCFL;
struct FFLiMiiDataOfficial;
struct FFLiMiiDataCore;
struct FFLiMiiDataCoreRFL;
template <class Descriptor> class FFLiCompressorImpl;
struct FFLiCompressorDescBC1;
struct FFLiCompressorDescBC3;
struct _GX2ContextState;
class __N_21_FFLiDefaultShader_cpp_d69dbbf4;
typedef __N_21_FFLiDefaultShader_cpp_d69dbbf4 FFLiDefaultShader;
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

int FFLInitRes(void* work, FFLResourceDesc* resourceDesc);
int FFLGetWorkSize(FFLResourceDesc* resourceDesc);
int FFLiGetBufferSizeFacelineTexture(uint32_t resolution, bool mipmap, bool useMipmap);

int FFLiLoadShape(FFLDrawParam*, FFLBoundingBox*, FFLiCharModel*, FFLiShapePartsType,
                  unsigned short, FFLiResourceLoader*, FFLiBufferAllocator*);
void FFLiInvalidate(unsigned int, void*, unsigned int);
int FFLiCanDrawShape(const FFLDrawParam*);
bool FFLiIsValidMiiID(const FFLCreateID*);
void FFLiBugEndianSwap(void*, unsigned int);
void FFLDeleteCharModel(void*);
bool FFLiIsSpecialMiiID(const FFLCreateID*);
int FFLiGetDatabasePath(char*, unsigned int, unsigned long long, const char*);
int FFLiGetResourcePath(char*, unsigned int, const char*);
void FFLSetShaderCallback(void*);
unsigned int FFLiInitMaskTextures(FFLiMaskTextures*, unsigned int, unsigned int, bool, bool, FFLiBufferAllocator*);
unsigned int FFLiGetIconBufferSize(const FFLIconSetting*);
void FFLiInitRenderTexture(FFLiRenderTexture*, unsigned int, unsigned int, unsigned int, FFLiBufferAllocator*);
void FFLiTempSetShaderMode(int);
void FFLDrawOpaWithCallback(const FFLiCharModel&, const FFLShaderCallback*);
void FFLDrawXluWithCallback(const FFLiCharModel&, const FFLShaderCallback*);
void FFLiFlushRenderTexture(FFLiRenderTexture*);
unsigned int FFLiGetMipMapLevelSize(unsigned int, unsigned int);
void FFLiSetupRenderTexture(FFLiRenderTexture*, const FFLColor*, void*, unsigned int, unsigned int, const FFLShaderCallback*);
void FFLiInitFacelineTexture(FFLiRenderTexture*, unsigned int, bool, bool, FFLiBufferAllocator*);
void FFLiTempSetContextState(const _GX2ContextState*);
void FFLiInitModulateShapeCap(FFLModulateParam*, int, const _GX2Texture&);
void FFLiInitModulateShapeHair(FFLModulateParam*, int);
void FFLiInitModulateShapeNose(FFLModulateParam*, int);
int FFLiGetBufferRenderTexture(unsigned int, unsigned int, unsigned int);
int FFLiGetBufferRenderTexture(unsigned int, unsigned int, unsigned int, unsigned int);
int FFLiStoreDataCFLToCharInfo(FFLiCharInfo*, const FFLiStoreDataCFL&);
unsigned int FFLiBugCanVgtFixedIndexSize(unsigned int);
unsigned int FFLiBugCanVgtFixedIndexSize();
int FFLiLoadTextureWithAllocate(_GX2Texture**, FFLiTexturePartsType, unsigned int, FFLiResourceLoader*, FFLiBufferAllocator*);
void FFLiInitModulateShapeFaceline(FFLModulateParam*, const _GX2Texture&);
uint32_t FFLiGetTextureMaxSizeWithAlign(const FFLiResourceManager*, FFLResourceType, FFLiTexturePartsType);
int FFLiInitTempObjectMaskTextures(FFLiMaskTexturesTempObject*, const FFLiMaskTextures*, const FFLiCharInfo*, unsigned int, unsigned int, bool, bool, bool, FFLiResourceLoader*, FFLiBufferAllocator*, FFLiRenderTextureBuffer*);
int FFLiGetTempBufferSizeMaskTextures(unsigned int, unsigned int, bool, bool, const FFLiResourceManager*, FFLResourceType);
int FFLiInitTempObjectFacelineTexture(FFLiFacelineTextureTempObject*, FFLiRenderTexture*, const FFLiCharInfo*, unsigned int, bool, bool, bool, FFLiResourceLoader*, FFLiBufferAllocator*, FFLiRenderTextureBuffer*);
int FFLiGetDepthBufferSizeRenderTexture(unsigned int, unsigned int);
int FFLiGetCompressBufferSizeMaskTexture(unsigned int, bool);
int FFLiGetTempBufferSizeFacelineTexture(unsigned int, bool, bool, const FFLiResourceManager*, FFLResourceType);
int FFLiGetCompressBufferSizeFacelineTexture(unsigned int, bool);
unsigned int FFLiMin(unsigned int, unsigned int);
void FFLSetMatrixDefaultShader(void*, const _Mat44*, const _Mat34*);

#endif
