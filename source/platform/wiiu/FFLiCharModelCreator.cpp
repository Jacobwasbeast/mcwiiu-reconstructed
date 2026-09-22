#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiCharModelCreateParam.hpp"
#include "platform/wiiu/FFLiCompressor.hpp"
#include "platform/wiiu/FFLiCoordinate.hpp"
#include "platform/wiiu/FFLiManager.hpp"
#include "platform/wiiu/FFLiResourceLoader.hpp"
#include "platform/wiiu/FFLiResourceLoaderBuffer.hpp"
#include "platform/wiiu/FFLiShaderCallback.hpp"
#include "platform/wiiu/FFLiTypes.hpp"

#include <cstring>

unsigned int FFLiCharModel::GetExpressionMask() const
{
    return description.expressionFlag;
}

int FFLiCharModel::GetFavoriteColorIndex() const
{
    return static_cast<int>(info.data[0x08]);
}

int FFLiLoadShape(FFLDrawParam*, FFLBoundingBox*, FFLiCharModel*, FFLiShapePartsType, unsigned short, FFLiResourceLoader*, FFLiBufferAllocator*);
void FFLiAdjustShape(FFLDrawParam*, FFLBoundingBox*, float, float, const FFLVec3*, bool, const FFLiCoordinate*, FFLiShapePartsType, bool);
int FFLiCanDrawShape(const FFLDrawParam*);
int FFLiLoadTextureWithAllocate(_GX2Texture**, FFLiTexturePartsType, unsigned int, FFLiResourceLoader*, FFLiBufferAllocator*);
void FFLiInvalidateTexture(_GX2Texture*);
unsigned int FFLiInitMaskTextures(FFLiMaskTextures*, unsigned int, unsigned int, bool, bool, FFLiBufferAllocator*);
int FFLiInitTempObjectMaskTextures(FFLiMaskTexturesTempObject*, const FFLiMaskTextures*, const FFLiCharInfo*, unsigned int, unsigned int, bool, bool, bool, FFLiResourceLoader*, FFLiBufferAllocator*, FFLiRenderTextureBuffer*);
void FFLiInitFacelineTexture(FFLiRenderTexture*, unsigned int, bool, bool, FFLiBufferAllocator*);
int FFLiInitTempObjectFacelineTexture(FFLiFacelineTextureTempObject*, FFLiRenderTexture*, const FFLiCharInfo*, unsigned int, bool, bool, bool, FFLiResourceLoader*, FFLiBufferAllocator*, FFLiRenderTextureBuffer*);
void FFLiInvalidateShape(FFLDrawParam*);
void FFLiTempSetContextState(const _GX2ContextState*);
void ASM_MTX44Identity(float (*)[4]);
void FFLiRenderMaskTextures(FFLiMaskTextures*, FFLiMaskTexturesTempObject*, const FFLiShaderCallback*, FFLiCopySurface*, FFLiCompressorImpl<FFLiCompressorDescBC3>*);
void FFLiRenderFacelineTexture(FFLiRenderTexture*, const FFLiCharInfo*, unsigned int, FFLiFacelineTextureTempObject*, const FFLiShaderCallback*, FFLiCopySurface*, FFLiCompressorImpl<FFLiCompressorDescBC1>*);
void FFLiInitModulateShapeFaceline(FFLModulateParam*, const _GX2Texture&);
void FFLiInitModulateShapeBeard(FFLModulateParam*, int);
void FFLiInitModulateShapeNose(FFLModulateParam*, int);
void FFLiInitModulateShapeForehead(FFLModulateParam*, int);
void FFLiInitModulateShapeHair(FFLModulateParam*, int);
void FFLiInitModulateShapeCap(FFLModulateParam*, int, const _GX2Texture&);
void FFLiInitModulateShapeMask(FFLModulateParam*, const _GX2Texture&);
void FFLiInitModulateShapeNoseline(FFLModulateParam*, const _GX2Texture&);
void FFLiInitModulateShapeGlass(FFLModulateParam*, int, const _GX2Texture&);

unsigned int FFLiCharModelCreatorSupport::ModelFlagToModelType(unsigned int flags)
{
    for (unsigned int flagIndex = 0; flagIndex < 3; ++flagIndex)
    {
        if ((flags & (1U << flagIndex)) != 0)
        {
            return flagIndex;
        }
    }

    return 0;
}

