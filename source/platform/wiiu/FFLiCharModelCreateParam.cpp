#include "platform/wiiu/FFLiCharModelCreateParam.hpp"
#include "platform/wiiu/FFLiCharModelCreator.hpp"

#include "platform/wiiu/FFLiResourceManager.hpp"
#include "platform/wiiu/FFLiResourceUncompressBuffer.hpp"

extern int FFLiGetBufferSizeFacelineTexture(uint32_t resolution, bool mipmap, bool useMipmap);
extern int FFLiGetBufferSizeMaskTextures(uint32_t mask, uint32_t resolution, bool mipmap, bool useMipmap);
extern uint32_t FFLiGetTextureMaxSizeWithAlign(const FFLiResourceManager* resourceManager,
                                          FFLResourceType resourceType,
                                          FFLiTexturePartsType parts);
extern int FFLiGetCompressBufferSizeFacelineTexture(uint32_t resolution, bool mipmap);
extern int FFLiGetCompressBufferSizeMaskTexture(uint32_t resolution, bool mipmap);
extern int FFLiGetTempBufferSizeMaskTextures(uint32_t mask, uint32_t resolution, bool mipmap,
                                             bool useMipmap, const FFLiResourceManager* resourceManager,
                                             FFLResourceType resourceType);
extern int FFLiGetTempBufferSizeFacelineTexture(uint32_t resolution, bool mipmap, bool useMipmap,
                                                const FFLiResourceManager* resourceManager,
                                                FFLResourceType resourceType);

FFLiCharModelCreateParam::FFLiCharModelCreateParam(FFLiDatabaseManager* databaseManager,
                                                   FFLiResourceManager* resourceManager,
                                                   FFLiShaderCallback* shaderCallback,
                                                   bool useMipmap)
    : m_databaseManager(databaseManager), m_resourceManager(resourceManager),
      m_shaderCallback(shaderCallback), m_useMipmap(useMipmap)
{
}

FFLiCharModelCreateParam::~FFLiCharModelCreateParam()
{
}

uint32_t FFLiCharModelCreateParam::GetResolution(FFLResolution resolution)
{
    return static_cast<uint32_t>(resolution) & 0x3fffffffU;
}

uint32_t FFLiCharModelCreateParam::IsEnabledMipMap(FFLResolution resolution)
{
    return (static_cast<uint32_t>(resolution) >> 30) & 1U;
}

uint32_t FFLiCharModelCreateParam::CheckModelDesc(const FFLCharModelDesc* description, bool specialMii)
{
    if (description != 0 && description->resourceType < 2 &&
        (description->expressionFlag & 0x7ffffU) != 0 &&
        (description->modelFlag & 7U) != 0 &&
        (description->useMipmap == false || specialMii))
        return 1;
    return 0;
}

int32_t FFLiCharModelCreateParam::GetBufferSize(const FFLCharModelDesc* description) const
{
    if (!CheckModelDesc(description, m_useMipmap) ||
        !m_resourceManager->IsValid(static_cast<FFLResourceType>(description->resourceType)))
        return 0;

    const uint32_t resolution = GetResolution(description->resolution);
    const bool mipmap = IsEnabledMipMap(description->resolution) != 0;
    const FFLResourceType resourceType = static_cast<FFLResourceType>(description->resourceType);
    int size = 0;
    unsigned int part;
    for (part = 0; part < 12; ++part)
        size += m_resourceManager->GetShapeAlignedMaxSize(resourceType, static_cast<FFLiShapePartsType>(part));
    size += FFLiGetBufferSizeFacelineTexture(resolution, mipmap, description->useMipmap);
    size += FFLiGetBufferSizeMaskTextures(description->expressionFlag, resolution, mipmap, description->useMipmap);
    size += FFLiGetTextureMaxSizeWithAlign(m_resourceManager, resourceType, static_cast<FFLiTexturePartsType>(1));
    size += FFLiGetTextureMaxSizeWithAlign(m_resourceManager, resourceType, static_cast<FFLiTexturePartsType>(10));
    size += FFLiGetTextureMaxSizeWithAlign(m_resourceManager, resourceType, static_cast<FFLiTexturePartsType>(6));
    return size;
}

uint64_t FFLiCharModelCreateParam::GetCompressBufferSize(const FFLCharModelDesc* description) const
{
    if (!description->useMipmap)
        return 0;
    const uint32_t resolution = GetResolution(description->resolution);
    const bool mipmap = IsEnabledMipMap(description->resolution) != 0;
    uint32_t size = FFLiGetCompressBufferSizeFacelineTexture(resolution, mipmap);
    const uint32_t maskSize = FFLiGetCompressBufferSizeMaskTexture(resolution, mipmap);
    return maskSize > size ? maskSize : size;
}

int32_t FFLiCharModelCreateParam::GetTempBufferSize(const FFLCharModelDesc* description) const
{
    if (!CheckModelDesc(description, m_useMipmap) ||
        !m_resourceManager->IsValid(static_cast<FFLResourceType>(description->resourceType)))
        return 0;
    const uint32_t resolution = GetResolution(description->resolution);
    const bool mipmap = IsEnabledMipMap(description->resolution) != 0;
    const FFLResourceType resourceType = static_cast<FFLResourceType>(description->resourceType);
    const int maskSize = FFLiGetTempBufferSizeMaskTextures(description->expressionFlag, resolution, mipmap,
                                                            description->useMipmap, m_resourceManager, resourceType);
    const int facelineSize = FFLiGetTempBufferSizeFacelineTexture(resolution, mipmap, description->useMipmap,
                                                                   m_resourceManager, resourceType);
    return maskSize + 0x220 + facelineSize + static_cast<int>(GetCompressBufferSize(description)) +
           FFLiResourceUncompressBuffer::GetBufferSize(m_resourceManager, resourceType);
}
