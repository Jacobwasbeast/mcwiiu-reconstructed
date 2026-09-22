#include "platform/wiiu/FFL.hpp"
#if defined(RECON_NATIVE)
#include "ffl_shape.hpp"
#include "ffl_texture.hpp"
#include "gx2_tiling.hpp"
#endif
#include "platform/wiiu/WiiU_GX2.hpp"
#include "platform/wiiu/FFLiDatabaseManager.hpp"
#define private public
#include "platform/wiiu/FFLiManager.hpp"
#undef private
#include "vendor/__N_17_FFLiCharModel_cpp_08c41fd9.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiUtilShaderAllocator.hpp"
#include "platform/wiiu/FFLiShaderCallback.hpp"
#include "vendor/__N_13_FFLiColor_cpp_87b9af10.hpp"
#include "vendor/__N_15_FFLiManager_cpp_28866420.hpp"
#include "vendor/__N_15_FFLiMiiData_cpp_2f9c0036.hpp"
#include "vendor/__N_21_FFLiDefaultShader_cpp_d69dbbf4.hpp"
#include "vendor/__N_20_FFLDefaultShader_cpp_5ec7e0a3.hpp"
#include "vendor/__N_12_FFLiIcon_cpp_65029ad9.hpp"
#include "vendor/__N_20_FFLiMaskTextures_cpp_feacafbb.hpp"
#include "vendor/__N_21_FFLiResourceShape_cpp_14c50527.hpp"
#include "vendor/__N_23_FFLiFacelineTexture_cpp_cff65387.hpp"
#include "platform/wiiu/FFLiTypes.hpp"
#include "platform/wiiu/FFLiResourceLoader.hpp"
#include "platform/wiiu/FFLiResourceTextureFooter.hpp"
#include "platform/wiiu/FFLiCompressor.hpp"
#include "platform/wiiu/FFLiDatabaseDefault.hpp"
#include "platform/wiiu/FFLiStoreDataCFL.hpp"
#include "vendor/__N_20_FFLiRawMaskParts_cpp_7c9651f3.hpp"
#include "vendor/__N_15_FFLiRawMask_cpp_1637d14a.hpp"
#include "platform/wiiu/FFLiCompressorParam.hpp"

#include <sysapp/title.h>
#include <nn/save/save.h>
#include <string.h>

extern void instructionSynchronize();
extern void GetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);
extern void C_MTXOrtho(float (*matrix)[4], float top, float bottom, float left, float right,
                        float nearDistance, float farDistance);
extern "C" void GX2SetAlphaTest(int, unsigned int, float);
extern "C" void GX2SetColorControl(unsigned int, unsigned char, int, int);
extern "C" void GX2SetDepthOnlyControl(int, int, unsigned int);
extern "C" void GX2SetCullOnlyControl(unsigned int, int, int);
extern "C" void GX2SetBlendControl(unsigned int, unsigned int, unsigned int, unsigned int, int,
                               unsigned int, unsigned int, unsigned int);
extern "C" void GX2DrawDone();
const void* FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
    uint32_t* size, const void* resource, uint32_t parts, uint32_t element);
void FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
    FFLiRenderTexture* texture, const FFLColor* clearColor, void* depthBuffer,
    uint32_t depthSize, uint32_t format, const FFLiShaderCallback* callback);
void FFLiInvalidatePartsTextures__FP17FFLiPartsTextures(FFLiPartsTextures* textures);
void FFLiInvalidateRawMask__FP20FFLiRawMaskDrawParam(void* drawParam);
void FFLiDrawRawMask__FPC20FFLiRawMaskDrawParamPC18FFLiShaderCallback(
    const void* drawParam, const FFLiShaderCallback* callback);
void FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
    void* drawParam, const FFLiRawMaskPartsDesc* description, const _Mat44* matrix,
    FFLiBufferAllocator* allocator);
void FFLiInitDrawParamRawMaskPartsFill__FP25FFLiRawMaskPartsDrawParamP19FFLiBufferAllocator(
    void* drawParam, FFLiBufferAllocator* allocator);

enum FFLAttributeBufferType
{
    FFLAttributeBufferTypePosition = 0,
    FFLAttributeBufferTypeNormal = 1,
    FFLAttributeBufferTypeColor = 2,
    FFLAttributeBufferTypeTexCoord = 3,
    FFLAttributeBufferTypeTangent = 4
};

class __N_13_FFLiShape_cpp_fe662c08
{
public:
    static uint8_t GetElementType(FFLAttributeBufferType type);
    static uint32_t GetStride(FFLAttributeBufferType type, unsigned int stride);
    static void EndianSwap(void* data, unsigned int size);
    static void AdjustAttribute(FFLVec4* value, unsigned int count, float scaleX, float scaleY, float scaleZ, const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate);
    static void AdjustAttributeWithoutScale(FFLiSnorm10_10_10_2* value, unsigned int count, bool flip, const FFLiCoordinate* coordinate);
    static void AdjustAttributeWithoutScale(FFLiSnorm8_8_8_8* value, unsigned int count, bool flip, const FFLiCoordinate* coordinate);
    static void AdjustAttribute(FFLVec3* value, unsigned int count, float scaleX, float scaleY, float scaleZ, const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate);
};

static int32_t FFLiReadCharInfoValue(const FFLiCharInfo* info, uint32_t offset)
{
    int32_t value;
    memcpy(&value, info->data + offset, sizeof(value));
    return value;
}

static FFLiDefaultShader sDefaultShader;
static void* sDefaultShaderWork;
#include "platform/wiiu/FFL_gFFLIconShaderData.inc"

uint32_t gOffScreenSrgbFetchMode = 0;
uint8_t gOffScreenSrgbFetchEnabled = 1;


extern "C" void GX2SetContextState(const void*);
extern "C" void GX2SetShaderModeEx(uint32_t, uint32_t, uint32_t, uint32_t,
                                    uint32_t, uint32_t, uint32_t);
extern "C" void GX2SetFetchShader(const void*);
extern "C" void GX2SetVertexShader(const void*);
extern "C" void GX2SetPixelShader(const void*);
extern "C" void GX2InitSampler(void*, unsigned int, unsigned int);
extern "C" void GX2InitSamplerLOD(_GX2Sampler*, float, float, float);
extern "C" void GX2SetupContextStateEx(void*, unsigned int);
extern "C" void GX2SetVertexUniformReg(unsigned int, unsigned int, const void*);
extern void ASM_MTX44Transpose(const float (*)[4], float (*)[4]);
extern void InitVertexShader__FPP16_GX2VertexShaderPCv(_GX2VertexShader**, const void*);
extern void InitPixelShader__FPP15_GX2PixelShaderPCv(_GX2PixelShader**, const void*);
extern void InitFetchShader__FP11FetchShaderPC16_GX2VertexShader(
    FFLiUtilShader::FetchShader*, const _GX2VertexShader*);


static const FFLColor* asColor(const float* color)
{
    return reinterpret_cast<const FFLColor*>(color);
}

static const FFLColor sTransparent = { 0.0f, 0.0f, 0.0f, 0.0f };

void FFLiGetAdditionalInfo__FP17FFLAdditionalInfoPC12FFLiCharInfoi13FFLFontRegion(
    FFLAdditionalInfo*, const FFLiCharInfo*, int, uint32_t);

uint32_t FFLiMax__tm__3_Ui__FZ1ZT1_Z1Z(uint32_t lhs, uint32_t rhs) { return lhs < rhs ? rhs : lhs; }
uint32_t FFLiMin__tm__3_Ui__FZ1ZT1_Z1Z(uint32_t lhs, uint32_t rhs) { return rhs < lhs ? rhs : lhs; }
float FFLiMin__tm__2_f__FZ1ZT1_Z1Z(float lhs, float rhs) { return rhs < lhs ? rhs : lhs; }
float FFLiMax__tm__2_f__FZ1ZT1_Z1Z(float lhs, float rhs) { return lhs < rhs ? rhs : lhs; }
bool FFLiRange__tm__2_i__FZ1ZN21_b(int minimum, int maximum, int value) { return minimum <= value && value <= maximum; }

uint32_t FFLiRoundUp(uint32_t value, uint32_t alignment) { return (value + alignment - 1) & ~(alignment - 1); }
void* FFLiRoundUpPtr__FPvUi(void* pointer, uint32_t alignment)
{
    uintptr_t value = reinterpret_cast<uintptr_t>(pointer);
    return reinterpret_cast<void*>((value + alignment - 1) & ~(uintptr_t)(alignment - 1));
}
bool FFLiCheckAlign__FUiT1(uint32_t value, uint32_t alignment) { return (value & (alignment - 1)) == 0; }
bool FFLiCheckAlignPtr__FPCvUi(const void* pointer, uint32_t alignment) { return (reinterpret_cast<uintptr_t>(pointer) & (alignment - 1)) == 0; }

uint8_t FFLiSwapEndianImpl__tm__3_Uc__FZ1Z_Z1Z(uint8_t value) { return value; }
uint16_t FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(uint16_t value) { return static_cast<uint16_t>((value << 8) | (value >> 8)); }
uint32_t FFLiSwapEndianImpl__tm__3_Ui__FZ1Z_Z1Z(uint32_t value)
{
    return ((value & 0xffu) << 24) | ((value & 0xff00u) << 8) | ((value & 0xff0000u) >> 8) | ((value & 0xff000000u) >> 24);
}
float FFLiSwapEndianImpl__tm__2_f__FZ1Z_Z1Z(float value)
{
    uint32_t bits;
    memcpy(&bits, &value, sizeof(bits));
    bits = FFLiSwapEndianImpl__tm__3_Ui__FZ1Z_Z1Z(bits);
    memcpy(&value, &bits, sizeof(value));
    return value;
}
void FFLiSwapEndianArrayImpl__tm__3_Uc__FPZ1ZUi_v(uint8_t*, uint32_t) {}
void FFLiSwapEndianArrayImpl__tm__3_Us__FPZ1ZUi_v(uint16_t* values, uint32_t count) { for (uint32_t i = 0; i < count; ++i) values[i] = FFLiSwapEndianImpl__tm__3_Us__FZ1Z_Z1Z(values[i]); }
void FFLiSwapEndianArrayImpl__tm__3_Ui__FPZ1ZUi_v(uint32_t* values, uint32_t count) { for (uint32_t i = 0; i < count; ++i) values[i] = FFLiSwapEndianImpl__tm__3_Ui__FZ1Z_Z1Z(values[i]); }
void FFLiSwapEndianArrayImpl__tm__2_f__FPZ1ZUi_v(float* values, uint32_t count) { for (uint32_t i = 0; i < count; ++i) values[i] = FFLiSwapEndianImpl__tm__2_f__FZ1Z_Z1Z(values[i]); }

struct FFLiSwapEndianDesc
{
    uint32_t type;
    uint32_t count;
};

int FFLiSwapEndianGroup__FPvPC18FFLiSwapEndianDescUi(void* data,
                                                     const FFLiSwapEndianDesc* descriptions,
                                                     uint32_t count)
{
    uint8_t* cursor = static_cast<uint8_t*>(data);
    for (uint32_t i = 0; i < count; ++i)
    {
        if (descriptions[i].type == 0)
            FFLiSwapEndianArrayImpl__tm__3_Uc__FPZ1ZUi_v(cursor, descriptions[i].count);
        else if (descriptions[i].type == 1)
            FFLiSwapEndianArrayImpl__tm__3_Us__FPZ1ZUi_v(reinterpret_cast<uint16_t*>(cursor), descriptions[i].count);
        else if (descriptions[i].type == 2)
            FFLiSwapEndianArrayImpl__tm__3_Ui__FPZ1ZUi_v(reinterpret_cast<uint32_t*>(cursor), descriptions[i].count);
        cursor += descriptions[i].count * (descriptions[i].type < 3 ? (1u << descriptions[i].type) : 0u);
    }
    return static_cast<int>(cursor - static_cast<uint8_t*>(data));
}

void FFLiInitBufferAllocator__FP19FFLiBufferAllocatorPvUi(FFLiBufferAllocator* allocator, void* buffer, uint32_t size) { allocator->Init(buffer, size); }
uint32_t FFLiAllocateBufferAllocator__FP19FFLiBufferAllocatorUiT2(FFLiBufferAllocator* allocator, uint32_t size, uint32_t alignment) { return allocator->Allocate(size, alignment); }
void FFLiAllocateBufferAllocator__FP19FFLiBufferAllocatorUi(FFLiBufferAllocator* allocator, uint32_t size) { allocator->Allocate(size); }
void FFLiBugEndianSwap__FPvUi(void*, uint32_t) {}
uint32_t FFLiBugCanSwapSize__FUi(uint32_t size) { return (size + 3) & ~3u; }
void* FFLiBugVgtFixedIndexPtr__FPv(void* pointer) { return static_cast<uint8_t*>(pointer) + 0x1c; }
uint32_t FFLiBugCanVgtFixedIndexSize__FUi(uint32_t size) { return size + 0x1c; }
uint32_t FFLiCalculateUnitTextureResolution__FUiT1(uint32_t resolution, uint32_t level)
{
    uint32_t reduced = resolution >> level;
    return ((reduced + ((4 - reduced) & 3)) << level) >> 2;
}

bool FFLiIsLeapYear__Fi(int year) { return year % 400 == 0 || (year % 100 != 0 && year % 4 == 0); }
uint32_t FFLiGetMonthOfDay__Fi(int month)
{
    static const uint8_t days[] = { 0, 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    return month >= 1 && month <= 12 ? days[month] : 0;
}
int FFLiDateToDays__FiN21(int year, int month, int day)
{
    int days = day;
    for (int current = 0; current < year; ++current) days += FFLiIsLeapYear__Fi(current) ? 366 : 365;
    for (int current = 1; current < month; ++current) days += FFLiGetMonthOfDay__Fi(current) - (current == 2 && !FFLiIsLeapYear__Fi(year));
    return days;
}
uint32_t FFLiGetMipMapLevelSize__FUiT1(uint32_t baseSize, uint32_t level) { return baseSize >> (level * 2); }
int FFLiGetMipMapNum(int width, int height)
{
    int count = 1;
    while (width > 32 && height > 32) { width /= 2; height /= 2; ++count; }
    return count;
}
bool FFLiVerifyBirthday__FiT1(int month, int day) { return month >= 1 && month <= 12 && day >= 1 && day <= (int)FFLiGetMonthOfDay__Fi(month); }

bool FFLiIsNumberCharCode__FUs(uint16_t c)
{
    return (c >= 0x30 && c <= 0x39) || (c >= 0xff10 && c <= 0xff19) ||
           (c >= 0x2160 && c <= 0x2169) || (c >= 0x2170 && c <= 0x2179) ||
           (c >= 0x2460 && c <= 0x2473) || (c >= 0x2776 && c <= 0x277e);
}
bool FFLiIsValidCharacterForName__FUs(uint16_t c) { return c != '%' && c != '\\'; }
void FFLiCopyWcharT2U16__FPUsUiPCw(uint16_t* destination, uint32_t capacity, const wchar_t* source)
{
    if (capacity == 0) return;
    uint32_t index = 0;
    while (index + 1 < capacity && source[index] != 0) { destination[index] = (uint16_t)source[index]; ++index; }
    destination[index] = 0;
}

uint16_t FFLiGetCRC16__FUsPCvUi(uint16_t crc, const void* data, uint32_t size)
{
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    for (uint32_t index = 0; index < size; ++index) {
        crc ^= (uint16_t)bytes[index] << 8;
        for (int bit = 0; bit < 8; ++bit) crc = (uint16_t)((crc << 1) ^ ((crc & 0x8000) ? 0x1021 : 0));
    }
    return crc;
}
void FFLiPutCRC16(uint16_t initial, void* data, uint32_t size)
{
    uint16_t* stored = reinterpret_cast<uint16_t*>(static_cast<uint8_t*>(data) + size - 2);
    *stored = 0;
    *stored = FFLiGetCRC16__FUsPCvUi(initial, data, size);
}
bool FFLiIsValidCRC16__FPCvUi(const void* data, uint32_t size) { return FFLiGetCRC16__FUsPCvUi(0, data, size) == 0; }

bool FFLiIsNullMiiID(const FFLiMiiID* id)
{
    if (id == 0) return false;
    static const FFLiMiiID empty = { { 0 } };
    return memcmp(id, &empty, sizeof(*id)) == 0;
}
bool FFLiIsNormalMiiID(const FFLiMiiID* id) { return (id->value[0] & 0x80) != 0; }
bool FFLiIsSpecialMiiID(const FFLiMiiID* id) { return !FFLiIsNormalMiiID(id); }
bool FFLiIsNTRMiiID(const FFLiMiiID* id) { return (id->value[0] & 0x50) == 0x40; }
bool FFLiIsTemporaryMiiID(const FFLiMiiID* id) { return (id->value[0] & 0x20) != 0; }
bool FFLiIsValidMiiID(const FFLiMiiID* id) { return !FFLiIsNullMiiID(id) && !FFLiIsTemporaryMiiID(id); }
bool FFLiIsSameMiiID(const FFLiMiiID* lhs, const FFLiMiiID* rhs) { return FFLiIsValidMiiID(lhs) && FFLiIsValidMiiID(rhs) && memcmp(lhs, rhs, sizeof(*lhs)) == 0; }

uint8_t FFLiGetTextureResourceNum__F20FFLiTexturePartsType(uint32_t type)
{
    static const uint8_t counts[] = { 0x03,0x84,0x3e,0x18,0x0c,0x0c,0x09,0x02,0x25,0x06,0x12 };
    return type < sizeof(counts) ? counts[type] : 0;
}
uint8_t FFLiGetShapeResourceNum__F18FFLiShapePartsType(uint32_t type)
{
    static const uint8_t counts[] = { 0x04,0x84,0x84,0x0c,0x01,0x0c,0x12,0x12,0x84,0x84,0x84,0x84 };
    return type < sizeof(counts) ? counts[type] : 0;
}
uint8_t FFLiResourceWindowBitsToZlibWindowBits__F22FFLiResourceWindowBits(uint32_t value)
{
    static const uint8_t bits[] = { 8,9,10,11,12,13,14,15,24,25,26,27,28,29,30,31,15 };
    return value < sizeof(bits) ? bits[value] : 15;
}
uint32_t FFLiGetResourceSurfaceFormat__F17FFLiTextureFormat(int format) { return format == 1 ? 7 : (format == 2 ? 0x1a : 1); }
uint32_t FFLiConvertFSStatusToFFLResult__Fi(int status) { return status == 0 ? 0 : 0x12; }

static FFLiColorContainer& colorContainer() { return *__N_13_FFLiColor_cpp_87b9af10::GetColorContainer(); }
static FFLiColorContainer& srgbContainer() { return *__N_13_FFLiColor_cpp_87b9af10::GetSrgbFetchColorContainer(); }

const FFLColor* FFLiGetSrgbFetchFacelineColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetFacelineColor(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchBeardColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetBeardColor(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchEyebrowColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetEyebrowColor(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchMustacheColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetMustacheColor(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchEyeColorR__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetEyeColorR(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchEyeColorG__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetEyeColorG(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchEyeColorB__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetEyeColorB(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchMouthColorR__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetMouthColorR(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchMouthColorG__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetMouthColorG(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchMouthColorB__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetMouthColorB(srgbContainer(), i)); }
const FFLColor* FFLiGetSrgbFetchMoleColor__Fv() { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetMoleColor(srgbContainer())); }
const FFLColor* FFLiGetFacelineColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetFacelineColor(colorContainer(), i)); }
const FFLColor* FFLiGetHairColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetHairColor(colorContainer(), i)); }
const FFLColor* FFLiGetGlassColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetGlassColor(colorContainer(), i)); }
FFLColor* FFLiGetFavoriteColor__Fi(int i)
{
    return reinterpret_cast<FFLColor*>(const_cast<float*>(
        __N_13_FFLiColor_cpp_87b9af10::GetFavoriteColor(colorContainer(), i)));
}
const FFLColor* FFLiGetFaceLine__Fv() { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetFaceLine(colorContainer())); }
const FFLColor* FFLiGetCapColor__Fi(int i) { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetCapColor(colorContainer(), i)); }
const FFLColor* FFLiGetNoselineColor__Fv() { return asColor(__N_13_FFLiColor_cpp_87b9af10::GetNoselineColor(colorContainer())); }