unsigned char FFLiCharModelCreatorSupport::ConvertShapePartsTypeToShapeType(FFLiShapePartsType parts)
{
    static const unsigned char types[12] = { 0, 7, 11, 1, 8, 4, 5, 6, 2, 9, 3, 10 };
    return parts < 12 ? types[parts] : 12;
}

void FFLiCharModelCreatorSupport::UpdateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* sourceBounds)
{
    if (sourceBounds->minX != sourceBounds->minX)
    {
        return;
    }

    if (destination->minX != destination->minX)
    {
        *destination = *sourceBounds;
        return;
    }

    destination->minX = destination->minX < sourceBounds->minX ? destination->minX : sourceBounds->minX;
    destination->minY = destination->minY < sourceBounds->minY ? destination->minY : sourceBounds->minY;
    destination->minZ = destination->minZ < sourceBounds->minZ ? destination->minZ : sourceBounds->minZ;
    destination->maxX = destination->maxX > sourceBounds->maxX ? destination->maxX : sourceBounds->maxX;
    destination->maxY = destination->maxY > sourceBounds->maxY ? destination->maxY : sourceBounds->maxY;
    destination->maxZ = destination->maxZ > sourceBounds->maxZ ? destination->maxZ : sourceBounds->maxZ;
}

void FFLiCharModelCreatorSupport::CalcluateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source, FFLiShapePartsType parts)
{
    static const unsigned char enabled[36] = {
        1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1,
        1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0
    };

    for (unsigned int boundsIndex = 0; boundsIndex < 3; ++boundsIndex)
    {
        if (enabled[parts * 3 + boundsIndex] != 0)
        {
            UpdateBoundingBox(destination + boundsIndex, source);
        }
    }
}

int FFLiCharModelCreatorSupport::InitShape(FFLiCharModel* model, FFLiShapePartsType parts, unsigned int index, float scaleX, float scaleY, const FFLVec3* offset, bool flip, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate)
{
    FFLDrawParam* drawParam = &model->shapes[ConvertShapePartsTypeToShapeType(parts)];
    FFLBoundingBox boundingBox;
    int result = FFLiLoadShape(drawParam, &boundingBox, model, parts, static_cast<unsigned short>(index), loader, allocator);
    if (result == 0)
    {
        FFLiAdjustShape(drawParam, &boundingBox, scaleX, scaleY, offset, flip, coordinate, parts,
                        (model->description.modelFlag & 8U) != 0);
        CalcluateBoundingBox(model->boundingBoxes, &boundingBox, parts);
    }

    return result;
}

int FFLiCharModelCreatorSupport::InitShapes(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate)
{
    int result = InitShape(model, static_cast<FFLiShapePartsType>(FFLiShapePartsType_0 + 3), model->charInfo.facelineType, 1.0f, 1.0f, 0, false, loader, allocator, coordinate);
    if (result != 0)
    {
        return result;
    }

    const unsigned int modelFlags = model->description.modelFlag & 7U;
    if ((modelFlags & 3U) != 0)
    {
        static const unsigned int parts[6] = { 8, 1, 10, 9, 2, 11 };
        for (unsigned int part = 0; part < 6; ++part)
        {
            if ((modelFlags & (1U << (part / 3U))) != 0)
            {
                result = InitShape(model, static_cast<FFLiShapePartsType>(parts[part]),
                                   model->charInfo.hairType, 1.0f, 1.0f, &model->commonOffset,
                                   model->charInfo.hairFlip != 0, loader, allocator, coordinate);
                if (result != 0)
                {
                    return result;
                }
            }
        }
    }

    if (model->charInfo.beardType < 4)
    {
        result = InitShape(model, static_cast<FFLiShapePartsType>(FFLiShapePartsType_0), model->charInfo.beardType,
                           1.0f, 1.0f, &model->faceOffset, false, loader, allocator, coordinate);
        if (result != 0)
        {
            return result;
        }
    }

    FFLVec3 offset = model->transformedParts[0];
    float scale = static_cast<int>(model->charInfo.noseScale) * 0.175f + 0.4f;
    offset.y += (static_cast<int>(model->charInfo.noseY) - 8) * -1.5f;
    result = InitShape(model, static_cast<FFLiShapePartsType>(FFLiShapePartsType_0 + 7), model->charInfo.noseType,
                       scale, scale, &offset, false, loader, allocator, coordinate);
    if (result != 0)
    {
        return result;
    }

    result = InitShape(model, static_cast<FFLiShapePartsType>(FFLiShapePartsType_0 + 6), model->charInfo.noseType,
                       scale, scale, &offset, false, loader, allocator, coordinate);
    if (result != 0)
    {
        return result;
    }

    result = InitShape(model, static_cast<FFLiShapePartsType>(FFLiShapePartsType_0 + 5), model->charInfo.facelineType,
                       1.0f, 1.0f, 0, false, loader, allocator, coordinate);
    if (result != 0)
    {
        return result;
    }

    if (model->charInfo.glassType != 0)
    {
        offset = model->transformedParts[0];
        scale = static_cast<int>(model->charInfo.glassScale) * 0.15f + 0.4f;
        offset.y += (static_cast<int>(model->charInfo.glassY) - 11) * -1.5f + 5.0f;
        offset.z += 2.0f;
        result = InitShape(model, static_cast<FFLiShapePartsType>(FFLiShapePartsType_0 + 4), 0, scale, scale, &offset,
                           false, loader, allocator, coordinate);
    }

    return result;
}

