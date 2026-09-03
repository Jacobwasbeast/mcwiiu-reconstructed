#include "vendor/__N_24_FFLiCharModelCreator_cpp_1f5483f2.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiResourceLoader.hpp"

extern int FFLiLoadShape(FFLDrawParam*, FFLBoundingBox*, FFLiCharModel*, FFLiShapePartsType, unsigned short, FFLiResourceLoader*, FFLiBufferAllocator*);
extern void FFLiAdjustShape(FFLDrawParam*, FFLBoundingBox*, float, float, float, const FFLVec3*, bool, const FFLiCoordinate*, FFLiShapePartsType, bool);
extern int FFLiCanDrawShape(const FFLDrawParam*);
extern int FFLiLoadTextureWithAllocate(_GX2Texture**, FFLiTexturePartsType, unsigned int, FFLiResourceLoader*, FFLiBufferAllocator*);
extern void FFLiInvalidateTexture(_GX2Texture*);
extern void FFLiInitModulateShapeFaceline(void*, const _GX2Texture&);
extern void FFLiInitModulateShapeBeard(void*, int);
extern void FFLiInitModulateShapeNose(void*, int);
extern void FFLiInitModulateShapeForehead(void*, int);
extern void FFLiInitModulateShapeHair(void*, int);
extern void FFLiInitModulateShapeCap(void*, int, const _GX2Texture&);
extern void FFLiInitModulateShapeMask(void*, const _GX2Texture&);
extern void FFLiInitModulateShapeNoseline(void*, const _GX2Texture&);
extern void FFLiInitModulateShapeGlass(void*, int, const _GX2Texture&);

uint32_t __N_24_FFLiCharModelCreator_cpp_1f5483f2::ModelFlagToModelType(unsigned int flags)
{
    for (uint32_t index = 0; index < 3; ++index)
        if ((flags & (1u << index)) != 0) return index;
    return 0;
}

uint8_t __N_24_FFLiCharModelCreator_cpp_1f5483f2::ConvertShapePartsTypeToShapeType(FFLiShapePartsType parts)
{
    static const uint8_t types[12] = { 0, 7, 11, 1, 8, 4, 5, 6, 2, 9, 3, 10 };
    return parts < 12 ? types[parts] : 12;
}

void __N_24_FFLiCharModelCreator_cpp_1f5483f2::UpdateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source)
{
    if (source->minX == 0.0f && source->minY == 0.0f && source->minZ == 0.0f && source->maxX == 0.0f && source->maxY == 0.0f && source->maxZ == 0.0f) return;
    if (destination->minX > destination->maxX) { *destination = *source; return; }
    if (destination->minX > source->minX) destination->minX = source->minX;
    if (destination->minY > source->minY) destination->minY = source->minY;
    if (destination->minZ > source->minZ) destination->minZ = source->minZ;
    if (destination->maxX < source->maxX) destination->maxX = source->maxX;
    if (destination->maxY < source->maxY) destination->maxY = source->maxY;
    if (destination->maxZ < source->maxZ) destination->maxZ = source->maxZ;
}

void __N_24_FFLiCharModelCreator_cpp_1f5483f2::CalcluateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source, FFLiShapePartsType parts)
{
    static const uint8_t use[12] = {1,1,1,1,0,0,1,0,1,1,1,0};
    for (uint32_t index = 0; index < 3; ++index) if (parts < 4 && use[parts * 3 + index]) UpdateBoundingBox(destination + index, source);
}

int __N_24_FFLiCharModelCreator_cpp_1f5483f2::InitShape(FFLiCharModel* model, FFLiShapePartsType parts, unsigned int index, float scaleX, float scaleY, const FFLVec3* offset, bool flip, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate)
{
    FFLiCharModelShape& shape = model->shapes[ConvertShapePartsTypeToShapeType(parts)];
    FFLBoundingBox bounds;
    int result = FFLiLoadShape(&shape.draw, &bounds, model, parts, (unsigned short)index, loader, allocator);
    if (result == 0) { FFLiAdjustShape(&shape.draw, &bounds, scaleX, scaleY, scaleY, offset, flip, coordinate, parts, (model->values[74] >> 3) & 1); CalcluateBoundingBox(model->bounds, &bounds, parts); }
    return result;
}

