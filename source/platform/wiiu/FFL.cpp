#include "platform/wiiu/FFL.hpp"
#include "platform/wiiu/WiiU_GX2.hpp"
#include "platform/wiiu/FFLiDatabaseManager.hpp"
#define private public
#include "platform/wiiu/FFLiManager.hpp"
#undef private
#include "vendor/__N_17_FFLiCharModel_cpp_08c41fd9.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "vendor/__N_13_FFLiColor_cpp_87b9af10.hpp"
#include "vendor/__N_15_FFLiManager_cpp_28866420.hpp"
#include "vendor/__N_21_FFLiDefaultShader_cpp_d69dbbf4.hpp"
#include "vendor/__N_20_FFLDefaultShader_cpp_5ec7e0a3.hpp"
#include "vendor/__N_12_FFLiIcon_cpp_65029ad9.hpp"
#include "vendor/__N_20_FFLiMaskTextures_cpp_feacafbb.hpp"
#include "vendor/__N_23_FFLiFacelineTexture_cpp_cff65387.hpp"
#include "platform/wiiu/FFLiTypes.hpp"
#include "platform/wiiu/FFLiResourceLoader.hpp"
#include "platform/wiiu/FFLiCompressor.hpp"
#include "platform/wiiu/FFLiDatabaseDefault.hpp"
#include "platform/wiiu/FFLiStoreDataCFL.hpp"
#include "vendor/__N_20_FFLiRawMaskParts_cpp_7c9651f3.hpp"

#include <sysapp/title.h>
#include <string.h>

extern void instructionSynchronize();
extern void GetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);

static FFLiDefaultShader sDefaultShader;
static void* sDefaultShaderWork;
#include "platform/wiiu/FFL_gFFLIconShaderData.inc"

uint32_t gOffScreenSrgbFetchMode = 0;
uint8_t gOffScreenSrgbFetchEnabled = 1;

static const void* resolveDefaultShaderData(const char* name)
{
    if (name == 0 || strcmp(name, "Gfx2") != 0) return 0;
    return gFFLIconShaderData;
}

extern "C" void GX2SetContextState(const void*);
extern "C" void GX2SetShaderModeEx(uint32_t, uint32_t, uint32_t, uint32_t,
                                    uint32_t, uint32_t, uint32_t);
extern "C" void GX2SetFetchShader(const void*);
extern "C" void GX2SetVertexShader(const void*);
extern "C" void GX2SetPixelShader(const void*);
extern "C" void GX2InitSampler(void*, unsigned int, unsigned int);
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
    static const uint8_t counts[] = { 0x0c,0x0c,0x09,0x02,0x25,0x06,0x12,0x00,0x04,0x84,0x84 };
    return type < sizeof(counts) ? counts[type] : 0;
}
uint8_t FFLiGetShapeResourceNum__F18FFLiShapePartsType(uint32_t type)
{
    static const uint8_t counts[] = { 0x01,0x0c,0x12,0x12,0x84,0x84,0x84,0x84,0x08,0x09,0x0a,0x0b };
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
    static const uint8_t offsets[64] = { 3,4,4,4,3,4,4,4,3,4,4,4,4,3,3,4 };
    return 0x20 - offsets[index];
}
int FFLiiGetEyebrowRotateOffset__Fi(int index)
{
    static const uint8_t offsets[64] = { 6,6,5,7,6,7,6,7,4,7,6,8,5,5,6,6 };
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

int FFLiInitDefaultShader__FP17FFLiDefaultShaderPvPCv(FFLiDefaultShader*, void*, const void*);
void FFLiSetDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
void FFLiSetupInitCharModelDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
void FFLiSetupDrawDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*);
void FFLiGetContextStateDefaultShader__FP17FFLiDefaultShader(FFLiDefaultShader*);
void FFLiSetMatrixDefaultShader__FP17FFLiDefaultShaderPC6_Mat44PC6_Mat34(
    FFLiDefaultShader*, const _Mat44*, const _Mat34*);
void FFLiGetIconMatrix__FP6_Mat44P6_Mat34UsT3(_Mat44*, _Mat34*, unsigned short, unsigned short);