int FFLiCharModelCreatorSupport::InitTextures(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator)
{
    int result = 0;
    if (FFLiCanDrawShape(&model->shapes[7]) || FFLiCanDrawShape(&model->shapes[11]))
    {
        result = FFLiLoadTextureWithAllocate(&model->facelineShapeTexture,
                                             static_cast<FFLiTexturePartsType>(FFLiTexturePartsType_0 + 1),
                                             model->charInfo.hairType, loader, allocator);
    }
    if (result != 0)
    {
        return result;
    }

    if (FFLiCanDrawShape(&model->shapes[5]))
    {
        result = FFLiLoadTextureWithAllocate(&model->noselineTexture,
                                             static_cast<FFLiTexturePartsType>(FFLiTexturePartsType_0 + 10),
                                             model->charInfo.noseType, loader, allocator);
    }
    if (result == 0 && model->charInfo.glassType != 0)
    {
        result = FFLiLoadTextureWithAllocate(&model->capTexture,
                                             static_cast<FFLiTexturePartsType>(FFLiTexturePartsType_0 + 6),
                                             model->charInfo.glassType, loader, allocator);
    }

    return result;
}

void FFLiCharModelCreatorSupport::AddVec3(FFLVec3* destination, FFLVec3 value)
{
    destination->x += value.x;
    destination->y += value.y;
    destination->z += value.z;
}

void FFLiCharModelCreatorSupport::AdjustPartsTransform(FFLiCharModel* model, const FFLiCoordinate* coordinate)
{
    AddVec3(&model->transformedParts[3], model->commonOffset);
    AddVec3(&model->transformedParts[5], model->commonOffset);
    AddVec3(&model->transformedParts[7], model->commonOffset);
    coordinate->Transform(&model->transformedParts[1]);
    coordinate->TransformWithoutScale(&model->transformedParts[2]);
    coordinate->Transform(&model->transformedParts[3]);
    coordinate->TransformWithoutScale(&model->transformedParts[4]);
    coordinate->Transform(&model->transformedParts[5]);
    coordinate->TransformWithoutScale(&model->transformedParts[6]);
    coordinate->Transform(&model->transformedParts[7]);
}

const unsigned int* FFLiCharModelCreatorSupport::GetShapeTypeInfo(FFLModelType type)
{
    static const unsigned int normal[4] = { 2, 3, 7, 9 };
    static const unsigned int special[4] = { 9, 10, 11, 0 };
    return type == FFLModelTypeSpecial ? special : normal;
}