int FFLiiGetEyeRotateOffset__Fi(int index)
{
    static const uint8_t offsets[62] = {
        3,4,4,4,3,4,4,4,3,4,4,4,4,3,3,4,
        4,4,3,3,4,3,4,3,3,4,3,4,4,3,4,4,
        4,3,3,3,4,4,3,3,3,4,4,3,3,3,3,3,
        3,3,3,3,4,4,4,4,3,4,4,3,4,4
    };
    return 0x20 - offsets[index];
}
int FFLiiGetEyebrowRotateOffset__Fi(int index)
{
    static const uint8_t offsets[24] = {
        6,6,5,7,6,7,6,7,4,7,6,8,5,5,6,6,
        7,7,6,6,5,6,7,5
    };
    return 0x20 - offsets[index];
}
float FFLiiGetAdjustedMouthH__Ffi(float height, int type)
{
    bool adjusted = type == 3 || type == 15 || (type >= 19 && type <= 21) || type == 23 || type == 25;
    return adjusted && height < 12.0f ? 12.0f : height;
}
float FFLiiGetAdjustedEyeH__Ffi(float height, int type) { return (type == 14 || type == 26) && height < 12.0f ? 12.0f : height; }

static void initModulate(FFLModulateParam* param, uint32_t mode, uint32_t type,
                         const FFLColor* r, const FFLColor* g, const FFLColor* b,
                         const _GX2Texture* texture)
{
    param->mode = mode;
    param->type = type;
    param->colorR = r;
    param->colorG = g;
    param->colorB = b;
    param->texture = texture;
}

void FFLiInitModulateMustache__FP16FFLModulateParamiRC11_GX2Texture(FFLModulateParam* p, int i, const _GX2Texture& t) { initModulate(p,3,9,FFLiGetSrgbFetchMustacheColor__Fi(i),0,0,&t); }
void FFLiInitModulateMouth__FP16FFLModulateParamiRC11_GX2Texture(FFLModulateParam* p, int i, const _GX2Texture& t) { initModulate(p,2,10,FFLiGetSrgbFetchMouthColorR__Fi(i),FFLiGetSrgbFetchMouthColorG__Fi(i),FFLiGetSrgbFetchMouthColorB__Fi(i),&t); }
void FFLiInitModulateEyebrow__FP16FFLModulateParamiRC11_GX2Texture(FFLModulateParam* p, int i, const _GX2Texture& t) { initModulate(p,3,11,FFLiGetSrgbFetchEyebrowColor__Fi(i),0,0,&t); }
void FFLiInitModulateEye__FP16FFLModulateParamiT2RC11_GX2Texture(FFLModulateParam* p, int iris, int sclera, const _GX2Texture& t) { initModulate(p,2,12,FFLiGetSrgbFetchEyeColorR__Fi(sclera),FFLiGetSrgbFetchEyeColorG__Fi(iris),FFLiGetSrgbFetchEyeColorB__Fi(iris),&t); }
void FFLiInitModulateMole__FP16FFLModulateParamRC11_GX2Texture(FFLModulateParam* p, const _GX2Texture& t) { initModulate(p,3,13,FFLiGetSrgbFetchMoleColor__Fv(),0,0,&t); }
void FFLiInitModulateFaceMake__FP16FFLModulateParamRC11_GX2Texture(FFLModulateParam* p, const _GX2Texture& t) { initModulate(p,1,14,0,0,0,&t); }
void FFLiInitModulateFaceLine__FP16FFLModulateParamRC11_GX2Texture(FFLModulateParam* p, const _GX2Texture& t) { initModulate(p,3,15,FFLiGetFaceLine__Fv(),0,0,&t); }
void FFLiInitModulateFaceBeard__FP16FFLModulateParamiRC11_GX2Texture(FFLModulateParam* p, int i, const _GX2Texture& t) { initModulate(p,3,16,FFLiGetSrgbFetchBeardColor__Fi(i),0,0,&t); }
void FFLiInitModulateShapeFaceline__FP16FFLModulateParamRC11_GX2Texture(FFLModulateParam* p, const _GX2Texture& t) { initModulate(p,1,0,0,0,0,&t); }
void FFLiInitModulateShapeBeard__FP16FFLModulateParami(FFLModulateParam* p, int i) { initModulate(p,0,1,FFLiGetHairColor__Fi(i),0,0,0); }
void FFLiInitModulateShapeNose__FP16FFLModulateParami(FFLModulateParam* p, int i) { initModulate(p,0,2,FFLiGetFacelineColor__Fi(i),0,0,0); }
void FFLiInitModulateShapeForehead__FP16FFLModulateParami(FFLModulateParam* p, int i) { initModulate(p,0,3,FFLiGetFacelineColor__Fi(i),0,0,0); }
void FFLiInitModulateShapeHair__FP16FFLModulateParami(FFLModulateParam* p, int i) { initModulate(p,0,4,FFLiGetHairColor__Fi(i),0,0,0); }
void FFLiInitModulateShapeCap__FP16FFLModulateParamiRC11_GX2Texture(FFLModulateParam* p, int i, const _GX2Texture& t) { initModulate(p,5,5,FFLiGetCapColor__Fi(i),0,0,&t); }
void FFLiInitModulateShapeMask__FP16FFLModulateParamRC11_GX2Texture(FFLModulateParam* p, const _GX2Texture& t) { initModulate(p,1,6,0,0,0,&t); }
void FFLiInitModulateShapeNoseline__FP16FFLModulateParamRC11_GX2Texture(FFLModulateParam* p, const _GX2Texture& t) { initModulate(p,3,7,FFLiGetNoselineColor__Fv(),0,0,&t); }
void FFLiInitModulateShapeGlass__FP16FFLModulateParamiRC11_GX2Texture(FFLModulateParam* p, int i, const _GX2Texture& t) { initModulate(p,4,8,FFLiGetGlassColor__Fi(i),0,0,&t); }
void FFLiInitModulateFill__FP16FFLModulateParam(FFLModulateParam* p) { initModulate(p,0,17,&sTransparent,0,0,0); }

extern "C" void GX2CalcSurfaceSizeAndAlignment(_GX2Surface*);
extern "C" void GX2InitTextureRegs(_GX2Texture*);
extern "C" void GX2Invalidate(uint32_t, void*, uint32_t);
void FFLiDestroyDefaultShader__FP17FFLiDefaultShader(FFLiDefaultShader* shader);
int FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
    _GX2Texture**, uint32_t, uint32_t, FFLiResourceLoader*, FFLiBufferAllocator*);
void FFLiInvalidateTexture__FP11_GX2Texture(_GX2Texture*);

int FFLiGetBufferRenderTexture__FUiT117_GX2SurfaceFormatT1(uint32_t width, uint32_t height, uint32_t format, uint32_t mipCount)
{
    _GX2Texture texture = {};
    GX2InitTexture(&texture, width, height, 0, mipCount,
                   static_cast<_GX2SurfaceFormat>(format), GX2_SURFACE_DIM_TEXTURE_2D);
    return texture.surface.alignment + FFLiRoundUp(texture.surface.imageSize, texture.surface.alignment) +
           FFLiRoundUp(texture.surface.mipSize, texture.surface.alignment);
}

int FFLiGetDepthBufferSizeRenderTexture__FUiT1(uint32_t width, uint32_t height)
{
    _GX2DepthBuffer depth = {};
    GX2InitDepthBuffer(&depth, width, height, GX2_SURFACE_FORMAT_UNORM_D24_S8, GX2_AA_MODE1X);
    return depth.surface.imageSize + depth.hiZSize;
}

void FFLiInvalidate__F18_GX2InvalidateTypePvUi(uint32_t type, void* data, uint32_t size) { GX2Invalidate(type, data, size); }
void FFLiInvalidateSurface__FP11_GX2Surface18_GX2InvalidateType(_GX2Surface* surface, uint32_t type)
{
    if (surface->imageSize != 0) FFLiInvalidate__F18_GX2InvalidateTypePvUi(type, surface->imagePtr, surface->imageSize);
    if (surface->mipSize != 0) FFLiInvalidate__F18_GX2InvalidateTypePvUi(type, surface->mipPtr, surface->mipSize);
}
void FFLiInvalidateColorBuffer__FP15_GX2ColorBuffer(_GX2ColorBuffer* buffer) { FFLiInvalidateSurface__FP11_GX2Surface18_GX2InvalidateType(&buffer->surface, 0x10); }
void FFLiInvalidateShader__FPvUi(void* shader, uint32_t size) { FFLiInvalidate__F18_GX2InvalidateTypePvUi(0x42, shader, size); }
void FFLiInvalidateRenderTexture__FP17FFLiRenderTexture(FFLiRenderTexture* texture) { if (texture) FFLiInvalidateSurface__FP11_GX2Surface18_GX2InvalidateType(&texture->texture.surface, 0x42); }
void FFLiInvalidateTexture__FP11_GX2Texture(_GX2Texture* texture) { if (texture) FFLiInvalidateSurface__FP11_GX2Surface18_GX2InvalidateType(&texture->surface, 0x42); }
void FFLiFlushRenderTexture__FP17FFLiRenderTexture(FFLiRenderTexture* texture) { FFLiInvalidateSurface__FP11_GX2Surface18_GX2InvalidateType(&texture->texture.surface, 0x10); }

int FFLiGetBufferRawMaskParts__Fv()
{
    uint32_t indices = FFLiRoundUp(FFLiBugCanVgtFixedIndexSize__FUi(8), 0x20);
    uint32_t positions = FFLiRoundUp(FFLiBugCanSwapSize__FUi(0x40), 0x40);
    uint32_t texCoords = FFLiRoundUp(FFLiBugCanSwapSize__FUi(0x20), 0x40);
    return indices + 0x40 + positions + texCoords;
}
int FFLiGetBufferRawMask__Fv() { return FFLiGetBufferRawMaskParts__Fv() * 9; }

uint32_t FFLiGetMaxMouthNum__FUi(uint32_t count) { return FFLiMin__tm__3_Ui__FZ1ZT1_Z1Z(count, 5); }
uint32_t FFLiGetMaxEyeNum__FUi(uint32_t count) { return FFLiMin__tm__3_Ui__FZ1ZT1_Z1Z(count + 1, 6); }

int FFLiGetBufferSizeCharModel__FPC16FFLCharModelDesc(const FFLCharModelDesc* description)
{
    if (!FFLiManager::IsConstruct()) return 0;
    return FFLiManager::GetInstance()->m_charModelCreateParam.GetBufferSize(description);
}

const FFLShaderCallback* FFLiGetCallbackDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
int FFLiInitDefaultShader__FP17FFLiDefaultShaderPvPCv(FFLiDefaultShader*, void*, const void*);
void FFLiSetDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
void FFLiSetupInitCharModelDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
void FFLiSetupDrawDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
_GX2ContextState* FFLiGetContextStateDefaultShader__FP17FFLiDefaultShader(FFLiDefaultShader*);
void FFLiSetMatrixDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34(
    FFLiDefaultShader*, const _Mat44*, const _Mat34*);
void FFLiGetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);

int FFLInitDefaultShader(void* shader, void* buffer, const void* shaderData)
{
    return FFLiInitDefaultShader__FP17FFLiDefaultShaderPvPCv(
        reinterpret_cast<FFLiDefaultShader*>(shader), buffer, shaderData);
}

void FFLDestroyDefaultShader(void* shader)
{
    FFLiDestroyDefaultShader__FP17FFLiDefaultShader(
        reinterpret_cast<FFLiDefaultShader*>(shader));
}

void FFLSetDefaultShader(void* shader)
{
    FFLiSetDefaultShader__FPC17FFLiDefaultShader(
        reinterpret_cast<const FFLiDefaultShader*>(shader));
}

void FFLSetupInitCharModelDefaultShader(void* shader)
{
    FFLiSetupInitCharModelDefaultShader__FPC17FFLiDefaultShader(
        reinterpret_cast<const FFLiDefaultShader*>(shader));
}

void FFLSetupDrawDefaultShader(void* shader)
{
    FFLiSetupDrawDefaultShader__FPC17FFLiDefaultShader(
        reinterpret_cast<const FFLiDefaultShader*>(shader));
}

void* FFLGetContextStateDefaultShader(void* shader)
{
    return FFLiGetContextStateDefaultShader__FP17FFLiDefaultShader(
        reinterpret_cast<FFLiDefaultShader*>(shader));
}

void FFLSetMatrixDefaultShader(const _Mat44* projection, const _Mat34* view)
{
    FFLiSetMatrixDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34(
        &sDefaultShader, projection, view);
}

void FFLSetMatrixDefaultShader(void* shader, const _Mat44* projection, const _Mat34* view)
{
    FFLiSetMatrixDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34(
        shader != 0 ? reinterpret_cast<FFLiDefaultShader*>(shader) : &sDefaultShader,
        projection, view);
}

void FFLiRenderTexture::CopyTo(_GX2Texture* destination)
{
    if (destination != 0) *destination = texture;
}

unsigned int FFLiRenderTexture::GetSurfaceFormat() const
{
    return texture.surface.format;
}

int FFLInitRes(void* work, const FFLInitDesc* initDesc)
{
    return __N_15_FFLiManager_cpp_28866420::InitResImpl(work, initDesc, 0);
}

void FFLInitResGPUStep(void)
{
    if (FFLiManager::IsConstruct()) FFLiManager::GetInstance()->SetupGPU();
}

int FFLGetWorkSize(const FFLInitDesc* initDesc)
{
    return FFLiManager::GetBufferSize(initDesc);
}

int FFLExit(void)
{
    return static_cast<int>(FFLiManager::Destroy());
}

unsigned int FFLGetIconBufferSize(const FFLIconSetting* setting)
{
    return static_cast<unsigned int>(
        FFLiGetBufferRenderTexture__FUiT117_GX2SurfaceFormatT1(
            setting->width, setting->height, setting->surfaceFormat, 1));
}

#include "platform/wiiu/FFL_gFFLDefaultShaderData.inc"

unsigned int FFLMakeIconExWithCallback(
    _GX2Texture* texture, const FFLCharModel* model, FFLExpression expression,
    const FFLIconSetting* setting, void* buffer, void* temporaryBuffer,
    const FFLShaderCallback* callback, bool flush)
{
    return __N_12_FFLiIcon_cpp_65029ad9::MakeIconImpl(
        texture, &model->model, expression, setting, buffer, temporaryBuffer,
        callback, flush);
}