int FFLInitDefaultShader(void* shader, void* buffer, const char* name)
{
    const void* shaderData = resolveDefaultShaderData(name);
    if (shader == 0 || buffer == 0 || shaderData == 0) return 1;
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

void FFLGetContextStateDefaultShader(void* shader)
{
    FFLiGetContextStateDefaultShader__FP17FFLiDefaultShader(
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

int FFLInitRes(void* work, FFLResourceDesc* resourceDesc)
{
    return __N_15_FFLiManager_cpp_28866420::InitResImpl(work, 0,
                                                         resourceDesc);
}

void FFLInitResGPUStep(void)
{
    if (FFLiManager::IsConstruct()) FFLiManager::GetInstance()->SetupGPU();
}

int FFLGetWorkSize(FFLResourceDesc* resourceDesc)
{
    return FFLiManager::GetBufferSize(reinterpret_cast<const FFLInitDesc*>(resourceDesc));
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
        callback = reinterpret_cast<const FFLShaderCallback*>(
            &FFLiManager::GetInstance()->m_shaderCallback);
    return FFLMakeIconExWithCallback(texture, model, expression, setting,
                                      buffer, temporaryBuffer, callback, true);
}

void FFLGetIconMatrix(_Mat44* projection, _Mat34* view,
                      unsigned short width, unsigned short height)
{
    FFLiGetIconMatrix__FP6_Mat44P6_Mat34UsT3(projection, view, width, height);
}

int FFLiInitDefaultShader__FP17FFLiDefaultShaderPvPCv(
    FFLiDefaultShader* shader, void* buffer, const void* shaderData)
{
    if (shader == 0 || buffer == 0) return 1;
    sDefaultShaderWork = buffer;
    if (shaderData == 0) return 1;
    InitVertexShader__FPP16_GX2VertexShaderPCv(&shader->m_vertexShader, shaderData);
    InitPixelShader__FPP15_GX2PixelShaderPCv(&shader->m_pixelShader, shaderData);
    InitFetchShader__FP11FetchShaderPC16_GX2VertexShader(&shader->m_fetchShader, shader->m_vertexShader);
    GX2InitSampler(&shader->m_sampler, 1, 1);
    GX2InitSampler(&shader->m_samplerAlt, 1, 1);
    shader->m_enabled = 1;
    FFLiUtilShader::GetUniformLocation(&shader->m_vertexUniformMatrix, shader->m_vertexShader, "u_mvp");
    FFLiUtilShader::GetUniformLocation(&shader->m_vertexUniformView, shader->m_vertexShader, "u_mv");
    FFLiUtilShader::GetUniformLocation(&shader->m_vertexUniformNormal, shader->m_vertexShader, "u_mn");
    FFLiUtilShader::GetSamplerLocation(&shader->m_pixelSamplerLocation, shader->m_pixelShader, "s_texture");
    FFLiUtilShader::GetUniformLocation(&shader->m_mode, shader->m_pixelShader, "u_mode");
    FFLiUtilShader::GetUniformLocation(&shader->m_pixelConst1, shader->m_pixelShader, "u_const1");
    FFLiUtilShader::GetUniformLocation(&shader->m_pixelConst2, shader->m_pixelShader, "u_const2");
    FFLiUtilShader::GetUniformLocation(&shader->m_pixelConst3, shader->m_pixelShader, "u_const3");
    FFLiUtilShader::GetUniformLocation(&shader->m_lightEnable, shader->m_pixelShader, "u_light_enable");
    FFLiUtilShader::GetUniformLocation(&shader->m_lightDirection, shader->m_pixelShader, "u_light_dir");
    FFLiUtilShader::GetUniformLocation(&shader->m_lightAmbient, shader->m_pixelShader, "u_light_ambient");
    FFLiUtilShader::GetUniformLocation(&shader->m_lightDiffuse, shader->m_pixelShader, "u_light_diffuse");
    FFLiUtilShader::GetUniformLocation(&shader->m_lightSpecular, shader->m_pixelShader, "u_light_specular");
    FFLiUtilShader::GetUniformLocation(&shader->m_materialAmbient, shader->m_pixelShader, "u_material_ambient");
    FFLiUtilShader::GetUniformLocation(&shader->m_materialDiffuse, shader->m_pixelShader, "u_material_diffuse");
    FFLiUtilShader::GetUniformLocation(&shader->m_materialSpecular, shader->m_pixelShader, "u_material_specular");
    FFLiUtilShader::GetUniformLocation(&shader->m_materialSpecularPower, shader->m_pixelShader, "u_material_specular_power");
    FFLiUtilShader::GetUniformLocation(&shader->m_materialSpecularMode, shader->m_pixelShader, "u_material_specular_mode");
    FFLiUtilShader::GetUniformLocation(&shader->m_rimColor, shader->m_pixelShader, "u_rim_color");
    FFLiUtilShader::GetUniformLocation(&shader->m_rimPower, shader->m_pixelShader, "u_rim_power");
    return 0;
}

void FFLiSetDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    if (shader == 0) return;
    GX2SetFetchShader(&shader->m_fetchShader);
    GX2SetVertexShader(shader->m_vertexShader);
    GX2SetPixelShader(shader->m_pixelShader);
}

void FFLiSetupInitCharModelDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    if (shader == 0) return;
    FFLiSetDefaultShader__FPC17FFLiDefaultShader(shader);
    __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformParam(shader);
}

void FFLiSetupInitCharModelDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    FFLiSetupInitCharModelDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(shader);
}

void FFLiSetupDrawDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    if (shader == 0) return;
    __N_21_FFLiDefaultShader_cpp_d69dbbf4::SetupUniformParam(shader);
}

void FFLiSetupDrawDefaultShader__FPC17FFLiDefaultShader(
    const FFLiDefaultShader* shader)
{
    FFLiSetupDrawDefaultShaderNoSetCallback__FPC17FFLiDefaultShader(shader);
}

void FFLiGetContextStateDefaultShader__FP17FFLiDefaultShader(
    FFLiDefaultShader* shader)
{
    if (shader != 0) GX2SetContextState(&shader->m_fetchShader);
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

int FFLiGetTempBufferSizeFacelineTexture__FUibT2P19FFLiResourceManager15FFLResourceType(
    uint32_t resolution, bool mipmap, bool useMipmap,
    FFLiResourceManager* resourceManager, FFLResourceType resourceType)
{
    const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
    const uint32_t height = __N_23_FFLiFacelineTexture_cpp_cff65387::GetHeight(resolution);
    const uint32_t mipCount = __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(
        width, height, mipmap && useMipmap);
    const uint32_t texture = __N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureBufferSize(
        width, height, mipCount, false);
    (void)resourceManager;
    (void)resourceType;
    return static_cast<int>(texture + sizeof(FFLiFacelineTextureTempObject) + 0x100);
}

void FFLiInitFacelineTexture__FP17FFLiRenderTextureUibT3P19FFLiBufferAllocator(
    FFLiRenderTexture* texture, uint32_t resolution, bool mipmap, bool useMipmap,
    FFLiBufferAllocator* allocator)
{
    const uint32_t width = __N_23_FFLiFacelineTexture_cpp_cff65387::GetWidth(resolution);
    const uint32_t height = __N_23_FFLiFacelineTexture_cpp_cff65387::GetHeight(resolution);
    const uint32_t mipCount = __N_23_FFLiFacelineTexture_cpp_cff65387::GetNumMips(
        width, height, mipmap && useMipmap);
    const uint32_t size = __N_23_FFLiFacelineTexture_cpp_cff65387::GetTextureBufferSize(
        width, height, mipCount, false);
    texture->texture.surface.imagePtr = reinterpret_cast<void*>(allocator->Allocate(size, texture->texture.surface.alignment));
    texture->texture.surface.mipPtr = 0;
    GX2InitTexture(&texture->texture, width, height, 0, mipCount,
                   GX2_SURFACE_FORMAT_UNORM_R8_G8_B8_A8, GX2_SURFACE_DIM_TEXTURE_2D);
}

int FFLiInitTempObjectFacelineTexture__FP29FFLiFacelineTextureTempObjectP17FFLiRenderTexturePC12FFLiCharInfoUibN25P18FFLiResourceLoaderP19FFLiBufferAllocatorP23FFLiRenderTextureBuffer(
    FFLiFacelineTextureTempObject* object, FFLiRenderTexture* renderTexture,
    const FFLiCharInfo* info, uint32_t resolution, bool mipmap, bool useMipmap,
    bool useUB, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator,
    FFLiRenderTextureBuffer* buffer)
{
    object->renderTexture = renderTexture;
    object->compressorParam = 0;
    object->faceline.texture = 0;
    object->make.texture = 0;
    object->beard.texture = 0;
    (void)info; (void)resolution; (void)mipmap; (void)useMipmap;
    (void)useUB; (void)loader; (void)allocator; (void)buffer;
    return 0;
}

void FFLiRenderFacelineTexture__FP17FFLiRenderTexturePC12FFLiCharInfoUiP29FFLiFacelineTextureTempObjectPC18FFLiShaderCallbackP15FFLiCopySurfaceP50FFLiCompressorImpl__tm__24_21FFLiCompressorDescBC1(
    FFLiRenderTexture* texture, const FFLiCharInfo* info, uint32_t resolution,
    FFLiFacelineTextureTempObject* object, const FFLShaderCallback* callback,
    FFLiCopySurface* copySurface, FFLiCompressorImpl<FFLiCompressorDescBC1>* compressor)
{
    (void)texture; (void)info; (void)resolution; (void)object;
    (void)callback; (void)copySurface; (void)compressor;
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
                                             bool useMipmap)
{
    const int count = __N_20_FFLiMaskTextures_cpp_feacafbb::ExpressionFlagCount(expressionMask);
    const uint32_t width = resolution * 2;
    const uint32_t levels = mipmap && useMipmap ? FFLiGetMipMapNum(width, width) : 1;
    return static_cast<int>(count * __N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureBufferSize(
        width, width, false) + levels * sizeof(FFLiRenderTexture*));
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
    const uint32_t width = resolution * 2;
    const uint32_t levels = mipmap ? FFLiGetMipMapNum(width, width) : 1;
    return static_cast<int>(__N_20_FFLiMaskTextures_cpp_feacafbb::GetTextureBufferSize(
        width, width, true) * 9 + levels * 0x40);
}

unsigned int FFLiInitMaskTextures__FP16FFLiMaskTexturesUiT2bT4P19FFLiBufferAllocator(
    FFLiMaskTextures* textures, uint32_t expressionMask, uint32_t resolution,
    bool mipmap, bool useMipmap, FFLiBufferAllocator* allocator)
{
    const uint32_t count = static_cast<uint32_t>(
        __N_20_FFLiMaskTextures_cpp_feacafbb::ExpressionFlagCount(expressionMask));
    for (uint32_t i = 0; i < 19; ++i) textures->textures[i] = 0;
    for (uint32_t i = 0; i < count; ++i)
        textures->textures[i] = reinterpret_cast<FFLiRenderTexture*>(allocator->Allocate(
            sizeof(FFLiRenderTexture), 4));
    (void)resolution; (void)mipmap; (void)useMipmap;
    return count == 0 ? 0 : 1;
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
    const FFLShaderCallback* callback, FFLiCopySurface* copySurface,
    FFLiCompressorImpl<FFLiCompressorDescBC3>* compressor)
{
    (void)textures; (void)object; (void)callback; (void)copySurface; (void)compressor;
}

void FFLiMiiDataCoreRFL2CharInfo__FP12FFLiCharInfoRC18FFLiMiiDataCoreRFLPCUsb(
    FFLiCharInfo* destination, const FFLiMiiDataCoreRFL& source,
    const uint16_t* replacementName)
{
    memcpy(destination->data, &source, sizeof(destination->data));
    if (replacementName != 0) memcpy(destination->data + 0xb4, replacementName, 22);
}

void FFLiCharInfo2MiiDataCore__FP15FFLiMiiDataCoreRC12FFLiCharInfob(
    FFLiMiiDataCore* destination, const FFLiCharInfo& source, bool replaceName)
{
    memset(destination, 0, sizeof(*destination));
    destination->data[0] = source.data[0];
    destination->data[1] = source.data[1];
    destination->data[2] = source.data[2];
    destination->data[3] = source.data[3];
    destination->data[4] = source.data[4];
    destination->data[5] = source.data[5];
    destination->data[6] = source.data[6];
    destination->data[7] = source.data[7];
    destination->data[8] = source.data[8];
    destination->data[9] = source.data[9];
    destination->data[10] = source.data[10];
    destination->data[11] = source.data[11];
    memcpy(destination->data + 12, source.data + 0x108, 10);
    if (replaceName) memcpy(destination->data + 22, source.data + 0xb4, 5);
}

int FFLiMiiDataCoreRFL2MiiDataCore__FP15FFLiMiiDataCoreRC18FFLiMiiDataCoreRFLb(
    FFLiMiiDataCore* destination, const FFLiMiiDataCoreRFL& source, bool replaceName)
{
    FFLiCharInfo info = {};
    FFLiMiiDataCoreRFL2CharInfo__FP12FFLiCharInfoRC18FFLiMiiDataCoreRFLPCUsb(
        &info, source, 0);
    FFLiCharInfo2MiiDataCore__FP15FFLiMiiDataCoreRC12FFLiCharInfob(
        destination, info, replaceName);
    return 0;
}

void FFLiClearCreatorNameFromOfficial__FP19FFLiMiiDataOfficial(
    FFLiMiiDataOfficial* data)
{
    memset(data->authorId.data, 0, sizeof(data->authorId.data));
}

int FFLiMiiDataCore2CharInfo__FP12FFLiCharInfoRC15FFLiMiiDataCorePCUsb(
    FFLiCharInfo* destination, const FFLiMiiDataCore& source,
    const uint16_t* replacementName)
{
    memset(destination, 0, sizeof(*destination));
    memcpy(destination->data, source.data, sizeof(source.data));
    if (replacementName != 0) memcpy(destination->data + 0xb4, replacementName, 22);
    return 0;
}

void FFLiMiiDataOfficial2CharInfo__FP12FFLiCharInfoRC19FFLiMiiDataOfficial(
    FFLiCharInfo* destination, const FFLiMiiDataOfficial& source)
{
    FFLiMiiDataCore2CharInfo__FP12FFLiCharInfoRC15FFLiMiiDataCorePCUsb(
        destination, source, 0);
}

int FFLiStoreDataCFLToCharInfo__FP12FFLiCharInfoRC16FFLiStoreDataCFL(
    FFLiCharInfo* destination, const FFLiStoreDataCFL& source)
{
    if (!source.IsValidCRC()) return 3;
    FFLiMiiDataOfficial2CharInfo__FP12FFLiCharInfoRC19FFLiMiiDataOfficial(
        destination, source);
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
    return FFLiResourceManager::GetResourcePath(path, size, static_cast<FFLResourceType>(0), suffix != 0);
}

int FFLiGetDatabasePath__FPcUiULPCc(char* path, uint32_t size, unsigned long long titleId,
                                    const char* suffix)
{
    (void)titleId;
    return FFLiResourceManager::GetResourcePath(path, size, static_cast<FFLResourceType>(1), suffix != 0);
}

void FFLiInitDrawParamRawMask__FP20FFLiRawMaskDrawParamPC12FFLiCharInfoiN23PC22FFLiRawMaskTextureDescP19FFLiBufferAllocator(
    void* drawParam, const FFLiCharInfo* info, int rotation, int eyeAdjust,
    int mouthAdjust, const void* textureDesc, FFLiBufferAllocator* allocator)
{
    memset(drawParam, 0, 0x300);
    (void)info; (void)rotation; (void)eyeAdjust; (void)mouthAdjust;
    (void)textureDesc; (void)allocator;
}

void FFLiInvalidateRawMask__FP20FFLiRawMaskDrawParam(void* drawParam)
{
    GX2Invalidate(0x10, drawParam, 0x300);
}

void FFLiDrawRawMask__FPC20FFLiRawMaskDrawParamPC18FFLiShaderCallback(
    const void* drawParam, const FFLShaderCallback* callback)
{
    (void)drawParam;
    FFLiShaderCallback shader; shader.Set(callback); shader.CallSetContextState();
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
    const void* drawParam, const FFLShaderCallback* callback)
{
    (void)drawParam;
    FFLiShaderCallback shader; shader.Set(callback); shader.CallSetContextState();
}

void FFLiInitRenderTexture__FP17FFLiRenderTextureUiT217_GX2SurfaceFormatT2P19FFLiBufferAllocator(
    FFLiRenderTexture* texture, uint32_t width, uint32_t height, uint32_t format,
    uint32_t mipCount, FFLiBufferAllocator* allocator)
{
    GX2InitTexture(&texture->texture, width, height, 0, mipCount,
                   static_cast<_GX2SurfaceFormat>(format), GX2_SURFACE_DIM_TEXTURE_2D);
    texture->texture.surface.imagePtr = reinterpret_cast<void*>(allocator->Allocate(
        texture->texture.surface.imageSize, texture->texture.surface.alignment));
    texture->texture.surface.mipPtr = texture->texture.surface.mipSize == 0 ? 0 :
        reinterpret_cast<void*>(allocator->Allocate(texture->texture.surface.mipSize, texture->texture.surface.alignment));
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
    uint32_t depthSize, uint32_t format, const FFLShaderCallback* callback)
{
    (void)texture; (void)clearColor; (void)depthBuffer; (void)depthSize; (void)format;
    FFLiShaderCallback shader; shader.Set(callback); shader.CallSetContextState();
}

void FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
    FFLiRenderTexture* texture, const FFLColor* clearColor, void* depthBuffer,
    uint32_t depthSize, uint32_t format, const FFLShaderCallback* callback)
{
    FFLiSetupRenderTexture__FP11_GX2TexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
        &texture->texture, clearColor, depthBuffer, depthSize, format, callback);
}

