#ifndef SOURCE_VENDOR_N_24_FFLICHARMODELCREATOR_CPP_1F5483F2_HPP
#define SOURCE_VENDOR_N_24_FFLICHARMODELCREATOR_CPP_1F5483F2_HPP

#include <stdint.h>
#include "platform/wiiu/FFLiCoordinate.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiResourceManager.hpp"
#include "vendor/__N_17_FFLiCharModel_cpp_08c41fd9.hpp"

struct _GX2Texture { uint32_t value[8]; };
class FFLiResourceLoader;
struct FFLDrawParam { uint32_t value[26]; };
struct FFLBoundingBox { float minX, minY, minZ, maxX, maxY, maxZ; };
struct FFLiCharModelShape { FFLDrawParam draw; FFLBoundingBox bounds; uint32_t values[14]; };
struct FFLiCharModel
{
    uint32_t values[81];
    FFLiCharModelShape shapes[12];
    FFLBoundingBox bounds[3];
    FFLVec3 center;
    FFLVec3 extent;
    FFLVec3 corner;
    uint32_t modelType;
    _GX2Texture* facelineTexture;
    _GX2Texture* capTexture;
    _GX2Texture* noselineTexture;
};

class __N_24_FFLiCharModelCreator_cpp_1f5483f2
{
public:
    static uint32_t ModelFlagToModelType(unsigned int flags);
    static uint8_t ConvertShapePartsTypeToShapeType(FFLiShapePartsType parts);
    static void UpdateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source);
    static void CalcluateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source, FFLiShapePartsType parts);
    static int InitShape(FFLiCharModel* model, FFLiShapePartsType parts, unsigned int index, float scaleX, float scaleY, const FFLVec3* offset, bool flip, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate);
    static int InitShapes(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate);
    static int InitTextures(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator);
    static void AddVec3(FFLVec3* destination, FFLVec3 value);
    static void AdjustPartsTransform(FFLiCharModel* model, const FFLiCoordinate* coordinate);
    static const uint32_t* GetShapeTypeInfo(FFLModelType type);
    static void SetupDrawParam(FFLiCharModel* model);
    static void InvalidateTextures(FFLiCharModel* model);
};

#endif