void FFLiCharModelCreatorSupport::SetupDrawParam(FFLiCharModel* model)
{
    model->shapes[1].flags = 1;
    FFLiInitModulateShapeFaceline(&model->shapes[1].modulate, model->facelineTexture.texture);
    model->shapes[0].flags = 1;
    FFLiInitModulateShapeBeard(&model->shapes[0].modulate, model->charInfo.beardColor);
    model->shapes[6].flags = 1;
    FFLiInitModulateShapeNose(&model->shapes[6].modulate, model->charInfo.facelineColor);
    const unsigned int drawMode = model->charInfo.hairFlip != 0 ? 2 : 1;
    for (unsigned int index = 0; index < 2; ++index)
    {
        if ((model->description.modelFlag & (1U << index)) != 0)
        {
            const unsigned int* shapeTypes = GetShapeTypeInfo(static_cast<FFLModelType>(index));
            model->shapes[shapeTypes[1]].flags = drawMode;
            FFLiInitModulateShapeForehead(&model->shapes[shapeTypes[1]].modulate,
                                          model->charInfo.facelineColor);
            model->shapes[shapeTypes[0]].flags = drawMode;
            FFLiInitModulateShapeHair(&model->shapes[shapeTypes[0]].modulate,
                                      model->charInfo.hairColor);
            if (model->facelineShapeTexture != 0)
            {
                model->shapes[shapeTypes[2]].flags = drawMode;
                FFLiInitModulateShapeCap(&model->shapes[shapeTypes[2]].modulate,
                                         model->charInfo.favoriteColor,
                                         *model->facelineShapeTexture);
            }
        }
    }

    if (model->maskTextures.textures[model->firstExpression] != 0)
    {
        model->shapes[8].flags = 1;
        FFLiInitModulateShapeMask(&model->shapes[8].modulate,
                                  model->maskTextures.textures[model->firstExpression]->texture);
    }
    if (model->noselineTexture != 0)
    {
        model->shapes[9].flags = 1;
        FFLiInitModulateShapeNoseline(&model->shapes[9].modulate, *model->noselineTexture);
    }
    if (model->capTexture != 0)
    {
        model->shapes[11].flags = 0;
        FFLiInitModulateShapeGlass(&model->shapes[11].modulate, model->charInfo.glassColor,
                                   *model->capTexture);
    }
}

void FFLiCharModelCreatorSupport::InvalidateTextures(FFLiCharModel* model)
{
    if (model->facelineShapeTexture != 0)
    {
        FFLiInvalidateTexture(model->facelineShapeTexture);
    }
    if (model->noselineTexture != 0)
    {
        FFLiInvalidateTexture(model->noselineTexture);
    }
    if (model->capTexture != 0)
    {
        FFLiInvalidateTexture(model->capTexture);
    }
}

FFLiCharModelCreator::FFLiCharModelCreator(FFLiCharModelCreateParam* createParam, FFLiManager* manager)
    : m_createParam(createParam), m_manager(manager)
{
}

FFLiCharModelCreator::~FFLiCharModelCreator()
{
}

void FFLiCharModelCreator::SetupRenderTextureBuffer(FFLiRenderTextureBuffer& buffer, const FFLCharModelDesc* description, FFLiBufferAllocator* allocator) const
{
    std::memset(&buffer, 0, sizeof(buffer));
    if (description->useMipmap)
    {
        buffer.size = static_cast<unsigned int>(m_createParam->GetCompressBufferSize(description));
        buffer.buffer = (void*)allocator->Allocate(buffer.size);
    }
}