unsigned int FFLMakeIconEx(
    _GX2Texture* texture, const FFLCharModel* model, FFLExpression expression,
    const FFLIconSetting* setting, void* buffer, void* temporaryBuffer)
{
    const FFLShaderCallback* callback = 0;
    if (FFLiManager::IsConstruct())
        callback = FFLiManager::GetInstance()->GetShaderCallback();
    return FFLMakeIconExWithCallback(texture, model, expression, setting,
                                      buffer, temporaryBuffer, callback, true);
}

void FFLGetIconMatrix(_Mat44* projection, _Mat34* view,
                      unsigned short width, unsigned short height)
{
    FFLiGetIconMatrix__FP6_Mat44P6_Mat34UsT3(projection, view, width, height);
}

static const FFLiUtilShader::AttributeStreamDesc sDefaultShaderAttributes[5] = {
    { "a_position", 0, 0, 0x811 },
    { "a_texCoord", 1, 0, 0x80d },
    { "a_normal", 2, 0, 0x20b },
    { "a_tangent", 3, 0, 0x20a },
    { "a_color", 4, 0, 0x0a }
};

int FFLiGetBufferSizeDefaultShader__FPCv(const void* shaderData);

int FFLiInitDefaultShader__FP17FFLiDefaultShaderPvPCv(
    FFLiDefaultShader* shader, void* buffer, const void* shaderData)
{
    if (FFLiCheckAlign__FUiT1(reinterpret_cast<uint32_t>(buffer), 0x100) == 0) return 1;
    memset(shader, 0, sizeof(*shader));

    FFLiBufferAllocator bufferAllocator;
    bufferAllocator.Init(buffer, FFLiGetBufferSizeDefaultShader__FPCv(shaderData));
    shader->m_callbackData = shader;
    shader->m_contextState = reinterpret_cast<_GX2ContextState*>(
        bufferAllocator.Allocate(0xa100, 0x100));
    shader->m_drawCallback = &__N_21_FFLiDefaultShader_cpp_d69dbbf4::ShaderCallbackDraw;
    shader->m_matrixCallback = &__N_21_FFLiDefaultShader_cpp_d69dbbf4::ShaderCallbackSetMatrix;
    GX2SetupContextStateEx(shader->m_contextState, 1);

    FFLiUtilShaderAllocator allocator(&bufferAllocator, 5, shaderData, 0);
    bool success = FFLiUtilShader::BuildShader(
        &shader->m_vertexShader, &allocator, shaderData, 0, true);
    if (success)
        success = FFLiUtilShader::BuildShader(
            &shader->m_pixelShader, &allocator, shaderData, 0, true);
    if (success)
        success = FFLiUtilShader::BuildShader(
            &shader->m_fetchShader, shader->m_vertexShader, &allocator,
            sDefaultShaderAttributes, 5, true);
    if (!success) return 1;

    GX2InitSampler(&shader->m_sampler, 1, 1);
    GX2InitSampler(&shader->m_samplerAlt, 1, 1);
    GX2InitSamplerLOD(&shader->m_samplerAlt, 0.0f, 15.99899959564209f, 0.5f);
    shader->m_enabled = 0;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_vertexUniformMatrix, shader->m_vertexShader, "u_proj")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_vertexUniformView, shader->m_vertexShader, "u_mv")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_vertexUniformNormal, shader->m_vertexShader, "u_it")) return 1;
    if (!FFLiUtilShader::GetSamplerLocation(
        &shader->m_pixelSamplerLocation, shader->m_pixelShader, "s_texture")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_mode, shader->m_pixelShader, "u_mode")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_pixelConst1, shader->m_pixelShader, "u_const1")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_pixelConst2, shader->m_pixelShader, "u_const2")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_pixelConst3, shader->m_pixelShader, "u_const3")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_lightEnable, shader->m_pixelShader, "u_light_enable")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_lightAmbient, shader->m_pixelShader, "u_light_ambient")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_lightDiffuse, shader->m_pixelShader, "u_light_diffuse")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_lightSpecular, shader->m_pixelShader, "u_light_specular")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_lightDirection, shader->m_pixelShader, "u_light_dir")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_materialAmbient, shader->m_pixelShader, "u_material_ambient")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_materialDiffuse, shader->m_pixelShader, "u_material_diffuse")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_materialSpecular, shader->m_pixelShader, "u_material_specular")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_materialSpecularPower, shader->m_pixelShader, "u_material_specular_power")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_materialSpecularMode, shader->m_pixelShader, "u_material_specular_mode")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_rimColor, shader->m_pixelShader, "u_rim_color")) return 1;
    if (!FFLiUtilShader::GetUniformLocation(
        &shader->m_rimPower, shader->m_pixelShader, "u_rim_power")) return 1;
    __N_21_FFLiDefaultShader_cpp_d69dbbf4::InitParam(
        reinterpret_cast<FFLiDefaultShaderParam*>(shader->m_parameterStorage));
    return 0;
}

void FFLiSetDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    GX2SetFetchShader(&shader->m_fetchShader);
    GX2SetVertexShader(shader->m_vertexShader);
    GX2SetPixelShader(shader->m_pixelShader);
}

void FFLiSetupInitCharModelDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    FFLiShaderCallback callback;
    callback.Set(reinterpret_cast<const FFLShaderCallback*>(shader));
    callback.CallSetContextState();
    FFLiSetDefaultShader__FPC17FFLiDefaultShader(shader);
    __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetUniformLightDisable(shader);
}

void FFLiSetupInitCharModelDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    if (FFLiManager::IsConstruct())
        FFLiManager::GetInstance()->m_shaderCallback.Set(
            FFLiGetCallbackDefaultShader__FPC17FFLiDefaultShader(shader));
    FFLiSetupInitCharModelDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(shader);
}

void FFLiSetupDrawDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    FFLiSetDefaultShader__FPC17FFLiDefaultShader(shader);
    __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformParam(shader);
}

void FFLiSetupDrawDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    if (FFLiManager::IsConstruct())
        FFLiManager::GetInstance()->m_shaderCallback.Set(
            FFLiGetCallbackDefaultShader__FPC17FFLiDefaultShader(shader));
    FFLiSetupDrawDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(shader);
}

_GX2ContextState* FFLiGetContextStateDefaultShader__FP17FFLiDefaultShader(
    FFLiDefaultShader* shader)
{
    return shader->m_contextState;
}

void FFLiSetMatrixExDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34T3(
    FFLiDefaultShader* shader, const _Mat44* projection, const _Mat34* view,
    const _Mat34* normal)
{
    if (shader == 0 || projection == 0 || view == 0) return;
    _Mat44 matrix;
    for (unsigned int row = 0; row < 3; ++row)
        for (unsigned int column = 0; column < 4; ++column)
            matrix.value[row][column] = view->value[row][column];
    matrix.value[3][0] = matrix.value[3][1] = matrix.value[3][2] = 0.0f;
    matrix.value[3][3] = 1.0f;
    ASM_MTX44Transpose(projection->value, matrix.value);
    GX2SetVertexUniformReg(shader->m_vertexUniformMatrix, 16, matrix.value);
    if (normal != 0) GX2SetVertexUniformReg(shader->m_vertexUniformNormal, 12, normal->value);
}

void FFLiSetMatrixDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34(
    FFLiDefaultShader* shader, const _Mat44* projection, const _Mat34* view)
{
    FFLiSetMatrixExDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34T3(
        shader, projection, view, 0);
}

uint32_t FFLiGetTextureMaxSizeWithAlign__FPC19FFLiResourceManager15FFLResourceType20FFLiTexturePartsType(
    const FFLiResourceManager*, FFLResourceType, FFLiTexturePartsType);

int FFLiGetTempBufferSizeFacelineTexture__FUibT2P19FFLiResourceManager15FFLResourceType(
    uint32_t resolution, bool mipmap, bool useMipmap,
    FFLiResourceManager* resourceManager, FFLResourceType resourceType)
{
    int size = static_cast<int>(FFLiGetTextureMaxSizeWithAlign__FPC19FFLiResourceManager15FFLResourceType20FFLiTexturePartsType(
        resourceManager, resourceType,
        static_cast<FFLiTexturePartsType>(FFLiTexturePartsType_0 + 4)));
    size += static_cast<int>(FFLiGetTextureMaxSizeWithAlign__FPC19FFLiResourceManager15FFLResourceType20FFLiTexturePartsType(
        resourceManager, resourceType,
        static_cast<FFLiTexturePartsType>(FFLiTexturePartsType_0 + 5)));
    size += static_cast<int>(FFLiGetTextureMaxSizeWithAlign__FPC19FFLiResourceManager15FFLResourceType20FFLiTexturePartsType(
        resourceManager, resourceType, FFLiTexturePartsType_0));
    size += static_cast<int>(__N_23_FFLiFacelineTexture_cpp_cff65387::GetDrawParamBufferSize()) * 3;
    if (useMipmap)
    {
        const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
        const uint32_t mipCount = __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(width, resolution, mipmap);
        size += 0xa4 + FFLiCompressorParam::GetBufferSize(mipCount);
    }
    return size;
}

bool FFLiUseOffScreenSrgbFetch__Fv();
void FFLiInitRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P19FFLiBufferAllocator(
    FFLiRenderTexture*, uint32_t, uint32_t, uint32_t, uint32_t, FFLiBufferAllocator*);

void FFLiInitByBufferRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P23FFLiRenderTextureBuffer(
    FFLiRenderTexture*, uint32_t, uint32_t, uint32_t, uint32_t, FFLiRenderTextureBuffer*);

void FFLiInitFacelineTexture__FP17FFLiRenderTextureUibT3P19FFLiBufferAllocator(
    FFLiRenderTexture* texture, uint32_t resolution, bool mipmap, bool useMipmap,
    FFLiBufferAllocator* allocator)
{
    const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
    const uint32_t mipCount = __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(width, resolution, mipmap);
    const uint32_t format = __N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureFormat(
        useMipmap, FFLiUseOffScreenSrgbFetch__Fv());
    FFLiInitRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P19FFLiBufferAllocator(
        texture, width, resolution, format, mipCount, allocator);
}

int FFLiInitTempObjectFacelineTexture__FP29FFLiFacelineTextureTempObjectP17FFLiRenderTexturePC12FFLiCharInfoUibN25P18FFLiResourceLoaderP19FFLiBufferAllocatorP23FFLiRenderTextureBuffer(
    FFLiFacelineTextureTempObject* object, FFLiRenderTexture* renderTexture,
    const FFLiCharInfo* info, uint32_t resolution, bool mipmap, bool useMipmap,
    bool useUB, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator,
    FFLiRenderTextureBuffer* buffer)
{
    memset(object, 0, sizeof(*object));
    int result = FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
        &object->faceline.texture, 4, static_cast<uint32_t>(FFLiReadCharInfoValue(info, 0xc)), loader, allocator);
    if (result == 0)
        result = FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
            &object->make.texture, 5, static_cast<uint32_t>(FFLiReadCharInfoValue(info, 0x10)), loader, allocator);
    const int32_t beardType = FFLiReadCharInfoValue(info, 0x7c);
    if (result == 0 && beardType >= 4)
        result = FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
            &object->beard.texture, 0, static_cast<uint32_t>(beardType - 3), loader, allocator);
    if (result == 0)
    {
        __N_23_FFLiFacelineTexture_cpp_cff65387::InitDrawParamWithoutModulate(&object->faceline.drawParam, resolution, allocator);
        __N_23_FFLiFacelineTexture_cpp_cff65387::InitDrawParamWithoutModulate(&object->make.drawParam, resolution, allocator);
        __N_23_FFLiFacelineTexture_cpp_cff65387::InitDrawParamWithoutModulate(&object->beard.drawParam, resolution, allocator);
        FFLiInitModulateFaceLine__FP16FFLModulateParamRC11_GX2Texture(&object->faceline.drawParam.modulate, *object->faceline.texture);
        FFLiInitModulateFaceMake__FP16FFLModulateParamRC11_GX2Texture(&object->make.drawParam.modulate, *object->make.texture);
        if (beardType >= 4)
            FFLiInitModulateFaceBeard__FP16FFLModulateParamiRC11_GX2Texture(&object->beard.drawParam.modulate, FFLiReadCharInfoValue(info, 0x80), *object->beard.texture);
        if (useMipmap)
        {
            const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
            const uint32_t levels = __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(width, resolution, mipmap);
            object->renderTexture = reinterpret_cast<FFLiRenderTexture*>(allocator->Allocate(0x9c, 4));
            FFLiInitByBufferRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P23FFLiRenderTextureBuffer(
                object->renderTexture, width, resolution, 0x1a, levels, buffer);
            object->compressorParam = FFLiCompressorParam::Create(levels, useUB, allocator);
        }
    }
    (void)renderTexture;
    return result;
}

void FFLiRenderFacelineTexture__FP17FFLiRenderTexturePC12FFLiCharInfoUiP29FFLiFacelineTextureTempObjectPC18FFLiShaderCallbackP15FFLiCopySurfaceP50FFLiCompressorImpl__tm__24_21FFLiCompressorDescBC1(
    FFLiRenderTexture* texture, const FFLiCharInfo* info, uint32_t resolution,
    FFLiFacelineTextureTempObject* object, const FFLiShaderCallback* callback,
    FFLiCopySurface* copySurface, FFLiCompressorImpl<FFLiCompressorDescBC1>* compressor)
{
    (void)resolution;

    const int32_t beardType = FFLiReadCharInfoValue(info, 0x7c);

    __N_23_FFLiFacelineTexture_cpp_cff65387::InvalidateDrawParam(&object->faceline.drawParam);
    __N_23_FFLiFacelineTexture_cpp_cff65387::InvalidateDrawParam(&object->make.drawParam);
    __N_23_FFLiFacelineTexture_cpp_cff65387::InvalidateDrawParam(&object->beard.drawParam);
    FFLiInvalidateTexture__FP11_GX2Texture(object->faceline.texture);
    FFLiInvalidateTexture__FP11_GX2Texture(object->make.texture);
    if (beardType > 3) {
        FFLiInvalidateTexture__FP11_GX2Texture(object->beard.texture);
    }

    FFLiRenderTexture* const sharedRenderTexture = object->renderTexture;
    const FFLColor* clearColor = FFLiGetSrgbFetchFacelineColor__Fi(FFLiReadCharInfoValue(info, 8));

    const FFLiShaderCallback& shader = *callback;

    GX2SetAlphaTest(0, 7, 0.0f);
    GX2SetColorControl(0xcc, 0xff, 0, 1);
    GX2SetDepthOnlyControl(0, 0, 1);
    GX2SetCullOnlyControl(1, 0, 0);
    GX2SetBlendControl(0, 4, 5, 0, 1, 1, 1, 3);

    FFLiRenderTexture* target;
    if (sharedRenderTexture == 0) {
        target = texture;
    } else {
        target = sharedRenderTexture;
    }
    FFLiInvalidateRenderTexture__FP17FFLiRenderTexture(target);
    FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
        target, clearColor, 0, 0, 0x1a, &shader);
    shader.CallDraw(object->make.drawParam);
    shader.CallDraw(object->faceline.drawParam);

    if (object->beard.texture != 0) {
        shader.CallDraw(object->beard.drawParam);
    }

    if (target->texture.surface.numMips > 1) {
        copySurface->Begin();
        for (uint32_t level = 0; level + 1 < target->texture.surface.numMips; ++level) {
            copySurface->Execute(&target->texture.surface, level + 1, &target->texture.surface, level);
        }
    }

    shader.CallSetContextState();
    FFLiFlushRenderTexture__FP17FFLiRenderTexture(target);

    if (sharedRenderTexture != 0) {
        FFLiInvalidateTexture__FP11_GX2Texture(&texture->texture);
        object->compressorParam->SetTexture(&sharedRenderTexture->texture);
        compressor->CompressImpl(&texture->texture, &sharedRenderTexture->texture, object->compressorParam);
        FFLiFlushRenderTexture__FP17FFLiRenderTexture(texture);
        shader.CallSetContextState();
    }
}

unsigned int FFLiGetIconBufferSize__FPC14FFLIconSetting(const FFLIconSetting* setting)
{
    return FFLGetIconBufferSize(setting);
}

unsigned int FFLiMakeIconEx__FP11_GX2TexturePC13FFLiCharModel13FFLExpressionPC14FFLIconSettingPvT5PC17FFLShaderCallback(
    _GX2Texture* texture, const FFLiCharModel* model, FFLExpression expression,
    const FFLIconSetting* setting, void* buffer, void* temporaryBuffer,
    const FFLShaderCallback* callback)
{
    FFLCharModel wrapper = {};
    wrapper.model = *model;
    return FFLMakeIconExWithCallback(texture, &wrapper, expression, setting,
                                      buffer, temporaryBuffer, callback, true);
}

int FFLiInitRes__FPvPC11FFLInitDesc(void* work, const FFLInitDesc* initDesc)
{
    return __N_15_FFLiManager_cpp_28866420::InitResImpl(work, initDesc, 0);
}

void FFLiInitResGPUStep__Fv(void)
{
    FFLInitResGPUStep();
}

