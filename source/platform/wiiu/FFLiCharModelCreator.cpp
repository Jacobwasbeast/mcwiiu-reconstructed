#include "platform/wiiu/FFLiCharModelCreator.hpp"
#include "platform/wiiu/FFLiBufferAllocator.hpp"
#include "platform/wiiu/FFLiCharModelCreateParam.hpp"
#include "platform/wiiu/FFLiManager.hpp"
#include "platform/wiiu/FFLiShaderCallback.hpp"
#include "platform/wiiu/FFLiCoordinate.hpp"
#include "platform/wiiu/FFLiResourceLoader.hpp"
#include "platform/wiiu/FFLiResourceLoaderBuffer.hpp"
#include "platform/wiiu/FFLiResourceManager.hpp"
#include "platform/wiiu/FFLiCompressor.hpp"
#include <cstring>

struct FFLiRenderTextureBuffer
{
    void* buffer;
    unsigned int size;
};

struct FFLCharModelDesc
{
    unsigned int resolution;
    unsigned int mask;
    unsigned int reserved;
    unsigned int resourceType;
    bool useMipmap;
};

enum FFLDataSource
{
    FFL_DATA_SOURCE_OFFICIAL = 0,
    FFL_DATA_SOURCE_DEFAULT = 1,
    FFL_DATA_SOURCE_MIDDLE_DB = 2,
    FFL_DATA_SOURCE_STORE_DATA_OFFICIAL = 3,
    FFL_DATA_SOURCE_STORE_DATA = 4,
    FFL_DATA_SOURCE_BUFFER = 5,
    FFL_DATA_SOURCE_DIRECT_POINTER = 6
};

struct FFLCharModelSource
{
    FFLDataSource source;
    const void* data;
    unsigned short index;
};

struct FFLCharModelBuffer
{
    void* vertexBuffer;
    unsigned int vertexSize;
    void* textureBuffer;
    unsigned int textureSize;
};

struct FFLiCharModel
{
    unsigned char data[0x7d4];
};

struct FFLiCharInfo;
class FFLiDatabaseManager
{
public:
    int PickupCharInfo(FFLiCharInfo*, FFLDataSource, const void*, unsigned short);
};

struct _Mat44
{
    float value[4][4];
};

extern int FFLiInitMaskTextures(void*, unsigned int, unsigned int, bool, bool, FFLiBufferAllocator*);
extern int FFLiInitTempObjectMaskTextures(void*, const void*, const void*, unsigned int, unsigned int, bool, bool, unsigned int, void*, FFLiBufferAllocator*, FFLiRenderTextureBuffer*);
extern void FFLiInitFacelineTexture(void*, unsigned int, bool, bool, FFLiBufferAllocator*);
extern int FFLiInitTempObjectFacelineTexture(void*, void*, const void*, unsigned int, bool, bool, unsigned int, void*, FFLiBufferAllocator*, FFLiRenderTextureBuffer*);
extern int FFLiInitCharModelParts(FFLiCharModel*, void*, FFLiBufferAllocator*, const FFLiCoordinate*);
extern int FFLiInitCharModelTextures(FFLiCharModel*, void*, FFLiBufferAllocator*);
extern void FFLiAdjustPartsTransform(FFLiCharModel*, const FFLiCoordinate*);
extern void FFLiSetupDrawParam(FFLiCharModel*);
extern void FFLiInvalidateShape(void*);
extern void FFLiInvalidateTextures(FFLiCharModel*);
extern void FFLiTempSetContextState(const void*);
extern void ASM_MTX44Identity(void*);
extern void FFLiRenderMaskTextures(void*, void*, const FFLiShaderCallback*, void*, void*);
extern void FFLiRenderFacelineTexture(void*, const void*, unsigned int, void*, const FFLiShaderCallback*, void*, void*);
extern unsigned int ModelFlagToModelType(unsigned int);

FFLiCharModelCreator::FFLiCharModelCreator(FFLiCharModelCreateParam* createParam, FFLiManager* manager)
    : m_createParam(createParam), m_manager(manager)
{
}
FFLiCharModelCreator::~FFLiCharModelCreator()
{
}

void FFLiCharModelCreator::SetupRenderTextureBuffer(FFLiRenderTextureBuffer& buffer, const FFLCharModelDesc* description, FFLiBufferAllocator* allocator) const
{
    buffer.buffer = 0;
    buffer.size = 0;
    if (description->useMipmap)
    {
        buffer.size = m_createParam->GetCompressBufferSize(description);
        buffer.buffer = (void*)allocator->Allocate(buffer.size);
    }
}