int FFLiCharModelCreator::ExecuteCPUStep(FFLiCharModel* model, const FFLCharModelSource* source, const FFLCharModelDesc* description, FFLCharModelBuffer* buffer)
{
    if (!FFLiCharModelCreateParam::CheckModelDesc(description, m_manager->m_compressor != 0))
    {
        return 1;
    }

    std::memset(model, 0, sizeof(*model));
    model->description = *description;
    model->modelBuffer = buffer->modelBuffer;
    model->modelBufferSize = buffer->modelBufferSize;
    model->modelType = static_cast<FFLModelType>(
        FFLiCharModelCreatorSupport::ModelFlagToModelType(description->modelFlag));

    int result = m_createParam->m_databaseManager->PickupCharInfo(
        &model->info, source->source, source->data, source->index);
    if (result != 0)
    {
        return result;
    }

    const unsigned int resolution =
        FFLiCharModelCreateParam::GetResolution(description->resolution);
    const bool mipmap =
        FFLiCharModelCreateParam::IsEnabledMipMap(description->resolution) != 0;

    FFLiBufferAllocator modelAllocator;
    FFLiBufferAllocator temporaryAllocator;
    modelAllocator.Init(buffer->modelBuffer, buffer->modelBufferSize);
    temporaryAllocator.Init(buffer->temporaryBuffer, buffer->temporaryBufferSize);

    if (!m_createParam->m_resourceManager->IsValid(description->resourceType))
    {
        return 1;
    }

    FFLiResourceLoaderBuffer resourceBuffer(m_createParam->m_resourceManager,
                                             &temporaryAllocator,
                                             description->resourceType);
    FFLiResourceLoader resourceLoader(m_createParam->m_resourceManager,
                                      &resourceBuffer,
                                      description->resourceType);
    model->tempObject = (FFLiCharModelTempObject*)temporaryAllocator.Allocate(
        sizeof(FFLiCharModelTempObject), 4);

    FFLiRenderTextureBuffer renderTextureBuffer;
    SetupRenderTextureBuffer(renderTextureBuffer, description, &temporaryAllocator);

    model->firstExpression = FFLiInitMaskTextures(&model->maskTextures,
                                                  description->expressionFlag,
                                                  resolution,
                                                  mipmap,
                                                  description->useMipmap,
                                                  &modelAllocator);
    const bool useUB =
        m_manager->m_compressor != 0 && m_manager->m_compressor->UseUB();

    result = FFLiInitTempObjectMaskTextures(&model->tempObject->maskTextures,
                                            &model->maskTextures,
                                            &model->info,
                                            description->expressionFlag,
                                            resolution,
                                            mipmap,
                                            description->useMipmap,
                                            useUB,
                                            &resourceLoader,
                                            &temporaryAllocator,
                                            &renderTextureBuffer);
    if (result != 0)
    {
        return result;
    }

    FFLiInitFacelineTexture(&model->facelineTexture,
                            resolution,
                            mipmap,
                            description->useMipmap,
                            &modelAllocator);
    result = FFLiInitTempObjectFacelineTexture(&model->tempObject->facelineTexture,
                                               &model->facelineTexture,
                                               &model->info,
                                               resolution,
                                               mipmap,
                                               description->useMipmap,
                                               useUB,
                                               &resourceLoader,
                                               &temporaryAllocator,
                                               &renderTextureBuffer);
    if (result != 0)
    {
        return result;
    }

    result = FFLiCharModelCreatorSupport::InitShapes(
        model, &resourceLoader, &modelAllocator, &m_createParam->m_coordinate);
    if (result != 0)
    {
        return result;
    }

    result = FFLiCharModelCreatorSupport::InitTextures(
        model, &resourceLoader, &modelAllocator);
    if (result != 0)
    {
        return result;
    }

    FFLiCharModelCreatorSupport::AdjustPartsTransform(
        model, &m_createParam->m_coordinate);
    FFLiCharModelCreatorSupport::SetupDrawParam(model);
    return 0;
}

void FFLiCharModelCreator::InvalidateShapes(FFLiCharModel* model)
{
    for (unsigned int index = 0; index < 12; ++index)
    {
        FFLiInvalidateShape(&model->shapes[index]);
    }
}

void FFLiCharModelCreator::AfterExecuteGPUStep(FFLiCharModel* model)
{
    InvalidateShapes(model);
    FFLiCharModelCreatorSupport::InvalidateTextures(model);
    FFLiTempSetContextState(0);
}

void FFLiCharModelCreator::ExecuteGPUStep(FFLiCharModel* model, const FFLShaderCallback* callback)
{
    const unsigned int resolution =
        FFLiCharModelCreateParam::GetResolution(model->description.resolution);
    FFLiCharModelCreateParam::IsEnabledMipMap(model->description.resolution);

    FFLiShaderCallback shader;
    shader.Set(callback);
    shader.CallSetContextState();

    _Mat44 matrix;
    ASM_MTX44Identity(matrix.value);
    shader.CallSetMatrix(matrix);

    FFLiRenderMaskTextures(&model->maskTextures,
                           &model->tempObject->maskTextures,
                           &shader,
                           &m_manager->m_copySurface,
                           &m_manager->m_compressor->mCompressorBC3);
    FFLiRenderFacelineTexture(&model->facelineTexture,
                              &model->info,
                              resolution,
                              &model->tempObject->facelineTexture,
                              &shader,
                              &m_manager->m_copySurface,
                              m_manager->m_compressor);
    AfterExecuteGPUStep(model);
    model->tempObject = 0;
}