int FFLiGetWorkSize__FPC11FFLInitDesc(const FFLInitDesc* initDesc)
{
    return FFLiManager::GetBufferSize(initDesc);
}

int FFLiExit__Fv(void)
{
    return FFLExit();
}

int FFLiGetBufferSizeMaskTextures__FUiT1bT3(uint32_t expressionMask,
                                             uint32_t resolution, bool mipmap,
                                             bool compressed)
{
    const int count = __N_20_FFLiMaskTextures_cpp_feacafbb::ExpressionFlagCount(expressionMask);
    const uint32_t mipCount = mipmap ? FFLiGetMipMapNum(resolution, resolution) : 1;
    const uint32_t size = __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureBufferSize(
        resolution, mipCount, compressed);
    return static_cast<int>(FFLiRoundUp(size * count, 4)) +
           static_cast<int>(FFLiRoundUp(0x9c, 4)) * count;
}

int FFLiGetTempBufferSizeMaskTextures__FUiT1bT3P19FFLiResourceManager15FFLResourceType(
    uint32_t expressionMask, uint32_t resolution, bool mipmap, bool useMipmap,
    FFLiResourceManager* resourceManager, FFLResourceType resourceType)
{
    (void)resourceManager; (void)resourceType;
    return FFLiGetBufferSizeMaskTextures__FUiT1bT3(expressionMask, resolution,
                                                     mipmap, useMipmap) + 0x100;
}

int FFLiGetCompressBufferSizeMaskTexture__FUib(uint32_t resolution, bool mipmap)
{
    const uint32_t mipCount = mipmap ? FFLiGetMipMapNum(resolution, resolution) : 1;
    return static_cast<int>(__N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureBufferSize(
        resolution, mipCount, false));
}

unsigned int FFLiInitMaskTextures__FP16FFLiMaskTexturesUiT2bT4P19FFLiBufferAllocator(
    FFLiMaskTextures* textures, uint32_t expressionMask, uint32_t resolution,
    bool mipmap, bool useMipmap, FFLiBufferAllocator* allocator)
{
    const uint32_t mipCount = mipmap ? FFLiGetMipMapNum(resolution, resolution) : 1;
    uint32_t first = 19;
    for (uint32_t i = 0; i < 19; ++i)
    {
        textures->textures[i] = 0;
        if ((expressionMask & (1u << i)) == 0) continue;
        if (first == 19) first = i;
        textures->textures[i] = __N_20_FFLiMaskTextures_cpp_feacafbb::FFLiRenderTextureAllocate(allocator);
        const uint32_t format = __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureFormat(
            useMipmap, FFLiUseOffScreenSrgbFetch__Fv());
        FFLiInitRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P19FFLiBufferAllocator(
            textures->textures[i], resolution, resolution, format, mipCount, allocator);
    }
    return first;
}

int FFLiInitTempObjectMaskTextures__FP26FFLiMaskTexturesTempObjectPC16FFLiMaskTexturesPC12FFLiCharInfoUiT4bN26P18FFLiResourceLoaderP19FFLiBufferAllocatorP23FFLiRenderTextureBuffer(
    FFLiMaskTexturesTempObject* object, const FFLiMaskTextures* textures,
    const FFLiCharInfo* info, uint32_t expressionMask, uint32_t resolution,
    bool mipmap, bool useMipmap, bool useUB, FFLiResourceLoader* loader,
    FFLiBufferAllocator* allocator, FFLiRenderTextureBuffer* buffer)
{
    object->renderTexture = 0;
    for (uint32_t i = 0; i < 19; ++i) {
        object->rawMasks[i] = 0;
        object->compressorParams[i] = 0;
    }
    (void)textures; (void)info; (void)expressionMask; (void)resolution;
    (void)mipmap; (void)useMipmap; (void)useUB; (void)loader; (void)allocator; (void)buffer;
    return 0;
}

void FFLiRenderMaskTextures__FP16FFLiMaskTexturesP26FFLiMaskTexturesTempObjectPC18FFLiShaderCallbackP15FFLiCopySurfaceP50FFLiCompressorImpl__tm__24_21FFLiCompressorDescBC3(
    FFLiMaskTextures* textures, FFLiMaskTexturesTempObject* object,
    const FFLiShaderCallback* callback, FFLiCopySurface* copySurface,
    FFLiCompressorImpl<FFLiCompressorDescBC3>* compressor)
{
    const FFLColor clearColor = { 0.0f, 0.0f, 0.0f, 0.0f };
    const bool sharedRenderTexture = object->renderTexture == 0;

    const FFLiShaderCallback& shader = *callback;

    FFLiInvalidatePartsTextures__FP17FFLiPartsTextures(&object->partsTextures);

    for (uint32_t i = 0; i < 19; ++i) {
        if (textures->textures[i] == 0 || object->rawMasks[i] == 0) continue;

        FFLiInvalidateRawMask__FP20FFLiRawMaskDrawParam(object->rawMasks[i]);

        FFLiRenderTexture* target = sharedRenderTexture ? textures->textures[i] : object->renderTexture;
        FFLiInvalidateRenderTexture__FP17FFLiRenderTexture(target);
        FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
            target, &clearColor, 0, 0, 0x1a, &shader);
        FFLiDrawRawMask__FPC20FFLiRawMaskDrawParamPC18FFLiShaderCallback(object->rawMasks[i], &shader);

        if (target->texture.surface.numMips > 1) {
            copySurface->Begin();
            for (uint32_t level = 0; level + 1 < target->texture.surface.numMips; ++level) {
                copySurface->Execute(&target->texture.surface, level + 1, &target->texture.surface, level);
            }
        }

        shader.CallSetContextState();
        FFLiFlushRenderTexture__FP17FFLiRenderTexture(target);

        if (!sharedRenderTexture) {
            FFLiRenderTexture* destination = textures->textures[i];
            FFLiCompressorParam* compressorParam = object->compressorParams[i];
            FFLiInvalidateTexture__FP11_GX2Texture(&destination->texture);
            compressorParam->SetTexture(&target->texture);
            compressor->CompressImpl(&destination->texture, &target->texture, compressorParam);
            FFLiFlushRenderTexture__FP17FFLiRenderTexture(destination);
            shader.CallSetContextState();
        }
    }
}

struct FFLiDecodedMiiInfo
{
    uint32_t version;
    uint32_t facelineType;
    uint32_t facelineColor;
    uint32_t facelineWrinkle;
    uint32_t facelineMakeup;
    uint32_t hairType;
    uint32_t hairColor;
    uint32_t hairFlip;
    uint32_t eyeType;
    uint32_t eyeColor;
    uint32_t eyeScale;
    uint32_t eyeAspect;
    uint32_t eyeRotate;
    uint32_t eyeX;
    uint32_t eyeY;
    uint32_t eyebrowType;
    uint32_t eyebrowColor;
    uint32_t eyebrowScale;
    uint32_t eyebrowAspect;
    uint32_t eyebrowRotate;
    uint32_t eyebrowX;
    uint32_t eyebrowY;
    uint32_t noseType;
    uint32_t noseScale;
    uint32_t noseY;
    uint32_t mouthType;
    uint32_t mouthColor;
    uint32_t mouthScale;
    uint32_t mouthAspect;
    uint32_t mouthY;
    uint32_t mustacheType;
    uint32_t beardType;
    uint32_t beardColor;
    uint32_t beardScale;
    uint32_t beardY;
    uint32_t glassesType;
    uint32_t glassesColor;
    uint32_t glassesScale;
    uint32_t glassesY;
    uint32_t moleEnabled;
    uint32_t moleScale;
    uint32_t moleX;
    uint32_t moleY;
    uint32_t height;
    uint32_t build;
    uint16_t name[11];
    uint16_t creatorName[11];
    uint32_t gender;
    uint32_t birthMonth;
    uint32_t birthDay;
    uint32_t favoriteColor;
    uint8_t flag0;
    uint8_t flag1;
    uint8_t flag2;
    uint8_t flag3;
    uint32_t metadataWordsAfterFlags[5];
    uint8_t createID[10];
    uint16_t createIDTerminator;
    uint32_t metadataAfterCreateID;
    uint8_t authorID[8];
};

static_assert(sizeof(FFLiDecodedMiiInfo) == 288, "FFLiDecodedMiiInfo size");
static_assert(offsetof(FFLiDecodedMiiInfo, name) == 0xb4, "FFLiDecodedMiiInfo name offset");
static_assert(offsetof(FFLiDecodedMiiInfo, creatorName) == 0xca, "FFLiDecodedMiiInfo creator offset");
static_assert(offsetof(FFLiDecodedMiiInfo, createID) == 0x108, "FFLiDecodedMiiInfo create ID offset");
static_assert(offsetof(FFLiDecodedMiiInfo, authorID) == 0x118, "FFLiDecodedMiiInfo author offset");


unsigned int FFLiMiiDataCoreRFL2CharInfo__FP12FFLiCharInfoRC18FFLiMiiDataCoreRFLPCUsb(
    FFLiCharInfo* destination, const FFLiMiiDataCoreRFL& source,
    const uint16_t* replacementName, bool replaceName)
{
    const uint16_t* value = source.data;
    FFLiDecodedMiiInfo decoded = {};
    const uint32_t face = value[0x10] >> 6 & 0xf;
    const uint32_t* faceLineAndMake = __N_15_FFLiMiiData_cpp_2f9c0036::RflToFaceLineAndMake(face);
    decoded.version = 3;
    decoded.facelineType = value[0x10] >> 13;
    decoded.facelineColor = value[0x10] >> 10 & 7;
    decoded.facelineWrinkle = faceLineAndMake[0];
    decoded.facelineMakeup = faceLineAndMake[1];
    decoded.hairType = value[0x11] >> 9;
    decoded.hairColor = value[0x11] >> 6 & 7;
    decoded.hairFlip = value[0x11] >> 5 & 1;
    decoded.eyeType = value[0x14] >> 10;
    decoded.eyeColor = value[0x15] >> 13;
    decoded.eyeScale = value[0x15] >> 9 & 0xf;
    decoded.eyeAspect = 3;
    decoded.eyeRotate = value[0x14] >> 5 & 0x1f;
    decoded.eyeX = value[0x15] >> 5 & 0xf;
    decoded.eyeY = value[0x14] & 0x1f;
    decoded.eyebrowType = value[0x12] >> 11;
    decoded.eyebrowColor = value[0x13] >> 13;
    decoded.eyebrowScale = value[0x13] >> 9 & 0xf;
    decoded.eyebrowAspect = 3;
    decoded.eyebrowRotate = value[0x12] >> 6 & 0x1f;
    decoded.eyebrowX = value[0x13] & 0xf;
    decoded.eyebrowY = value[0x13] >> 4 & 0x1f;
    decoded.noseType = value[0x16] >> 12;
    decoded.noseScale = value[0x16] >> 8 & 0xf;
    decoded.noseY = value[0x16] >> 3 & 0x1f;
    decoded.mouthType = value[0x17] >> 11;
    decoded.mouthColor = value[0x17] >> 9 & 3;
    decoded.mouthScale = value[0x17] >> 5 & 0xf;
    decoded.mouthAspect = 3;
    decoded.mouthY = value[0x17] & 0x1f;
    decoded.mustacheType = value[0x19] >> 14;
    decoded.beardType = value[0x19] >> 12 & 3;
    decoded.beardColor = value[0x19] >> 9 & 7;
    decoded.beardScale = value[0x19] >> 5 & 0xf;
    decoded.beardY = value[0x19] & 0x1f;
    decoded.glassesType = value[0x18] >> 12;
    decoded.glassesColor = value[0x18] >> 9 & 7;
    decoded.glassesScale = value[0x18] >> 5 & 0xf;
    decoded.glassesY = value[0x18] & 0x1f;
    decoded.moleEnabled = value[0x1a] >> 15;
    decoded.moleScale = value[0x1a] >> 11 & 0xf;
    decoded.moleX = value[0x1a] >> 1 & 0x1f;
    decoded.moleY = value[0x1a] >> 6 & 0x1f;
    decoded.height = value[0x0b] >> 8;
    decoded.build = value[0x0b] & 0xff;
    decoded.gender = value[0] >> 14 & 1;
    decoded.birthMonth = value[0] >> 10 & 0xf;
    decoded.birthDay = value[0] >> 5 & 0x1f;
    decoded.favoriteColor = value[0] >> 1 & 0xf;
    decoded.flag0 = value[0] & 1;
    decoded.flag1 = 0;
    decoded.flag2 = 0;
    decoded.flag3 = value[0x10] >> 2 & 1;
    decoded.metadataWordsAfterFlags[0] = 0;
    decoded.metadataWordsAfterFlags[1] = 0;
    decoded.metadataWordsAfterFlags[2] = 0;
    decoded.metadataWordsAfterFlags[3] = 0;
    decoded.metadataWordsAfterFlags[4] = 0;
    decoded.metadataAfterCreateID = 0;
    memcpy(decoded.name, value + 1, 20);
    decoded.name[10] = 0;
    if (replacementName != 0) memcpy(decoded.creatorName, replacementName, 22);
    decoded.creatorName[10] = 0;
    memset(decoded.createID, 0, sizeof(decoded.createID));
    decoded.createIDTerminator = 0;
    memset(decoded.authorID, 0, sizeof(decoded.authorID));
    decoded.metadataWordsAfterFlags[4] = FFLiIsNTRMiiID(reinterpret_cast<const FFLiMiiID*>(decoded.createID)) ? 2u : 1u;
    unsigned int nameResult = 0;
    unsigned int creatorResult = 0;
    if (replaceName)
    {
        nameResult = __N_15_FFLiMiiData_cpp_2f9c0036::ReplaceNameFromRFL(decoded.name, 10, 5);
        creatorResult = __N_15_FFLiMiiData_cpp_2f9c0036::ReplaceNameFromRFL(decoded.creatorName, 10, 0);
    }
    memcpy(destination->data, &decoded, sizeof(decoded));
    return (nameResult == 0 && creatorResult == 0) ? 0u : 1u;
}

void FFLiCharInfo2MiiDataCore__FP15FFLiMiiDataCoreRC12FFLiCharInfob(
    FFLiMiiDataCore* destination, const FFLiCharInfo& source, bool suppressBirthday)
{
    FFLiDecodedMiiInfo value;
    memcpy(&value, source.data, sizeof(value));
    uint32_t words[18];
    memcpy(words, destination, sizeof(words));
    const uint8_t* bytes = source.data;
    words[0] = static_cast<uint32_t>(value.version & 0xffu) |
               (uint32_t(value.flag1 != 0) << 8) | (uint32_t(value.flag2 != 0) << 9) |
               ((value.metadataWordsAfterFlags[0] & 3u) << 10) |
               ((value.metadataWordsAfterFlags[1] & 3u) << 12) |
               ((value.metadataWordsAfterFlags[2] & 0xfu) << 16) |
               ((value.metadataWordsAfterFlags[3] & 0xfu) << 20) |
               ((value.metadataAfterCreateID & 0xfu) << 24) |
               ((value.metadataWordsAfterFlags[4] & 7u) << 28);
    words[6] = uint32_t(value.gender & 1u) << 16 | uint32_t(suppressBirthday ? 0 : (value.birthMonth & 0xfu)) << 17 |
               uint32_t(suppressBirthday ? 0 : (value.birthDay & 0x1fu)) << 21 | uint32_t(value.favoriteColor & 0xfu) << 26 |
               uint32_t((value.flag0 & 1u) != 0) << 30;
    words[12] = (value.hairType & 0xffu) | ((value.hairColor & 7u) << 8) |
                ((value.hairFlip & 1u) << 11) | (uint32_t(value.flag3 != 0) << 16) | ((value.facelineType & 0xfu) << 17) |
                ((value.facelineColor & 7u) << 21) | ((value.facelineWrinkle & 0xfu) << 24) |
                ((value.facelineMakeup & 0xfu) << 28);
    words[13] = (value.eyeRotate & 0x1fu) | ((value.eyeX & 0xfu) << 5) |
                ((value.eyeY & 0x1fu) << 9) | ((value.eyeType & 0x3fu) << 16) |
                ((value.eyeColor & 7u) << 22) | ((value.eyeScale & 0xfu) << 25) | ((value.eyeAspect & 7u) << 29);
    words[14] = (value.eyebrowRotate & 0x1fu) | ((value.eyebrowX & 0xfu) << 5) |
                ((value.eyebrowY & 0x1fu) << 9) | ((value.eyebrowType & 0x1fu) << 16) |
                ((value.eyebrowColor & 7u) << 21) | ((value.eyebrowScale & 0xfu) << 24) | ((value.eyebrowAspect & 7u) << 28);
    words[15] = (value.mouthType & 0x3fu) | ((value.mouthColor & 7u) << 6) |
                ((value.mouthScale & 0xfu) << 9) | ((value.mouthAspect & 7u) << 13) |
                ((value.noseType & 0x1fu) << 16) | ((value.noseScale & 0xfu) << 21) | ((value.noseY & 0x1fu) << 25);
    words[16] = (value.beardType & 7u) | ((value.beardColor & 7u) << 3) |
                ((value.beardScale & 0xfu) << 6) | ((value.beardY & 0x1fu) << 10) |
                ((value.mouthY & 0x1fu) << 16) | ((value.mustacheType & 7u) << 21);
    words[17] = (value.moleEnabled & 1u) | ((value.moleScale & 0xfu) << 1) |
                ((value.moleX & 0x1fu) << 5) | ((value.moleY & 0x1fu) << 10) |
                ((value.glassesType & 0xfu) << 16) | ((value.glassesColor & 7u) << 20) |
                ((value.glassesScale & 0xfu) << 23) | ((value.glassesY & 0x1fu) << 27);
    reinterpret_cast<uint8_t*>(words)[0x16] = 0;
    reinterpret_cast<uint8_t*>(words)[0x17] = 0;
    reinterpret_cast<uint8_t*>(words)[0x2e] = static_cast<uint8_t>(value.height);
    reinterpret_cast<uint8_t*>(words)[0x2f] = static_cast<uint8_t>(value.build);
    memcpy(destination, words, sizeof(words));
    memcpy(reinterpret_cast<uint8_t*>(destination) + 4, value.authorID, 8);
    memcpy(reinterpret_cast<uint8_t*>(destination) + 12, value.createID, 10);
    memcpy(reinterpret_cast<uint8_t*>(destination) + 26, value.name, 20);
}

