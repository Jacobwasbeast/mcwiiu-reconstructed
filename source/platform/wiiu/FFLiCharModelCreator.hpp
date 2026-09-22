#ifndef PLATFORM_WIIU_FFLICHARMODELCREATOR_HPP
#define PLATFORM_WIIU_FFLICHARMODELCREATOR_HPP

#include "game/client/render/Renderer.hpp"
#include "platform/wiiu/FFLiCharModelCreateParam.hpp"
#include "platform/wiiu/FFLiCoordinate.hpp"
#include "platform/wiiu/FFLiResourceManager.hpp"
#include "platform/wiiu/FFLiTypes.hpp"
#include "platform/wiiu/FFLModelType.hpp"

class FFLiCharModelCreateParam;
class FFLiManager;
class FFLiBufferAllocator;
class FFLiResourceLoader;
class FFLiCompressor;
struct FFLShaderCallback;
struct FFLColor;

struct FFLCharModelDesc
{
    FFLResolution resolution;
    unsigned int expressionFlag;
    unsigned int modelFlag;
    FFLResourceType resourceType;
    unsigned int useMipmap;
};

struct FFLCharModelSource
{
    FFLDataSource source;
    const void* data;
    unsigned short index;
};

struct FFLCharModelBuffer
{
    void* modelBuffer;
    unsigned int modelBufferSize;
    void* temporaryBuffer;
    unsigned int temporaryBufferSize;
};

struct FFLiRenderTextureBuffer
{
    void* buffer;
    unsigned int size;
};

struct FFLAttributeBuffer
{
    unsigned int size;
    unsigned int stride;
    void* buffer;
};

struct FFLAttributeBufferParam
{
    FFLAttributeBuffer attributes[5];
};

struct FFLPrimitiveParam
{
    unsigned int primitiveType;
    unsigned int indexCount;
    unsigned int vertexCount;
    void* indexBuffer;
};

struct FFLModulateParam
{
    unsigned int mode;
    unsigned int type;
    const FFLColor* colorR;
    const FFLColor* colorG;
    const FFLColor* colorB;
    const _GX2Texture* texture;
};

struct FFLDrawParam
{
    FFLAttributeBufferParam attributes;
    FFLModulateParam modulate;
    unsigned int flags;
    FFLPrimitiveParam primitive;
};

struct FFLBoundingBox
{
    float minX;
    float minY;
    float minZ;
    float maxX;
    float maxY;
    float maxZ;
};

struct FFLiRenderTexture
{
    _GX2Texture texture;

    void CopyTo(_GX2Texture* destination);
    unsigned int GetSurfaceFormat() const;
};

struct FFLiMaskTextures
{
    FFLiRenderTexture* textures[19];
};

struct FFLiRawMaskDrawParam;
class FFLiCompressorParam;

struct FFLiPartsTextures
{
    _GX2Texture* textures[14];
};

struct FFLiMaskTexturesTempObject
{
    FFLiPartsTextures partsTextures;
    FFLiRawMaskDrawParam* rawMasks[19];
    FFLiRenderTexture* renderTexture;
    FFLiCompressorParam* compressorParams[19];
};

struct FFLiFacelineTextureElement
{
    _GX2Texture* texture;
    FFLDrawParam drawParam;
};

struct FFLiFacelineTextureTempObject
{
    FFLiFacelineTextureElement faceline;
    FFLiFacelineTextureElement make;
    FFLiFacelineTextureElement beard;
    FFLiRenderTexture* renderTexture;
    FFLiCompressorParam* compressorParam;
};

struct FFLiCharModelTempObject
{
    FFLiMaskTexturesTempObject maskTextures;
    FFLiFacelineTextureTempObject facelineTexture;
};

struct FFLiCharInfoData
{
    unsigned int version;
    unsigned int facelineType;
    unsigned int facelineColor;
    unsigned int facelineTexture;
    unsigned int facelineMake;
    unsigned int hairType;
    unsigned int hairColor;
    unsigned int hairFlip;
    unsigned int eye[7];
    unsigned int eyebrow[7];
    unsigned int noseType;
    unsigned int noseScale;
    unsigned int noseY;
    unsigned int mouth[5];
    unsigned int mustacheType;
    unsigned int beardType;
    unsigned int beardColor;
    unsigned int beardScale;
    unsigned int beardY;
    unsigned int glassType;
    unsigned int glassColor;
    unsigned int glassScale;
    unsigned int glassY;
    unsigned int mole[4];
    unsigned int personal[16];
    unsigned int favoriteColor;
    unsigned int additional[12];
};

struct FFLiCharModel
{
    union
    {
        FFLiCharInfo info;
        FFLiCharInfoData charInfo;
    };
    FFLCharModelDesc description;
    void* modelBuffer;
    unsigned int modelBufferSize;
    unsigned int firstExpression;
    FFLiCharModelTempObject* tempObject;
    FFLDrawParam shapes[12];
    FFLiRenderTexture facelineTexture;
    _GX2Texture* facelineShapeTexture;
    _GX2Texture* capTexture;
    _GX2Texture* noselineTexture;
    FFLiMaskTextures maskTextures;
    FFLVec3 faceOffset;
    FFLVec3 commonOffset;
    FFLVec3 transformedParts[8];
    FFLModelType modelType;
    FFLBoundingBox boundingBoxes[3];

    unsigned int GetExpressionMask() const;
    int GetFavoriteColorIndex() const;
};

class FFLiCharModelCreatorSupport
{
public:
    static unsigned int ModelFlagToModelType(unsigned int flags);
    static unsigned char ConvertShapePartsTypeToShapeType(FFLiShapePartsType parts);
    static void UpdateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source);
    static void CalcluateBoundingBox(FFLBoundingBox* destination, const FFLBoundingBox* source, FFLiShapePartsType parts);
    static int InitShape(FFLiCharModel* model, FFLiShapePartsType parts, unsigned int index, float scaleX, float scaleY, const FFLVec3* offset, bool flip, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate);
    static int InitShapes(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator, const FFLiCoordinate* coordinate);
    static int InitTextures(FFLiCharModel* model, FFLiResourceLoader* loader, FFLiBufferAllocator* allocator);
    static void AddVec3(FFLVec3* destination, FFLVec3 value);
    static void AdjustPartsTransform(FFLiCharModel* model, const FFLiCoordinate* coordinate);
    static const unsigned int* GetShapeTypeInfo(FFLModelType type);
    static void SetupDrawParam(FFLiCharModel* model);
    static void InvalidateTextures(FFLiCharModel* model);
};

class FFLiCharModelCreator
{
private:
    FFLiCharModelCreateParam* m_createParam;
    FFLiManager* m_manager;

public:
    FFLiCharModelCreator(FFLiCharModelCreateParam* createParam, FFLiManager* manager);
    ~FFLiCharModelCreator();
    void SetupRenderTextureBuffer(FFLiRenderTextureBuffer& buffer, const FFLCharModelDesc* description, FFLiBufferAllocator* allocator) const;
    int ExecuteCPUStep(FFLiCharModel* model, const FFLCharModelSource* source, const FFLCharModelDesc* description, FFLCharModelBuffer* buffer);
    void InvalidateShapes(FFLiCharModel* model);
    void BeforeExecuteGPUStep(FFLiCharModel* model);
    void AfterExecuteGPUStep(FFLiCharModel* model);
    void ExecuteGPUStep(FFLiCharModel* model, const FFLShaderCallback* callback);
};

#endif