uint32_t FFLiGetTextureResoucePartsInfos__FPUiP25FFLiResourceTextureHeader20FFLiTexturePartsType(
    uint32_t* size, const void* header, uint32_t parts)
{
    *size = 0;
    (void)header; (void)parts;
    return 0;
}

uint32_t FFLiGetShapeResoucePartsInfos__FPUiP23FFLiResourceShapeHeader18FFLiShapePartsType(
    uint32_t* size, const void* header, uint32_t parts)
{
    *size = 0;
    (void)header; (void)parts;
    return 0;
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
    (void)drawParam; (void)bounds; (void)model; (void)parts; (void)index;
    (void)loader; (void)allocator;
    return 0;
}

void FFLiAdjustShape__FP12FFLDrawParamP14FFLBoundingBoxfT3PC7FFLVec3bPC14FFLiCoordinate18FFLiShapePartsTypeT6(
    FFLDrawParam* drawParam, FFLBoundingBox* bounds, float scaleX, float scaleY,
    const FFLVec3* offset, bool flip, const FFLiCoordinate* coordinate,
    uint32_t parts, bool adjust)
{
    (void)drawParam; (void)bounds; (void)scaleX; (void)scaleY; (void)offset;
    (void)flip; (void)coordinate; (void)parts; (void)adjust;
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
    uint32_t size = loader->GetTextureSize(static_cast<FFLiTexturePartsType>(parts), index);
    *texture = reinterpret_cast<_GX2Texture*>(allocator->Allocate(sizeof(_GX2Texture), 4));
    return loader->LoadTexture(*texture, &size, static_cast<FFLiTexturePartsType>(parts), index);
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
    *size = 0;
    return static_cast<const uint8_t*>(resource) + (parts * 0x20u) + element * 4u;
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

int FFLGetBufferSizeDefaultShader(const char* name)
{
    const void* shaderData = resolveDefaultShaderData(name);
    return shaderData == 0 ? 0 : FFLiUtilShader::GetBufferSize(5, shaderData, 0) + 0xa100;
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

void FFLiGetCallbackDefaultShader__FPC17FFLiDefaultShader(const FFLiDefaultShader*) {}
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
    const uint32_t width = resolution * 2;
    const uint32_t levels = mipmap ? FFLiGetMipMapNum(width, width) : 1;
    return FFLiGetBufferRenderTexture__FUiT117_GX2SurfaceFormatT1(width, width, 1, useMipmap ? levels : 1);
}

int FFLiGetCompressBufferSizeFacelineTexture__FUib(uint32_t resolution, bool mipmap)
{
    const uint32_t width = resolution * 2;
    const uint32_t levels = mipmap ? FFLiGetMipMapNum(width, width) : 1;
    return FFLiGetBufferRenderTexture__FUiT117_GX2SurfaceFormatT1(width, width, 0x1a, levels);
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
        textures, object, reinterpret_cast<const FFLShaderCallback*>(callback),
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
        reinterpret_cast<const FFLShaderCallback*>(callback), copySurface, compressor);
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
    FFLiSetupRenderTexture__FP17FFLiRenderTexturePC8FFLColorPvUi17_GX2SurfaceFormatPC18FFLiShaderCallback(
        texture, clearColor, depthBuffer, mipLevel, format, callback);
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
                               unsigned int format)
{
    return FFLiGetBufferRenderTexture__FUiT117_GX2SurfaceFormatT1(width, height, format, 1);
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