int FFLiMiiDataCoreRFL2MiiDataCore__FP15FFLiMiiDataCoreRC18FFLiMiiDataCoreRFLb(
    FFLiMiiDataCore* destination, const FFLiMiiDataCoreRFL& source, bool replaceName)
{
    FFLiCharInfo info = {};
    const unsigned int result = FFLiMiiDataCoreRFL2CharInfo__FP12FFLiCharInfoRC18FFLiMiiDataCoreRFLPCUsb(
        &info, source, 0, replaceName);
    FFLiCharInfo2MiiDataCore__FP15FFLiMiiDataCoreRC12FFLiCharInfob(destination, info, false);
    return static_cast<int>(result);
}

void FFLiClearCreatorNameFromOfficial__FP19FFLiMiiDataOfficial(
    FFLiMiiDataOfficial* data)
{
    memset(data->authorId.data, 0, sizeof(data->authorId.data));
}

void FFLiMiiDataCore2CharInfo__FP12FFLiCharInfoRC15FFLiMiiDataCorePCUsb(
    FFLiCharInfo* destination, const FFLiMiiDataCore& source,
    const uint16_t* creatorName, bool suppressBirthday)
{
    uint32_t value[18];
    memcpy(value, &source, sizeof(value));
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(&source);
    FFLiDecodedMiiInfo decoded = {};
    decoded.version = static_cast<uint8_t>(value[0]);
    decoded.facelineType = (value[12] >> 17) & 0xf;
    decoded.facelineColor = (value[12] >> 21) & 7;
    decoded.facelineWrinkle = (value[12] >> 24) & 0xf;
    decoded.facelineMakeup = value[12] >> 28;
    decoded.hairType = bytes[0x33];
    decoded.hairColor = (value[12] >> 8) & 7;
    decoded.hairFlip = (value[12] >> 11) & 1;
    decoded.eyeType = (value[13] >> 16) & 0x3f;
    decoded.eyeColor = (value[13] >> 22) & 7;
    decoded.eyeScale = (value[13] >> 25) & 0xf;
    decoded.eyeAspect = value[13] >> 29;
    decoded.eyeRotate = value[13] & 0x1f;
    decoded.eyeX = (value[13] >> 5) & 0xf;
    decoded.eyeY = (value[13] >> 9) & 0x1f;
    decoded.eyebrowType = (value[14] >> 16) & 0x1f;
    decoded.eyebrowColor = (value[14] >> 21) & 7;
    decoded.eyebrowScale = (value[14] >> 24) & 0xf;
    decoded.eyebrowAspect = (value[14] >> 28) & 7;
    decoded.eyebrowRotate = value[14] & 0x1f;
    decoded.eyebrowX = (value[14] >> 5) & 0xf;
    decoded.eyebrowY = (value[14] >> 9) & 0x1f;
    decoded.noseType = (value[15] >> 16) & 0x1f;
    decoded.noseScale = (value[15] >> 21) & 0xf;
    decoded.noseY = (value[15] >> 25) & 0x1f;
    decoded.mouthType = value[15] & 0x3f;
    decoded.mouthColor = (value[15] >> 6) & 7;
    decoded.mouthScale = (value[15] >> 9) & 0xf;
    decoded.mouthAspect = (value[15] >> 13) & 7;
    decoded.mouthY = (value[16] >> 16) & 0x1f;
    decoded.mustacheType = (value[16] >> 21) & 7;
    decoded.beardType = value[16] & 7;
    decoded.beardColor = (value[16] >> 3) & 7;
    decoded.beardScale = (value[16] >> 6) & 0xf;
    decoded.beardY = (value[16] >> 10) & 0x1f;
    decoded.glassesType = (value[17] >> 16) & 0xf;
    decoded.glassesColor = (value[17] >> 20) & 7;
    decoded.glassesScale = (value[17] >> 23) & 0xf;
    decoded.glassesY = value[17] >> 27;
    decoded.moleEnabled = value[17] & 1;
    decoded.moleScale = (value[17] >> 1) & 0xf;
    decoded.moleX = (value[17] >> 5) & 0x1f;
    decoded.moleY = (value[17] >> 10) & 0x1f;
    decoded.height = bytes[0x2e];
    decoded.build = bytes[0x2f];
    decoded.gender = (value[6] >> 16) & 1;
    decoded.birthMonth = suppressBirthday ? 0 : ((value[6] >> 17) & 0xf);
    decoded.birthDay = suppressBirthday ? 0 : ((value[6] >> 21) & 0x1f);
    decoded.favoriteColor = (value[6] >> 26) & 0xf;
    decoded.flag0 = static_cast<uint8_t>((value[6] >> 30) & 1);
    decoded.flag1 = static_cast<uint8_t>((value[0] >> 8) & 1);
    decoded.flag2 = static_cast<uint8_t>((value[0] >> 9) & 1);
    decoded.flag3 = static_cast<uint8_t>((value[12] >> 16) & 1);
    decoded.metadataWordsAfterFlags[0] = (value[0] >> 10) & 3;
    decoded.metadataWordsAfterFlags[1] = (value[0] >> 12) & 3;
    decoded.metadataWordsAfterFlags[2] = (value[0] >> 16) & 0xf;
    decoded.metadataWordsAfterFlags[3] = (value[0] >> 20) & 0xf;
    decoded.metadataWordsAfterFlags[4] = (value[0] >> 28) & 7;
    decoded.metadataAfterCreateID = (value[0] >> 24) & 0xf;
    memcpy(decoded.authorID, bytes + 4, sizeof(decoded.authorID));
    memcpy(decoded.name, bytes + 0x1a, sizeof(decoded.name));
    decoded.name[10] = 0;
    memcpy(decoded.createID, bytes + 12, sizeof(decoded.createID));
    decoded.createIDTerminator = 0;
    if (creatorName != 0)
    {
        memcpy(decoded.creatorName, creatorName, sizeof(decoded.creatorName));
        decoded.creatorName[10] = 0;
    }
    else
        memset(decoded.creatorName, 0, sizeof(decoded.creatorName));
    memcpy(destination->data, &decoded, sizeof(decoded));
}

void FFLiMiiDataOfficial2CharInfo__FP12FFLiCharInfoRC19FFLiMiiDataOfficial(
    FFLiCharInfo* destination, const FFLiMiiDataOfficial& source)
{
    FFLiMiiDataCore2CharInfo__FP12FFLiCharInfoRC15FFLiMiiDataCorePCUsb(
        destination, source, source.dataWords, false);
}

int FFLiStoreDataCFLToCharInfo__FP12FFLiCharInfoRC16FFLiStoreDataCFL(
    FFLiCharInfo* destination, const FFLiStoreDataCFL& source)
{
    if (!source.IsValidCRC()) return 3;
    FFLiStoreDataCFL converted;
    memcpy(&converted, &source, 0x60);
    converted.SwapEndian();
    FFLiMiiDataOfficial2CharInfo__FP12FFLiCharInfoRC19FFLiMiiDataOfficial(
        destination, converted);
    return 0;
}

uint32_t FFLiCharInfoAndTypeToEyeIndex__FPC12FFLiCharInfo18FFLiEyeTextureType(
    const FFLiCharInfo* info, uint32_t type)
{
    static const uint8_t indices[6] = {0x3d, 0x1a, 0x2f, 0x00, 0x0a, 0x0c};
    return type == 0 ? info->data[0x20] : (type < 6 ? indices[type] : 0);
}

uint32_t FFLiCharInfoAndTypeToMouthIndex__FPC12FFLiCharInfo20FFLiMouthTextureType(
    const FFLiCharInfo* info, uint32_t type)
{
    static const uint8_t indices[5] = {0x0a, 0x0c, 0x24, 0x13, 0x00};
    return type == 0 ? info->data[0x64] : (type < 5 ? indices[type] : 0);
}

void FFLiLoadPartsTextures__FP17FFLiPartsTexturesPC12FFLiCharInfoUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
    FFLiPartsTextures* textures, const FFLiCharInfo* info, uint32_t mask,
    FFLiResourceLoader* loader, FFLiBufferAllocator* allocator)
{
    memset(textures, 0, sizeof(*textures));
    for (uint32_t i = 0; i < 6; ++i)
        if ((mask & (1u << i)) != 0)
            FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(&textures->textures[i], 2,
                FFLiCharInfoAndTypeToEyeIndex__FPC12FFLiCharInfo18FFLiEyeTextureType(info, i), loader, allocator);
    for (uint32_t i = 0; i < 5; ++i)
        if ((mask & (1u << (i + 8))) != 0)
            FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(&textures->textures[i + 8], 8,
                FFLiCharInfoAndTypeToMouthIndex__FPC12FFLiCharInfo20FFLiMouthTextureType(info, i), loader, allocator);
}

void FFLiInvalidatePartsTextures__FP17FFLiPartsTextures(FFLiPartsTextures* textures)
{
    for (uint32_t i = 0; i < 14; ++i) FFLiInvalidateTexture__FP11_GX2Texture(textures->textures[i]);
}

const uint32_t* FFLiGetEyeMouthTypeElement__F13FFLExpression(uint32_t expression)
{
    static const uint32_t elements[4][3] = {{0, 0, 0}, {1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
    return elements[expression < 4 ? expression : 0];
}

uint64_t FFLiGetMiiStudioTitleID__Fv(void)
{
    return _SYSGetSystemApplicationTitleId(SYSTEM_APP_ID_MII_MAKER);
}

int FFLiGetResourcePath__FPcUiPCc(char* path, uint32_t size, const char* suffix)
{
    return SAVEGetSharedDataTitlePath(0x0005001b10056000ULL, suffix, path, size);
}

int FFLiGetDatabasePath__FPcUiULPCc(char* path, uint32_t size, unsigned long long titleId,
                                    const char* suffix)
{
    return SAVEGetSharedSaveDataPath(titleId, suffix, path, size);
}

void FFLiInitDrawParamRawMask__FP20FFLiRawMaskDrawParamPC12FFLiCharInfoiN23PC22FFLiRawMaskTextureDescP19FFLiBufferAllocator(
    void* drawParam, const FFLiCharInfo* info, int rotation, int eyeAdjust,
    int mouthAdjust, const void* textureDesc, FFLiBufferAllocator* allocator)
{
    __N_15_FFLiRawMask_cpp_1637d14a::RawMasks rawMasks;
    __N_15_FFLiRawMask_cpp_1637d14a::CalcRawMask(&rawMasks, info, rotation, eyeAdjust, mouthAdjust);

    _Mat44 matrix;
    const float extent = static_cast<float>(rotation);
    C_MTXOrtho(matrix.value, 0.0f, extent, 0.0f, extent, -200.0f, 200.0f);

    uint8_t* base = static_cast<uint8_t*>(drawParam);
    const void* const* textures = static_cast<const void* const*>(textureDesc);

    FFLiInitModulateMustache__FP16FFLModulateParamiRC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x244),
        FFLiReadCharInfoValue(info, 0x80), *static_cast<const _GX2Texture*>(textures[5]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0x208, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 30), &matrix, allocator);

    FFLiInitModulateMustache__FP16FFLModulateParamiRC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x2ac),
        FFLiReadCharInfoValue(info, 0x80), *static_cast<const _GX2Texture*>(textures[6]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0x270, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 36), &matrix, allocator);

    FFLiInitModulateMouth__FP16FFLModulateParamiRC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x1dc),
        FFLiReadCharInfoValue(info, 0x68), *static_cast<const _GX2Texture*>(textures[4]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0x1a0, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 24), &matrix, allocator);

    FFLiInitModulateEyebrow__FP16FFLModulateParamiRC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x10c),
        FFLiReadCharInfoValue(info, 0x40), *static_cast<const _GX2Texture*>(textures[2]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0xd0, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 12), &matrix, allocator);

    FFLiInitModulateEyebrow__FP16FFLModulateParamiRC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x174),
        FFLiReadCharInfoValue(info, 0x40), *static_cast<const _GX2Texture*>(textures[3]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0x138, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 18), &matrix, allocator);

    FFLiInitModulateEye__FP16FFLModulateParamiT2RC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x3c),
        FFLiReadCharInfoValue(info, 0x24), FFLiReadCharInfoValue(info, 0x20),
        *static_cast<const _GX2Texture*>(textures[0]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 0), &matrix, allocator);

    FFLiInitModulateEye__FP16FFLModulateParamiT2RC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0xa4),
        FFLiReadCharInfoValue(info, 0x24), FFLiReadCharInfoValue(info, 0x20),
        *static_cast<const _GX2Texture*>(textures[1]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0x68, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 6), &matrix, allocator);

    FFLiInitModulateMole__FP16FFLModulateParamRC11_GX2Texture(
        reinterpret_cast<FFLModulateParam*>(base + 0x314),
        *static_cast<const _GX2Texture*>(textures[7]));
    FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
        base + 0x2d8, reinterpret_cast<const FFLiRawMaskPartsDesc*>(rawMasks.values + 42), &matrix, allocator);

    FFLiInitModulateFill__FP16FFLModulateParam(reinterpret_cast<FFLModulateParam*>(base + 0x37c));
    FFLiInitDrawParamRawMaskPartsFill__FP25FFLiRawMaskPartsDrawParamP19FFLiBufferAllocator(
        base + 0x340, allocator);
}

void FFLiInvalidateRawMask__FP20FFLiRawMaskDrawParam(void* drawParam)
{
    GX2Invalidate(0x10, drawParam, 0x300);
}

void FFLiDrawRawMask__FPC20FFLiRawMaskDrawParamPC18FFLiShaderCallback(
    const void* drawParam, const FFLiShaderCallback* callback)
{
    (void)drawParam;
    callback->CallSetContextState();
}

void FFLiInitDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParamPC20FFLiRawMaskPartsDescPC6_Mat44P19FFLiBufferAllocator(
    void* drawParam, const FFLiRawMaskPartsDesc* description, const _Mat44* matrix,
    FFLiBufferAllocator* allocator)
{
    (void)drawParam; (void)description; (void)matrix; (void)allocator;
}

void FFLiInvalidateDrawParamRawMaskParts__FP25FFLiRawMaskPartsDrawParam(void* drawParam)
{
    GX2Invalidate(0x10, drawParam, sizeof(FFLPrimitiveParam) + sizeof(FFLAttributeBufferParam));
}

void FFLiInitDrawParamRawMaskPartsFill__FP25FFLiRawMaskPartsDrawParamP19FFLiBufferAllocator(
    void* drawParam, FFLiBufferAllocator* allocator)
{
    FFLAttributeBufferParam* attributes = static_cast<FFLAttributeBufferParam*>(drawParam);
    __N_20_FFLiRawMaskParts_cpp_7c9651f3::InitAttributesForFill(attributes, allocator);
}

void FFLiDrawRawMaskParts__FPC25FFLiRawMaskPartsDrawParamPC18FFLiShaderCallback(
    const void* drawParam, const FFLiShaderCallback* callback)
{
    (void)drawParam;
    callback->CallSetContextState();
}

void FFLiInitRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P19FFLiBufferAllocator(
    FFLiRenderTexture* texture, uint32_t width, uint32_t height, uint32_t format,
    uint32_t mipCount, FFLiBufferAllocator* allocator)
{
    memset(&texture->texture, 0, sizeof(texture->texture));
    GX2InitTexture(&texture->texture, width, height, 0, mipCount,
                   static_cast<_GX2SurfaceFormat>(format), GX2_SURFACE_DIM_TEXTURE_2D);
    texture->texture.surface.imagePtr = reinterpret_cast<void*>(allocator->Allocate(
        texture->texture.surface.imageSize, texture->texture.surface.alignment));
    texture->texture.surface.mipPtr = texture->texture.surface.mipSize == 0 ? 0 :
        reinterpret_cast<void*>(allocator->Allocate(texture->texture.surface.mipSize,
                                                     texture->texture.surface.alignment));
    if (texture->texture.surface.numMips > 1 && texture->texture.surface.mipPtr == 0)
        texture->texture.surface.mipPtr = static_cast<uint8_t*>(texture->texture.surface.imagePtr) +
                                          texture->texture.surface.mipOffset[0];
}

void FFLiInitByBufferRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P23FFLiRenderTextureBuffer(
    FFLiRenderTexture* texture, uint32_t width, uint32_t height, uint32_t format,
    uint32_t mipCount, FFLiRenderTextureBuffer* buffer)
{
    GX2InitTexture(&texture->texture, width, height, 0, mipCount,
                   static_cast<_GX2SurfaceFormat>(format), GX2_SURFACE_DIM_TEXTURE_2D);
    texture->texture.surface.imagePtr = buffer->buffer;
    texture->texture.surface.mipPtr = 0;
}

void FFLiSetupRenderTexture__FP11_GX2TexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
    _GX2Texture* texture, const FFLColor* clearColor, void* depthBuffer,
    uint32_t depthSize, uint32_t format, const FFLiShaderCallback* callback)
{
    (void)texture; (void)clearColor; (void)depthBuffer; (void)depthSize; (void)format;
    callback->CallSetContextState();
}

void FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
    FFLiRenderTexture* texture, const FFLColor* clearColor, void* depthBuffer,
    uint32_t depthSize, uint32_t format, const FFLiShaderCallback* callback)
{
    FFLiSetupRenderTexture__FP11_GX2TexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
        &texture->texture, clearColor, depthBuffer, depthSize, format, callback);
}

const FFLiResourcePartsInfo* FFLiGetTextureResoucePartsInfos__FPUiP25FFLiResourceTextureHeader20FFLiTexturePartsType(
    uint32_t* size, const void* header, uint32_t parts)
{
    static const uint32_t offsets[] = { 0x2c, 0x5c, 0x89c, 0xc7c, 0xdfc, 0xebc,
                                        0xf7c, 0x100c, 0x102c, 0x127c, 0x12dc };
    *size = FFLiGetTextureResourceNum__F20FFLiTexturePartsType(parts);
    if (parts >= sizeof(offsets) / sizeof(offsets[0])) return 0;
    return reinterpret_cast<const FFLiResourcePartsInfo*>(
        static_cast<const uint8_t*>(header) + offsets[parts]);
}

const FFLiResourcePartsInfo* FFLiGetShapeResoucePartsInfos__FPUiP23FFLiResourceShapeHeader18FFLiShapePartsType(
    uint32_t* size, const void* header, uint32_t parts)
{
    static const uint32_t offsets[] = { 0x30, 0x70, 0x8b0, 0x10f0, 0x11b0, 0x11c0,
                                        0x1280, 0x13a0, 0x14c0, 0x1d00, 0x2540, 0x2d80 };
    *size = FFLiGetShapeResourceNum__F18FFLiShapePartsType(parts);
    if (parts >= sizeof(offsets) / sizeof(offsets[0])) return 0;
    return reinterpret_cast<const FFLiResourcePartsInfo*>(
        static_cast<const uint8_t*>(header) + offsets[parts]);
}

uint32_t FFLiIsVaildResourceHeader__FPC18FFLiResourceHeader(
    const FFLiResourceHeader* header)
{
    return header != 0 ? header->GetResult() : 1;
}

int FFLiCanDrawShape__FPC12FFLDrawParam(const FFLDrawParam* drawParam)
{
    return drawParam != 0 && drawParam->primitive.indexCount != 0;
}

int FFLiLoadShape__FP12FFLDrawParamP14FFLBoundingBoxP13FFLiCharModel18FFLiShapePartsTypeUsP18FFLiResourceLoaderP19FFLiBufferAllocator(
    FFLDrawParam* drawParam, FFLBoundingBox* bounds, FFLiCharModel* model,
    uint32_t parts, unsigned short index, FFLiResourceLoader* loader,
    FFLiBufferAllocator* allocator)
{
    const FFLiShapePartsType shapeParts = static_cast<FFLiShapePartsType>(parts);
    uint32_t size = loader->GetShapeAlignedMaxSize(shapeParts);
    void* resource = reinterpret_cast<void*>(allocator->Allocate(size, 0x40));
    int result = static_cast<int>(loader->LoadShape(resource, &size, shapeParts, index));
    if (result != 0) return result;

    if (size == 0) {
        memset(drawParam, 0, sizeof(FFLDrawParam));
        float nanValue;
        uint32_t nanBits = 0x7fc00000u;
        memcpy(&nanValue, &nanBits, sizeof(nanValue));
        bounds->minX = bounds->minY = bounds->minZ = nanValue;
        bounds->maxX = bounds->maxY = bounds->maxZ = nanValue;
        return result;
    }

#if defined(RECON_NATIVE)
    if (!host::decodeFFLShape(resource, size, parts))
        return 3;
#endif

    for (uint32_t i = 0; i < 5; ++i) {
        FFLAttributeBuffer& attribute = drawParam->attributes.attributes[i];
        uint8_t elementType = __N_13_FFLiShape_cpp_fe662c08::GetElementType(static_cast<FFLAttributeBufferType>(i));
        attribute.buffer = const_cast<void*>(
            FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
                &attribute.size, resource, parts, elementType));
        attribute.stride = __N_13_FFLiShape_cpp_fe662c08::GetStride(
            static_cast<FFLAttributeBufferType>(i), attribute.size);
    }

    drawParam->primitive.indexBuffer = const_cast<void*>(
        FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
            &drawParam->primitive.indexCount, resource, parts, 5));
    drawParam->primitive.vertexCount = 4;
    drawParam->primitive.primitiveType = 4;

    if (parts == 8) {
        uint32_t auxSize = 0;
        const float* aux = reinterpret_cast<const float*>(
            FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
                &auxSize, resource, parts, 6));
        model->transformedParts[2].x = aux[3]; model->transformedParts[2].y = aux[4]; model->transformedParts[2].z = aux[5];
        model->transformedParts[3].x = aux[0]; model->transformedParts[3].y = aux[1]; model->transformedParts[3].z = aux[2];
        model->transformedParts[4].x = aux[9]; model->transformedParts[4].y = aux[10]; model->transformedParts[4].z = aux[11];
        model->transformedParts[5].x = aux[6]; model->transformedParts[5].y = aux[7]; model->transformedParts[5].z = aux[8];
        model->transformedParts[6].x = aux[15]; model->transformedParts[6].y = aux[16]; model->transformedParts[6].z = aux[17];
        model->transformedParts[7].x = aux[12]; model->transformedParts[7].y = aux[13]; model->transformedParts[7].z = aux[14];
    } else if (parts == 3) {
        uint32_t auxSize = 0;
        const float* aux = reinterpret_cast<const float*>(
            FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
                &auxSize, resource, parts, 7));
        model->faceOffset.x = aux[6]; model->faceOffset.y = aux[7]; model->faceOffset.z = aux[8];
        model->commonOffset.x = aux[0]; model->commonOffset.y = aux[1]; model->commonOffset.z = aux[2];
        model->transformedParts[0].x = aux[3]; model->transformedParts[0].y = aux[4]; model->transformedParts[0].z = aux[5];
        model->transformedParts[1].x = aux[0]; model->transformedParts[1].y = aux[1]; model->transformedParts[1].z = aux[2];
    }

    uint32_t boundsSize = 0;
    const void* boundsData = FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
        &boundsSize, resource, parts, 8);
    memcpy(bounds, boundsData, sizeof(FFLBoundingBox));

    return result;
}

void FFLiAdjustShape__FP12FFLDrawParamP14FFLBoundingBoxfT3PC7FFLVec3bPC14FFLiCoordinate18FFLiShapePartsTypeT6(
    FFLDrawParam* drawParam, FFLBoundingBox* bounds, float scaleX, float scaleY,
    const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate,
    uint32_t parts, bool adjust)
{
    float scaleZ = (scaleX + scaleY) * 0.5f;
    if (adjust && (parts == 6 || parts == 7) && scaleZ > 1.1f) scaleZ = 1.1f;

    __N_13_FFLiShape_cpp_fe662c08::AdjustAttribute(
        reinterpret_cast<FFLVec4*>(drawParam->attributes.attributes[0].buffer),
        drawParam->attributes.attributes[0].size >> 4,
        scaleX, scaleY, scaleZ, offset, flip, coordinate);

    __N_13_FFLiShape_cpp_fe662c08::AdjustAttributeWithoutScale(
        reinterpret_cast<FFLiSnorm10_10_10_2*>(drawParam->attributes.attributes[2].buffer),
        drawParam->attributes.attributes[2].size >> 2, flip, coordinate);

    __N_13_FFLiShape_cpp_fe662c08::AdjustAttributeWithoutScale(
        reinterpret_cast<FFLiSnorm8_8_8_8*>(drawParam->attributes.attributes[3].buffer),
        drawParam->attributes.attributes[3].size >> 2, flip, coordinate);

    uint32_t minXBits;
    memcpy(&minXBits, &bounds->minX, sizeof(minXBits));
    if ((minXBits << 1) < 0xff000001u) {
        __N_13_FFLiShape_cpp_fe662c08::AdjustAttribute(
            reinterpret_cast<FFLVec3*>(bounds), 2, scaleX, scaleY, scaleZ, offset, flip, coordinate);
    }

    for (uint32_t i = 0; i < 5; ++i) {
        FFLAttributeBuffer& attribute = drawParam->attributes.attributes[i];
        if (attribute.buffer != 0)
            __N_13_FFLiShape_cpp_fe662c08::EndianSwap(attribute.buffer, attribute.size);
    }

    if (drawParam->primitive.indexBuffer != 0) {
        __N_13_FFLiShape_cpp_fe662c08::EndianSwap(
            drawParam->primitive.indexBuffer, drawParam->primitive.indexCount << 1);
    }

    instructionSynchronize();
}

void FFLiInvalidateShape__FP12FFLDrawParam(FFLDrawParam* drawParam)
{
    if (drawParam != 0) GX2Invalidate(0x10, drawParam, sizeof(*drawParam));
}

void FFLiTempSetShaderMode__F14_GX2ShaderMode(uint32_t mode)
{
    if (mode == 2)
        GX2SetShaderModeEx(mode, 0x2c, 0x20, 0x40, 0x30, 0x4c, 0xb0);
    else
        GX2SetShaderModeEx(mode, 0x30, 0x40, 0, 0, 0xc8, 0xc0);
}

int FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
    _GX2Texture** texture, uint32_t parts, uint32_t index, FFLiResourceLoader* loader,
    FFLiBufferAllocator* allocator)
{
    static const uint32_t compMaps[] = {
        0x04040405, 0x00040405, 0x00010405, 0x04040405, 0x04040405, 0x00040405,
        0x00040405, 0x00010405, 0x00010205, 0x00010205, 0x00010203, 0x00010203,
        0x03020100, 0x00040405, 0x00040405, 0x00010405, 0x00010405, 0x04040405,
        0x04040405, 0x04040405, 0x04040405, 0x04040405, 0x00010205, 0x04040405,
        0x04040405, 0x00010203, 0x00010203, 0x03020100, 0x00010405, 0x00010405,
        0x00010405, 0x00010203, 0x00010203, 0x04040405, 0x00010203, 0x00010203,
        0x04040405, 0x04040405, 0x04040405, 0x00010205, 0x00010205, 0x00040405,
        0x00010405, 0x00010205, 0x04040405, 0x04040405, 0x04040405, 0x00010205,
        0x00010205, 0x00010203, 0x00010203, 0x00010203, 0x00040405, 0x00010405
    };
    uint32_t resourceSize = loader->GetTextureAlignedMaxSize(static_cast<FFLiTexturePartsType>(parts));
    void* resource = 0;
    if (!loader->IsExpand())
        resource = reinterpret_cast<void*>(allocator->Allocate(resourceSize, 0x800));
    *texture = reinterpret_cast<_GX2Texture*>(allocator->Allocate(0x9c, 4));
    int result;
    if (!loader->IsExpand())
        result = loader->LoadTexture(resource, &resourceSize, static_cast<FFLiTexturePartsType>(parts), index);
    else
        result = loader->GetPointerTextureByExpandCache(&resource, &resourceSize,
                                                         static_cast<FFLiTexturePartsType>(parts), index);
    if (result != 0)
        return result;
    FFLiResourceTextureFooter* footer = static_cast<FFLiResourceTextureFooter*>(
        FFLiResourceTextureFooter::GetFooterImpl(resource, resourceSize));
    uint16_t width;
    uint16_t height;
#if defined(RECON_NATIVE)
    host::FFLTextureDimensions dimensions;
    if (!host::decodeFFLTextureDimensions(resource, resourceSize, dimensions))
        return 3;
    width = dimensions.width;
    height = dimensions.height;
#else
    memcpy(&width, footer->imageSize, sizeof(width));
    memcpy(&height, footer->imageSize + 2, sizeof(height));
#endif
    (*texture)->surface.aaMode = 0;
    (*texture)->surface.tileMode = 0;
    (*texture)->surface.swizzle = 0;
    (*texture)->viewFirstMip = 0;
    (*texture)->viewNumMips = footer->imageSize[4];
    (*texture)->viewFirstSlice = 0;
    (*texture)->viewNumSlices = 1;
    (*texture)->surface.dim = GX2_SURFACE_DIM_TEXTURE_2D;
    (*texture)->surface.width = width;
    (*texture)->surface.height = height;
    (*texture)->surface.depth = 1;
    (*texture)->surface.numMips = footer->imageSize[4];
    (*texture)->surface.format = footer->SurfaceFormat();
    (*texture)->surface.use = 1;
    (*texture)->compMap = compMaps[(*texture)->surface.format & 0x3f];
    GX2CalcSurfaceSizeAndAlignment(&(*texture)->surface);
#if defined(RECON_NATIVE)
    if (footer->textureFormat > 2)
        return 3;
    const uint32_t pixelBytes = 1U << footer->textureFormat;
    const uint32_t tileMode = host::fflTextureTileMode(width, height, pixelBytes);
    const uint32_t pitch = host::fflTexturePitch(width, pixelBytes, tileMode);
    const uint32_t imageBytes = host::fflTextureBytes(pitch, height, pixelBytes, tileMode);
    const uint32_t available = dimensions.mipOffset ? dimensions.mipOffset : resourceSize - 12;
    if (imageBytes > available)
        return 3;
    (*texture)->surface.tileMode = tileMode;
    (*texture)->surface.pitch = pitch;
    (*texture)->surface.imageSize = imageBytes;
#endif
    GX2InitTextureRegs(*texture);
    (*texture)->surface.imagePtr = footer->GetImagePtrImpl(resourceSize);
#if defined(RECON_NATIVE)
    (*texture)->surface.mipPtr = dimensions.mipOffset ?
        static_cast<uint8_t*>(resource) + dimensions.mipOffset : 0;
#else
    (*texture)->surface.mipPtr = footer->GetMipPtrImpl(resourceSize);
#endif
    if ((*texture)->surface.numMips > 1 && (*texture)->surface.mipPtr == 0)
        (*texture)->surface.mipPtr = static_cast<uint8_t*>((*texture)->surface.imagePtr) +
                                      (*texture)->surface.mipOffset[0];
    return 0;
}

bool FFLiVerifyCharInfoWithReason(const FFLiCharInfo* info, int mode, int* reason)
{
    const bool valid = info != 0 && mode >= 0;
    if (reason != 0) *reason = valid ? 0 : 1;
    return valid;
}

bool FFLiiVerifyCharInfo__FPC12FFLiCharInfoi(const FFLiCharInfo* info, int mode)
{
    return FFLiVerifyCharInfoWithReason(info, mode, 0);
}

bool FFLiVerifyCharInfo(const FFLiCharInfo* info, int mode)
{
    return FFLiiVerifyCharInfo__FPC12FFLiCharInfoi(info, mode);
}

void FFLiSetupColorBufferForBC3__FP15_GX2ColorBufferP11_GX2TextureUi(
    _GX2ColorBuffer* buffer, _GX2Texture* texture, uint32_t level)
{
    buffer->surface = texture->surface;
    buffer->viewMip = level;
}

void FFLiSetupColorBufferForBC1__FP15_GX2ColorBufferP11_GX2TextureUi(
    _GX2ColorBuffer* buffer, _GX2Texture* texture, uint32_t level)
{
    buffer->surface = texture->surface;
    buffer->viewMip = level;
}

const void* FFLiGetResourceShapeElement__FPUiPCv18FFLiShapePartsType28FFLiResourceShapeElementType(
    uint32_t* size, const void* resource, uint32_t parts, uint32_t element)
{
    const uint32_t* words = static_cast<const uint32_t*>(resource);
    switch (element)
    {
    case 0: return __N_21_FFLiResourceShape_cpp_14c50527::GetElement(size, resource, words[0], words[6]);
    case 1: return __N_21_FFLiResourceShape_cpp_14c50527::GetElement(size, resource, words[1], words[7]);
    case 2: return __N_21_FFLiResourceShape_cpp_14c50527::GetElement(size, resource, words[2], words[8]);
    case 3: return __N_21_FFLiResourceShape_cpp_14c50527::GetElement(size, resource, words[3], words[9]);
    case 4: return __N_21_FFLiResourceShape_cpp_14c50527::GetElement(size, resource, words[4], words[10]);
    case 5: return __N_21_FFLiResourceShape_cpp_14c50527::GetElement(size, resource, words[5], words[11]);
    case 6:
        if (parts == 8) { *size = 0x48; return words + 0x12; }
        break;
    case 7:
        if (parts == 3) { *size = 0x24; return words + 0x12; }
        break;
    case 8:
        *size = 0x18;
        return words + 0xc;
    default:
        break;
    }
    *size = 0;
    return 0;
}