int __N_24_FFLiCharModelCreator_cpp_1f5483f2::InitShapes(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate)
{
    int result = InitShape(model, (FFLiShapePartsType)3, model->values[1], 1.0f, 1.0f, 0, false, loader, allocator, coordinate);
    if (result != 0) return result;
    if ((model->values[74] & 3) != 0)
    {
        FFLVec3 offset = model->center;
        result = InitShape(model, (FFLiShapePartsType)0, model->values[5], 1.0f, 1.0f, &offset, false, loader, allocator, coordinate);
        if (result != 0) return result;
    }
    result = InitShape(model, (FFLiShapePartsType)7, model->values[22], 0.4f, 0.15f, &model->extent, false, loader, allocator, coordinate);
    if (result != 0) return result;
    result = InitShape(model, (FFLiShapePartsType)6, model->values[22], 0.4f, 0.15f, &model->extent, false, loader, allocator, coordinate);
    if (result != 0) return result;
    return InitShape(model, (FFLiShapePartsType)5, model->values[1], 1.0f, 1.0f, 0, false, loader, allocator, coordinate);
}

int __N_24_FFLiCharModelCreator_cpp_1f5483f2::InitTextures(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator)
{
    int result = 0;
    if (FFLiCanDrawShape(&model->shapes[6].draw) || FFLiCanDrawShape(&model->shapes[8].draw)) result = FFLiLoadTextureWithAllocate(&model->facelineTexture, (FFLiTexturePartsType)1, model->values[5], loader, allocator);
    if (result != 0) return result;
    if (!FFLiCanDrawShape(&model->shapes[4].draw)) return model->values[35] ? FFLiLoadTextureWithAllocate(&model->capTexture, (FFLiTexturePartsType)6, model->values[35], loader, allocator) : 0;
    result = FFLiLoadTextureWithAllocate(&model->noselineTexture, (FFLiTexturePartsType)10, model->values[22], loader, allocator);
    return result == 0 && model->values[35] ? FFLiLoadTextureWithAllocate(&model->capTexture, (FFLiTexturePartsType)6, model->values[35], loader, allocator) : result;
}

void __N_24_FFLiCharModelCreator_cpp_1f5483f2::AddVec3(FFLVec3* destination, FFLVec3 value) { destination->x += value.x; destination->y += value.y; destination->z += value.z; }
void __N_24_FFLiCharModelCreator_cpp_1f5483f2::AdjustPartsTransform(FFLiCharModel* model, const FFLiCoordinate* coordinate)
{ FFLVec3 value = model->corner; AddVec3(&model->center, value); AddVec3(&model->extent, value); AddVec3(&model->corner, value); coordinate->Transform(&model->center); coordinate->TransformWithoutScale(&model->extent); coordinate->Transform(&model->corner); coordinate->TransformWithoutScale(&model->center); coordinate->Transform(&model->extent); coordinate->TransformWithoutScale(&model->corner); coordinate->Transform(&model->corner); }
const uint32_t* __N_24_FFLiCharModelCreator_cpp_1f5483f2::GetShapeTypeInfo(FFLModelType type)
{ static const uint32_t normal[4] = {2,3,7,9}; static const uint32_t special[4] = {9,10,11,0}; return type == FFLModelTypeSpecial ? special : normal; }
void __N_24_FFLiCharModelCreator_cpp_1f5483f2::SetupDrawParam(FFLiCharModel* model)
{
    FFLiInitModulateShapeFaceline(&model->values[122], *model->facelineTexture);
    FFLiInitModulateShapeBeard(&model->values[96], model->values[32]);
    FFLiInitModulateShapeNose(&model->values[252], model->values[2]);
    const uint32_t* info = GetShapeTypeInfo((FFLModelType)model->modelType);
    for (uint32_t index = 0; index < 2; ++index) if ((model->values[74] & (1u << index)) != 0) { FFLiInitModulateShapeForehead(&model->shapes[info[1]].values[0], model->values[2]); FFLiInitModulateShapeHair(&model->shapes[info[0]].values[0], model->values[6]); }
    if (model->capTexture) FFLiInitModulateShapeCap(&model->shapes[info[2]].values[0], model->values[59], *model->capTexture);
    if (model->noselineTexture) FFLiInitModulateShapeNoseline(&model->values[206], *model->noselineTexture);
}
void __N_24_FFLiCharModelCreator_cpp_1f5483f2::InvalidateTextures(FFLiCharModel* model)
{ if (model->facelineTexture) FFLiInvalidateTexture(model->facelineTexture); if (model->noselineTexture) FFLiInvalidateTexture(model->noselineTexture); if (model->capTexture) FFLiInvalidateTexture(model->capTexture); }