int FFLiCharModelCreator::ExecuteCPUStep(FFLiCharModel* model, const FFLCharModelSource* source, const FFLCharModelDesc* description, FFLCharModelBuffer* buffer)
{
    if (!FFLiCharModelCreateParam::CheckModelDesc(description, *(void**)((unsigned char*)m_manager + 0x2964) != 0))
        return 1;
    std::memset(model, 0, 0x7dc);
    std::memcpy((unsigned char*)model + 0x11c, description, 20);
    std::memcpy((unsigned char*)model + 0x134, buffer, 8);
    *(unsigned int*)((unsigned char*)model + 0x790) = ModelFlagToModelType(*(unsigned int*)((unsigned char*)model + 0x128));
    FFLiDatabaseManager* database = *(FFLiDatabaseManager**)m_createParam;
    int result = database->PickupCharInfo((FFLiCharInfo*)model, source->source, source->data, source->index);
    if (result != 0)
        return result;
    FFLiBufferAllocator temporaryBuffer;
    FFLiBufferAllocator renderBuffer;
    temporaryBuffer.Init(buffer->vertexBuffer, buffer->vertexSize);
    renderBuffer.Init(buffer->textureBuffer, buffer->textureSize);
    FFLiResourceManager* resourceManager = *(FFLiResourceManager**)((unsigned char*)m_manager + 4);
    if (!resourceManager->IsValid((FFLResourceType)description->resourceType))
        return 1;
    FFLiResourceLoaderBuffer resourceBuffer(resourceManager, &renderBuffer, (FFLResourceType)description->resourceType);
    FFLiResourceLoader resourceLoader(resourceManager, &resourceBuffer, (FFLResourceType)description->resourceType);
    *(void**)((unsigned char*)model + 0x140) = (void*)renderBuffer.Allocate(0x220, 4);
    FFLiRenderTextureBuffer renderTexture;
    SetupRenderTextureBuffer(renderTexture, description, &renderBuffer);
    if (FFLiInitMaskTextures((unsigned char*)model + 0x6cc, description->mask, FFLiCharModelCreateParam::GetResolution(description->resolution), FFLiCharModelCreateParam::IsEnabledMipMap(description->resolution), description->useMipmap, &temporaryBuffer) != 0)
        return 1;
    unsigned int useUB = 0;
    FFLiCompressor* compressor = *(FFLiCompressor**)((unsigned char*)m_manager + 0x2964);
    if (compressor != 0)
        compressor->UseUB();
    if (FFLiInitTempObjectMaskTextures(*(void**)((unsigned char*)model + 0x140), (unsigned char*)model + 0x6cc, model, description->mask, FFLiCharModelCreateParam::GetResolution(description->resolution), FFLiCharModelCreateParam::IsEnabledMipMap(description->resolution), description->useMipmap, useUB, &resourceLoader, &renderBuffer, &renderTexture) != 0)
        return 1;
    FFLiInitFacelineTexture((unsigned char*)model + 0x624, FFLiCharModelCreateParam::GetResolution(description->resolution), FFLiCharModelCreateParam::IsEnabledMipMap(description->resolution), description->useMipmap, &temporaryBuffer);
    if (FFLiInitTempObjectFacelineTexture(*(void**)((unsigned char*)model + 0x140) + 0xd4, (unsigned char*)model + 0x624, model, FFLiCharModelCreateParam::GetResolution(description->resolution), FFLiCharModelCreateParam::IsEnabledMipMap(description->resolution), description->useMipmap, useUB, &resourceLoader, &renderBuffer, &renderTexture) != 0)
        return 1;
    FFLiCoordinate* coordinate = *(FFLiCoordinate**)((unsigned char*)m_createParam + 0xc);
    if (FFLiInitCharModelParts(model, &resourceLoader, &temporaryBuffer, coordinate) != 0)
        return 1;
    if (FFLiInitCharModelTextures(model, &resourceLoader, &temporaryBuffer) != 0)
        return 1;
    FFLiAdjustPartsTransform(model, coordinate);
    FFLiSetupDrawParam(model);
    return 0;
}

void FFLiCharModelCreator::InvalidateShapes(FFLiCharModel* model)
{
    unsigned int index;
    for (index = 0; index < 12; ++index)
        FFLiInvalidateShape((unsigned char*)model + 0x144 + index * 0x68);
}

void FFLiCharModelCreator::BeforeExecuteGPUStep(FFLiCharModel*)
{
}

void FFLiCharModelCreator::AfterExecuteGPUStep(FFLiCharModel* model)
{
    InvalidateShapes(model);
    FFLiInvalidateTextures(model);
    FFLiTempSetContextState(0);
}

void FFLiCharModelCreator::ExecuteGPUStep(FFLiCharModel* model, const FFLShaderCallback* callback)
{
    unsigned int resolution = FFLiCharModelCreateParam::GetResolution(*(unsigned int*)((unsigned char*)model + 0x120));
    FFLiCharModelCreateParam::IsEnabledMipMap(*(unsigned int*)((unsigned char*)model + 0x120));
    FFLiShaderCallback shader;
    _Mat44 matrix;
    shader.Set(callback);
    shader.CallSetContextState();
    ASM_MTX44Identity(&matrix);
    shader.CallSetMatrix(matrix);
    void* compressor = *(void**)((unsigned char*)m_manager + 0x2964);
    void* copySurface = (unsigned char*)m_manager + 0x2968;
    FFLiRenderMaskTextures((unsigned char*)model + 0x6cc, *(void**)((unsigned char*)model + 0x140), &shader, copySurface, (unsigned char*)compressor + 0xf8);
    FFLiRenderFacelineTexture((unsigned char*)model + 0x624, model, resolution, *(void**)((unsigned char*)model + 0x140) + 0xd4, &shader, copySurface, compressor);
    AfterExecuteGPUStep(model);
    *(void**)((unsigned char*)model + 0x140) = 0;
}