int FFLiGetTempBufferSizeCharModel__FPC16FFLCharModelDesc(const FFLCharModelDesc* description)
{
    if (!FFLiManager::IsConstruct()) return 0;
    return FFLiManager::GetInstance()->m_charModelCreateParam.GetTempBufferSize(description);
}

int FFLiInitCharModelCPUStep__FP13FFLiCharModelPC18FFLCharModelSourcePC16FFLCharModelDescP18FFLCharModelBuffer(
    FFLiCharModel* model, const FFLCharModelSource* source, const FFLCharModelDesc* description, FFLCharModelBuffer* buffer)
{
    if (!FFLiManager::IsConstruct()) return 4;
    FFLiManager* manager = FFLiManager::GetInstance();
    manager->CanInitCharModel();
    FFLiCharModelCreator creator(&manager->m_charModelCreateParam, manager);
    return creator.ExecuteCPUStep(model, source, description, buffer);
}

void FFLiInitCharModelGPUStep__FP13FFLiCharModelPC17FFLShaderCallback(FFLiCharModel* model, const FFLShaderCallback* callback)
{
    if (!FFLiManager::IsConstruct()) return;
    FFLiManager* manager = FFLiManager::GetInstance();
    FFLiCharModelCreator creator(&manager->m_charModelCreateParam, manager);
    creator.ExecuteGPUStep(model, callback);
}

void FFLiSetExpression__FP13FFLiCharModel13FFLExpression(FFLiCharModel* model, uint32_t expression)
{
    if ((model->description.expressionFlag & (1u << expression)) == 0) return;
    model->firstExpression = expression;
    FFLiInitModulateShapeMask__FP16FFLModulateParamRC11_GX2Texture(&model->shapes[4].modulate, model->maskTextures.textures[expression]->texture);
}

const FFLDrawParam* FFLiGetDrawParamOpaFacelineFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model) { return &model->shapes[1]; }
const FFLDrawParam* FFLiGetDrawParamOpaBeardFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model) { return &model->shapes[0]; }
const FFLDrawParam* FFLiGetDrawParamOpaNoseFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model) { return &model->shapes[6]; }
const FFLDrawParam* FFLiGetDrawParamXluMaskFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model) { return &model->shapes[4]; }
const FFLDrawParam* FFLiGetDrawParamXluNoseLineFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model) { return model->noselineTexture ? &model->shapes[5] : 0; }
const FFLDrawParam* FFLiGetDrawParamXluGlassFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model) { return model->capTexture ? &model->shapes[8] : 0; }

const FFLDrawParam* FFLiGetDrawParamOpaHairFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model)
{
    if ((uint32_t)model->modelType == 2) return 0;
    return &model->shapes[__N_17_FFLiCharModel_cpp_08c41fd9::GetShapeTypeInfo(model->modelType)[0]];
}

const FFLDrawParam* FFLiGetDrawParamOpaForeheadFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model)
{
    if ((uint32_t)model->modelType == 2) return 0;
    return &model->shapes[__N_17_FFLiCharModel_cpp_08c41fd9::GetShapeTypeInfo(model->modelType)[1]];
}

const FFLDrawParam* FFLiGetDrawParamOpaCapFromCharModel__FPC13FFLiCharModel(const FFLiCharModel* model)
{
    if ((uint32_t)model->modelType == 2 || model->facelineShapeTexture == 0) return 0;
    return &model->shapes[__N_17_FFLiCharModel_cpp_08c41fd9::GetShapeTypeInfo(model->modelType)[2]];
}

void FFLiDeleteCharModel__FP13FFLiCharModel(FFLiCharModel*) {}

int FFLGetBufferSizeCharModel(const FFLCharModelDesc* description) { return FFLiGetBufferSizeCharModel__FPC16FFLCharModelDesc(description); }
int FFLGetTempBufferSizeCharModel(const FFLCharModelDesc* description) { return FFLiGetTempBufferSizeCharModel__FPC16FFLCharModelDesc(description); }
int FFLInitCharModelCPUStep(FFLCharModel* model, const FFLCharModelSource* source, const FFLCharModelDesc* description, FFLCharModelBuffer* buffer) { return FFLiInitCharModelCPUStep__FP13FFLiCharModelPC18FFLCharModelSourcePC16FFLCharModelDescP18FFLCharModelBuffer(&model->model, source, description, buffer); }
void FFLInitCharModelGPUStepWithCallback(FFLCharModel* model, const FFLShaderCallback* callback) { FFLiInitCharModelGPUStep__FP13FFLiCharModelPC17FFLShaderCallback(&model->model, callback); }
void FFLInitCharModelGPUStep(FFLCharModel* model) { FFLInitCharModelGPUStepWithCallback(model, FFLiManager::IsConstruct() ? FFLiManager::GetInstance()->m_shaderCallback.m_callback : 0); }
void FFLDeleteCharModel(FFLCharModel* model) { FFLiDeleteCharModel__FP13FFLiCharModel(&model->model); }
void FFLSetExpression(FFLCharModel* model, uint32_t expression) { FFLiSetExpression__FP13FFLiCharModel13FFLExpression(&model->model, expression); }
const FFLDrawParam* FFLGetDrawParamOpaFaceline(const FFLCharModel* m) { return FFLiGetDrawParamOpaFacelineFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamOpaBeard(const FFLCharModel* m) { return FFLiGetDrawParamOpaBeardFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamOpaNose(const FFLCharModel* m) { return FFLiGetDrawParamOpaNoseFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamOpaForehead(const FFLCharModel* m) { return FFLiGetDrawParamOpaForeheadFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamOpaHair(const FFLCharModel* m) { return FFLiGetDrawParamOpaHairFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamOpaCap(const FFLCharModel* m) { return FFLiGetDrawParamOpaCapFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamXluMask(const FFLCharModel* m) { return FFLiGetDrawParamXluMaskFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamXluNoseLine(const FFLCharModel* m) { return FFLiGetDrawParamXluNoseLineFromCharModel__FPC13FFLiCharModel(&m->model); }
const FFLDrawParam* FFLGetDrawParamXluGlass(const FFLCharModel* m) { return FFLiGetDrawParamXluGlassFromCharModel__FPC13FFLiCharModel(&m->model); }

void FFLDrawOpaWithCallback(const FFLCharModel* model, const FFLShaderCallback* callback)
{
    FFLiShaderCallback shader; shader.Set(callback);
    const FFLDrawParam* params[] = { FFLGetDrawParamOpaFaceline(model), FFLGetDrawParamOpaBeard(model), FFLGetDrawParamOpaNose(model), FFLGetDrawParamOpaForehead(model), FFLGetDrawParamOpaHair(model), FFLGetDrawParamOpaCap(model) };
    for (uint32_t i = 0; i < 6; ++i) if (params[i]) shader.CallDraw(*params[i]);
}

void FFLDrawXluWithCallback(const FFLCharModel* model, const FFLShaderCallback* callback)
{
    FFLiShaderCallback shader; shader.Set(callback);
    const FFLDrawParam* params[] = { FFLGetDrawParamXluMask(model), FFLGetDrawParamXluNoseLine(model), FFLGetDrawParamXluGlass(model) };
    for (uint32_t i = 0; i < 3; ++i) if (params[i]) shader.CallDraw(*params[i]);
}

bool FFLIsAvailableOfficialData(uint16_t index)
{
    if (!FFLiManager::IsConstruct()) return false;
    FFLiManager* manager = FFLiManager::GetInstance();
    return manager->m_databaseFile->IsAvailable(index, true, manager->m_databaseManager.IsEnabledSpecialMii());
}

bool FFLIsRegularOfficialData(uint16_t index)
{
    if (!FFLiManager::IsConstruct()) return false;
    FFLiManager* manager = FFLiManager::GetInstance();
    return manager->m_databaseFile->IsRegular(index, true, manager->m_databaseManager.IsEnabledSpecialMii());
}

int FFLGetAvailableOfficialDataNum()
{
    if (!FFLiManager::IsConstruct()) return 0;
    FFLiManager* manager = FFLiManager::GetInstance();
    return manager->m_databaseFile->NumOfAvailable(true, manager->m_databaseManager.IsEnabledSpecialMii());
}

bool FFLiIsSameAuthorID__FPC12FFLiAuthorIDT1(const FFLiAuthorID* lhs, const FFLiAuthorID* rhs)
{
    const uint16_t* left = reinterpret_cast<const uint16_t*>(lhs->data);
    const uint16_t* right = reinterpret_cast<const uint16_t*>(rhs->data);
    return left[0] == right[0] && left[1] == right[1] && left[2] == right[2] && left[3] == right[3];
}

const FFLiCreateIDBase* FFLiGetUserID()
{
    if (!FFLiManager::IsConstruct()) return 0;
    return FFLiManager::GetInstance()->m_systemContext.UserID();
}

bool FFLiIsHomeAuthorID__FPC12FFLiAuthorID(const FFLiAuthorID* authorId)
{
    const FFLiCreateIDBase* userId = FFLiGetUserID();
    return userId != 0 && FFLiIsSameAuthorID__FPC12FFLiAuthorIDT1(
        authorId, reinterpret_cast<const FFLiAuthorID*>(userId));
}

unsigned long long FFLiGetTick__Fv() { return OSGetTime(); }

void FFLiGetDateTimeNow__FP12FFLiDateTime(FFLiDateTime* dateTime)
{
    OSCalendarTime calendar;
    OSTicksToCalendarTime(OSGetTime(), &calendar);
    dateTime->year = calendar.tm_year;
    dateTime->month = calendar.tm_mon + 1;
    dateTime->day = calendar.tm_mday;
    dateTime->hour = calendar.tm_hour;
    dateTime->minute = calendar.tm_min;
    dateTime->second = calendar.tm_sec;
}

void FFLiGetNowDateTime__FPiN51(int* year, int* month, int* day, int* hour, int* minute, int* second)
{
    FFLiDateTime dateTime;
    FFLiGetDateTimeNow__FP12FFLiDateTime(&dateTime);
    *year = dateTime.year;
    *month = dateTime.month;
    *day = dateTime.day;
    *hour = dateTime.hour;
    *minute = dateTime.minute;
    *second = dateTime.second;
}

unsigned int FFLiGetSecondsFrom_2000_01_01__Fv()
{
    FFLiDateTime now;
    FFLiGetDateTimeNow__FP12FFLiDateTime(&now);
    const int days = FFLiDateToDays__FiN21(now.year, now.month, now.day) -
                     FFLiDateToDays__FiN21(2000, 1, 1);
    return static_cast<unsigned int>(days * 86400 + now.hour * 3600 + now.minute * 60 + now.second);
}

void FFLiGetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44* projection, _Mat34* view,
                                               unsigned short width, unsigned short height)
{
    GetIconMatrix__FP6_Mat44P6_Mat34UsT3(projection, view, width, height);
}

int FFLGetBufferSizeDefaultShader(const void* shaderData)
{
    return FFLiUtilShader::GetBufferSize(5, shaderData, 0) + 0xa100;
}

int FFLiGetBufferSizeDefaultShader__FPCv(const void* shaderData)
{
    return FFLiUtilShader::GetBufferSize(5, shaderData, 0) + 0xa100;
}

int FFLGetIconTempBufferSize(const FFLIconSetting* setting)
{
    return FFLiGetDepthBufferSizeRenderTexture__FUiT1(setting->width, setting->height) + 0x9c;
}

int FFLiGetIconTempBufferSize__FPC14FFLIconSetting(const FFLIconSetting* setting)
{
    return FFLiGetDepthBufferSizeRenderTexture__FUiT1(setting->width, setting->height) + 0x9c;
}

const FFLShaderCallback* FFLiGetCallbackDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    return reinterpret_cast<const FFLShaderCallback*>(shader);
}
void FFLiDestroyDefaultShader__FP17FFLiDefaultShader(FFLiDefaultShader*) {}

void FFLSetShaderCallback(const FFLShaderCallback* callback)
{
    if (FFLiManager::IsConstruct())
        FFLiManager::GetInstance()->m_shaderCallback.Set(callback);
}

int FFLGetAdditionalInfo(FFLAdditionalInfo* output, FFLDataSource source,
                         const void* data, uint16_t index, int checkFontRegion)
{
    if (!FFLiManager::IsConstruct()) return 4;
    FFLiManager* manager = FFLiManager::GetInstance();
    FFLiCharInfo charInfo;
    const int result = manager->m_databaseManager.PickupCharInfo(&charInfo, source, data, index);
    if (result == 0)
        FFLiGetAdditionalInfo__FP17FFLAdditionalInfoPC12FFLiCharInfoi13FFLFontRegion(
            output, &charInfo, checkFontRegion, manager->m_initDesc.resourceConfig);
    return result;
}

void FFLiGetAdditionalInfo__FP17FFLAdditionalInfoPC12FFLiCharInfoi13FFLFontRegion(
    FFLAdditionalInfo* output, const FFLiCharInfo* info, int, uint32_t)
{
    const uint8_t* bytes = info->data;
    memcpy(output->name, bytes + 0xb4, sizeof(output->name));
    memcpy(output->creatorName, bytes + 0xca, sizeof(output->creatorName));
    output->name[10] = 0;
    output->creatorName[10] = 0;
    memcpy(output->createId.value, bytes + 0x108, sizeof(output->createId.value));
    memcpy(&output->skinColor, FFLiGetFacelineColor__Fi(bytes[8]), sizeof(output->skinColor));
    output->flags = 0;
    output->faceType = bytes[4];
    output->faceLine = bytes[5];
    output->faceTexture = bytes[6];
    output->hairType = bytes[7];
}

bool FFLiUseOffScreenSrgbFetch__Fv()
{
    return gOffScreenSrgbFetchEnabled != 0 && gOffScreenSrgbFetchMode == 1;
}

int FFLiGetBufferSizeFacelineTexture(uint32_t resolution, bool mipmap, bool useMipmap)
{
    const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
    const uint32_t mipCount = __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(width, resolution, mipmap);
    return static_cast<int>(__N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureBufferSize(
        width, resolution, mipCount, useMipmap));
}

int FFLiGetCompressBufferSizeFacelineTexture__FUib(uint32_t resolution, bool mipmap)
{
    const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
    const uint32_t mipCount =
        __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(width, resolution, mipmap);
    return static_cast<int>(__N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureBufferSize(
        width, resolution, mipCount, false));
}

uint32_t FFLiGetTextureMaxSizeWithAlign__FPC19FFLiResourceManager15FFLResourceType20FFLiTexturePartsType(
    const FFLiResourceManager* manager, FFLResourceType type, FFLiTexturePartsType parts)
{
    return manager->GetTextureAlignedMaxSize(type, parts) + 0x89c;
}

void FFLiTempSetContextState__FPC16_GX2ContextState(const void* state)
{
    GX2SetContextState(state);
}

void FFLiAdjustShape(FFLDrawParam* drawParam, FFLBoundingBox* bounds,
                     float scaleX, float scaleY, const FFLVec3* offset,
                     bool flip, const FFLiCoordinate* coordinate,
                     FFLiShapePartsType parts, bool adjust)
{
    FFLiAdjustShape__FP12FFLDrawParamP14FFLBoundingBoxfT3PC7FFLVec3bPC14FFLiCoordinate18FFLiShapePartsTypeT6(
        drawParam, bounds, scaleX, scaleY, offset, flip, coordinate, parts, adjust);
}

bool FFLiIsNullMiiID(const FFLCreateID* id)
{
    return FFLiIsNullMiiID(reinterpret_cast<const FFLiMiiID*>(id));
}

bool FFLiIsSameMiiID(const FFLCreateID* lhs, const FFLCreateID* rhs)
{
    return FFLiIsSameMiiID(reinterpret_cast<const FFLiMiiID*>(lhs),
                           reinterpret_cast<const FFLiMiiID*>(rhs));
}

void FFLSetExpression(FFLiCharModel& model, FFLExpression expression)
{
    FFLiSetExpression__FP13FFLiCharModel13FFLExpression(&model, expression);
}

bool FFLiIsValidCRC16(const void* data, unsigned int size)
{
    return FFLiIsValidCRC16__FPCvUi(data, size);
}

bool FFLiCheckAlignPtr(const void* pointer, unsigned int alignment)
{
    return FFLiCheckAlignPtr__FPCvUi(pointer, alignment);
}

unsigned int FFLiBugCanSwapSize(unsigned int size)
{
    return FFLiBugCanSwapSize__FUi(size);
}

void FFLiInvalidateShape(FFLDrawParam* drawParam)
{
    FFLiInvalidateShape__FP12FFLDrawParam(drawParam);
}

bool FFLiiVerifyCharInfo(const FFLiCharInfo* info, int mode)
{
    return FFLiiVerifyCharInfo__FPC12FFLiCharInfoi(info, mode);
}

int FFLGetAdditionalInfo(FFLAdditionalInfo* output, FFLDataSource source,
                         FFLStoreData* data, unsigned short index,
                         int checkFontRegion)
{
    return FFLGetAdditionalInfo(output, source, static_cast<const void*>(data),
                                index, checkFontRegion);
}

FFLColor* FFLiGetFavoriteColor(int index)
{
    return FFLiGetFavoriteColor__Fi(index);
}

bool FFLiIsTemporaryMiiID(const FFLCreateID* id)
{
    return FFLiIsTemporaryMiiID(reinterpret_cast<const FFLiMiiID*>(id));
}

void FFLiInvalidateSurface(_GX2Surface* surface, unsigned int type)
{
    FFLiInvalidateSurface__FP11_GX2Surface18_GX2InvalidateType(surface, type);
}

void FFLiInvalidateTexture(_GX2Texture* texture)
{
    FFLiInvalidateTexture__FP11_GX2Texture(texture);
}

void FFLiRenderMaskTextures(
    FFLiMaskTextures* textures, FFLiMaskTexturesTempObject* object,
    const FFLiShaderCallback* callback, FFLiCopySurface* copySurface,
    FFLiCompressorImpl<FFLiCompressorDescBC3>* compressor)
{
    FFLiRenderMaskTextures__FP16FFLiMaskTexturesP26FFLiMaskTexturesTempObjectPC18FFLiShaderCallbackP15FFLiCopySurfaceP50FFLiCompressorImpl__tm__24_21FFLiCompressorDescBC3(
        textures, object, callback,
        copySurface, compressor);
}

void* FFLiBugVgtFixedIndexPtr(void* pointer)
{
    return FFLiBugVgtFixedIndexPtr__FPv(pointer);
}

int FFLiiGetEyeRotateOffset(int index)
{
    return FFLiiGetEyeRotateOffset__Fi(index);
}

int FFLiiGetEyebrowRotateOffset(int index)
{
    return FFLiiGetEyebrowRotateOffset__Fi(index);
}

float FFLiiGetAdjustedEyeH(float value, int adjust)
{
    return FFLiiGetAdjustedEyeH__Ffi(value, adjust);
}

float FFLiiGetAdjustedMouthH(float value, int adjust)
{
    return FFLiiGetAdjustedMouthH__Ffi(value, adjust);
}

unsigned int FFLiGetResourceSurfaceFormat(int textureFormat)
{
    if (textureFormat == 1) return 7;
    if (textureFormat != 2) return 1;
    return 0x1a;
}

unsigned int FFLiIsNumberCharCode(unsigned short character)
{
    const unsigned int code = character;
    if (code - 0x0030U <= 9U) return 1;
    if (code - 0xff10U <= 9U) return 1;
    if (code - 0x2160U <= 9U) return 1;
    if (code - 0x2170U <= 9U) return 1;
    if (code - 0x2460U <= 0x13U) return 1;
    if (code - 0x2776U <= 8U) return 1;
    return 0;
}

unsigned int FFLiIsValidCharacterForName(unsigned short character)
{
    if (character != 0x25 && character != 0x5c) return 1;
    return 0;
}

void FFLiCopyWcharT2U16(unsigned short* destination, unsigned int maximumLength,
                        const wchar_t* source)
{
    unsigned int read = 0;
    for (unsigned int written = 0; written < maximumLength; ++written)
    {
        const unsigned short character = static_cast<unsigned short>(source[read]);
        destination[written] = character;
        if (character != 0) ++read;
    }
}

int FFLiGetIconTempBufferSize(const FFLIconSetting* setting)
{
    return FFLiGetIconTempBufferSize__FPC14FFLIconSetting(setting);
}

void FFLiInitModulateShapeMask(FFLModulateParam* param,
                               const _GX2Texture& texture)
{
    FFLiInitModulateShapeMask__FP16FFLModulateParamRC11_GX2Texture(param, texture);
}

uint32_t FFLiIsVaildResourceHeader(const FFLiResourceHeader* header)
{
    return FFLiIsVaildResourceHeader__FPC18FFLiResourceHeader(header);
}

void FFLiRenderFacelineTexture(
    FFLiRenderTexture* texture, const FFLiCharInfo* info, unsigned int resolution,
    FFLiFacelineTextureTempObject* object, const FFLiShaderCallback* callback,
    FFLiCopySurface* copySurface,
    FFLiCompressorImpl<FFLiCompressorDescBC1>* compressor)
{
    FFLiRenderFacelineTexture__FP17FFLiRenderTexturePC12FFLiCharInfoUiP29FFLiFacelineTextureTempObjectPC18FFLiShaderCallbackP15FFLiCopySurfaceP50FFLiCompressorImpl__tm__24_21FFLiCompressorDescBC1(
        texture, info, resolution, object,
        callback, copySurface, compressor);
}

void FFLiInitModulateShapeBeard(FFLModulateParam* param, int index)
{
    FFLiInitModulateShapeBeard__FP16FFLModulateParami(param, index);
}

void FFLiInitModulateShapeGlass(FFLModulateParam* param, int index,
                                const _GX2Texture& texture)
{
    FFLiInitModulateShapeGlass__FP16FFLModulateParamiRC11_GX2Texture(
        param, index, texture);
}

void FFLiInvalidateRenderTexture(FFLiRenderTexture* texture)
{
    FFLiInvalidateRenderTexture__FP17FFLiRenderTexture(texture);
}

void FFLiMiiDataOfficial2CharInfo(FFLiCharInfo* destination,
                                  const FFLiMiiDataOfficial& source)
{
    FFLiMiiDataOfficial2CharInfo__FP12FFLiCharInfoRC19FFLiMiiDataOfficial(
        destination, source);
}

int FFLiGetBufferSizeMaskTextures(unsigned int expressionMask,
                                  unsigned int resolution, bool mipmap,
                                  bool useMipmap)
{
    return FFLiGetBufferSizeMaskTextures__FUiT1bT3(
        expressionMask, resolution, mipmap, useMipmap);
}

void FFLiInitModulateShapeForehead(FFLModulateParam* param, int index)
{
    FFLiInitModulateShapeForehead__FP16FFLModulateParami(param, index);
}

void FFLiInitModulateShapeNoseline(FFLModulateParam* param,
                                   const _GX2Texture& texture)
{
    FFLiInitModulateShapeNoseline__FP16FFLModulateParamRC11_GX2Texture(
        param, texture);
}

unsigned int FFLiConvertFSStatusToFFLResult(int status)
{
    return FFLiConvertFSStatusToFFLResult__Fi(status);
}

int FFLiMiiDataCoreRFL2MiiDataCore(FFLiMiiDataCore* destination,
                                   const FFLiMiiDataCoreRFL& source,
                                   bool replaceName)
{
    return FFLiMiiDataCoreRFL2MiiDataCore__FP15FFLiMiiDataCoreRC18FFLiMiiDataCoreRFLb(
        destination, source, replaceName);
}

void FFLiClearCreatorNameFromOfficial(FFLiMiiDataOfficial* data)
{
    FFLiClearCreatorNameFromOfficial__FP19FFLiMiiDataOfficial(data);
}

int FFLiLoadShape(FFLDrawParam* drawParam, FFLBoundingBox* bounds,
                  FFLiCharModel* model, FFLiShapePartsType parts,
                  unsigned short index, FFLiResourceLoader* loader,
                  FFLiBufferAllocator* allocator)
{
    return FFLiLoadShape__FP12FFLDrawParamP14FFLBoundingBoxP13FFLiCharModel18FFLiShapePartsTypeUsP18FFLiResourceLoaderP19FFLiBufferAllocator(
        drawParam, bounds, model, parts, index, loader, allocator);
}

void FFLiInvalidate(unsigned int type, void* data, unsigned int size)
{
    FFLiInvalidate__F18_GX2InvalidateTypePvUi(type, data, size);
}

int FFLiCanDrawShape(const FFLDrawParam* drawParam)
{
    return FFLiCanDrawShape__FPC12FFLDrawParam(drawParam);
}

bool FFLiIsValidMiiID(const FFLCreateID* id)
{
    return FFLiIsValidMiiID(reinterpret_cast<const FFLiMiiID*>(id));
}

void FFLiBugEndianSwap(void* data, unsigned int size)
{
    FFLiBugEndianSwap__FPvUi(data, size);
}

void FFLDeleteCharModel(void* model)
{
    FFLDeleteCharModel(reinterpret_cast<FFLCharModel*>(model));
}

bool FFLiIsSpecialMiiID(const FFLCreateID* id)
{
    const FFLiMiiID* value = reinterpret_cast<const FFLiMiiID*>(id);
    return FFLiIsSpecialMiiID(value);
}

int FFLiGetDatabasePath(char* path, unsigned int size,
                        unsigned long long titleId, const char* suffix)
{
    return FFLiGetDatabasePath__FPcUiULPCc(path, size, titleId, suffix);
}

int FFLiGetResourcePath(char* path, unsigned int size, const char* suffix)
{
    return FFLiGetResourcePath__FPcUiPCc(path, size, suffix);
}

void FFLSetShaderCallback(void* callback)
{
    FFLSetShaderCallback(reinterpret_cast<const FFLShaderCallback*>(callback));
}

unsigned int FFLiInitMaskTextures(FFLiMaskTextures* textures,
                                   unsigned int expressionMask,
                                   unsigned int resolution, bool mipmap,
                                   bool useMipmap, FFLiBufferAllocator* allocator)
{
    return FFLiInitMaskTextures__FP16FFLiMaskTexturesUiT2bT4P19FFLiBufferAllocator(
        textures, expressionMask, resolution, mipmap, useMipmap, allocator);
}

unsigned int FFLiGetIconBufferSize(const FFLIconSetting* setting)
{
    return FFLiGetIconBufferSize__FPC14FFLIconSetting(setting);
}

void FFLiInitRenderTexture(FFLiRenderTexture* texture, unsigned int width,
                           unsigned int height, unsigned int format,
                           FFLiBufferAllocator* allocator)
{
    FFLiInitRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P19FFLiBufferAllocator(
        texture, width, height, format, 1, allocator);
}

void FFLiTempSetShaderMode(int mode)
{
    FFLiTempSetShaderMode__F14_GX2ShaderMode(static_cast<uint32_t>(mode));
}

void FFLDrawOpaWithCallback(const FFLiCharModel& model,
                            const FFLShaderCallback* callback)
{
    FFLDrawOpaWithCallback(reinterpret_cast<const FFLCharModel*>(&model), callback);
}

void FFLDrawXluWithCallback(const FFLiCharModel& model,
                            const FFLShaderCallback* callback)
{
    FFLDrawXluWithCallback(reinterpret_cast<const FFLCharModel*>(&model), callback);
}

void FFLiFlushRenderTexture(FFLiRenderTexture* texture)
{
    FFLiFlushRenderTexture__FP17FFLiRenderTexture(texture);
}

unsigned int FFLiGetMipMapLevelSize(unsigned int baseSize, unsigned int level)
{
    return FFLiGetMipMapLevelSize__FUiT1(baseSize, level);
}

void FFLiSetupRenderTexture(FFLiRenderTexture* texture,
                            const FFLColor* clearColor, void* depthBuffer,
                            unsigned int mipLevel, unsigned int format,
                            const FFLShaderCallback* callback)
{
    FFLiShaderCallback shader;
    shader.Set(callback);
    FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
        texture, clearColor, depthBuffer, mipLevel, format, &shader);
}

void FFLiInitFacelineTexture(FFLiRenderTexture* texture, unsigned int resolution,
                             bool mipmap, bool useMipmap,
                             FFLiBufferAllocator* allocator)
{
    FFLiInitFacelineTexture__FP17FFLiRenderTextureUibT3P19FFLiBufferAllocator(
        texture, resolution, mipmap, useMipmap, allocator);
}

void FFLiTempSetContextState(const _GX2ContextState* state)
{
    FFLiTempSetContextState__FPC16_GX2ContextState(state);
}

void FFLiInitModulateShapeCap(FFLModulateParam* param, int index,
                              const _GX2Texture& texture)
{
    FFLiInitModulateShapeCap__FP16FFLModulateParamiRC11_GX2Texture(param, index, texture);
}

void FFLiInitModulateShapeHair(FFLModulateParam* param, int index)
{
    FFLiInitModulateShapeHair__FP16FFLModulateParami(param, index);
}

void FFLiInitModulateShapeNose(FFLModulateParam* param, int index)
{
    FFLiInitModulateShapeNose__FP16FFLModulateParami(param, index);
}

int FFLiGetBufferRenderTexture(unsigned int width, unsigned int height,
                               unsigned int format, unsigned int mipCount)
{
    return FFLiGetBufferRenderTexture__FUiT117_GX2SurfaceFormatT1(width, height, format, mipCount);
}

int FFLiStoreDataCFLToCharInfo(FFLiCharInfo* destination,
                               const FFLiStoreDataCFL& source)
{
    return FFLiStoreDataCFLToCharInfo__FP12FFLiCharInfoRC16FFLiStoreDataCFL(destination, source);
}

unsigned int FFLiBugCanVgtFixedIndexSize(unsigned int size)
{
    return FFLiBugCanVgtFixedIndexSize__FUi(size);
}

unsigned int FFLiBugCanVgtFixedIndexSize()
{
    return FFLiBugCanVgtFixedIndexSize__FUi(0);
}

int FFLiLoadTextureWithAllocate(_GX2Texture** texture,
                                FFLiTexturePartsType parts, unsigned int index,
                                FFLiResourceLoader* loader,
                                FFLiBufferAllocator* allocator)
{
    return FFLiLoadTextureWithAllocate__FPP11_GX2Texture20FFLiTexturePartsTypeUiP18FFLiResourceLoaderP19FFLiBufferAllocator(
        texture, parts, index, loader, allocator);
}

void FFLiInitModulateShapeFaceline(FFLModulateParam* param,
                                   const _GX2Texture& texture)
{
    FFLiInitModulateShapeFaceline__FP16FFLModulateParamRC11_GX2Texture(param, texture);
}

uint32_t FFLiGetTextureMaxSizeWithAlign(const FFLiResourceManager* manager,
                                        FFLResourceType type,
                                        FFLiTexturePartsType parts)
{
    return FFLiGetTextureMaxSizeWithAlign__FPC19FFLiResourceManager15FFLResourceType20FFLiTexturePartsType(manager, type, parts);
}

int FFLiInitTempObjectMaskTextures(
    FFLiMaskTexturesTempObject* object, const FFLiMaskTextures* textures,
    const FFLiCharInfo* info, unsigned int expressionMask, unsigned int resolution,
    bool mipmap, bool useMipmap, bool useUB, FFLiResourceLoader* loader,
    FFLiBufferAllocator* allocator, FFLiRenderTextureBuffer* buffer)
{
    return FFLiInitTempObjectMaskTextures__FP26FFLiMaskTexturesTempObjectPC16FFLiMaskTexturesPC12FFLiCharInfoUiT4bN26P18FFLiResourceLoaderP19FFLiBufferAllocatorP23FFLiRenderTextureBuffer(
        object, textures, info, expressionMask, resolution, mipmap, useMipmap,
        useUB, loader, allocator, buffer);
}

int FFLiGetTempBufferSizeMaskTextures(unsigned int expressionMask, unsigned int resolution,
                                      bool mipmap, bool useMipmap,
                                      const FFLiResourceManager* manager,
                                      FFLResourceType type)
{
    return FFLiGetTempBufferSizeMaskTextures__FUiT1bT3P19FFLiResourceManager15FFLResourceType(
        expressionMask, resolution, mipmap, useMipmap,
        const_cast<FFLiResourceManager*>(manager), type);
}

int FFLiInitTempObjectFacelineTexture(
    FFLiFacelineTextureTempObject* object, FFLiRenderTexture* texture,
    const FFLiCharInfo* info, unsigned int resolution, bool mipmap,
    bool useMipmap, bool useUB, FFLiResourceLoader* loader,
    FFLiBufferAllocator* allocator, FFLiRenderTextureBuffer* buffer)
{
    return FFLiInitTempObjectFacelineTexture__FP29FFLiFacelineTextureTempObjectP17FFLiRenderTexturePC12FFLiCharInfoUibN25P18FFLiResourceLoaderP19FFLiBufferAllocatorP23FFLiRenderTextureBuffer(
        object, texture, info, resolution, mipmap, useMipmap, useUB,
        loader, allocator, buffer);
}

int FFLiGetDepthBufferSizeRenderTexture(unsigned int width, unsigned int height)
{
    return FFLiGetDepthBufferSizeRenderTexture__FUiT1(width, height);
}

int FFLiGetCompressBufferSizeMaskTexture(unsigned int resolution, bool mipmap)
{
    return FFLiGetCompressBufferSizeMaskTexture__FUib(resolution, mipmap);
}

int FFLiGetTempBufferSizeFacelineTexture(unsigned int resolution, bool mipmap,
                                         bool useMipmap,
                                         const FFLiResourceManager* manager,
                                         FFLResourceType type)
{
    return FFLiGetTempBufferSizeFacelineTexture__FUibT2P19FFLiResourceManager15FFLResourceType(
        resolution, mipmap, useMipmap, const_cast<FFLiResourceManager*>(manager), type);
}

int FFLiGetCompressBufferSizeFacelineTexture(unsigned int resolution, bool mipmap)
{
    return FFLiGetCompressBufferSizeFacelineTexture__FUib(resolution, mipmap);
}

unsigned int FFLiMin(unsigned int lhs, unsigned int rhs)
{
    return FFLiMin__tm__3_Ui__FZ1ZT1_Z1Z(lhs, rhs);
}
