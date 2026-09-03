#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCELOADER_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCELOADER_HPP
#include <stdint.h>
#include "platform/wiiu/FFLiFsFile.hpp"
#include "platform/wiiu/FFLiResourceManager.hpp"
#include "platform/wiiu/FFLiResourceLoaderBuffer.hpp"
struct FFLiResourcePartsInfo;
class FFLiResourceLoader {
    FFLiResourceManager* m_resourceManager; FFLiResourceLoaderBuffer* m_buffer; FFLiFsCommand* m_command; FFLiFsFile m_file; FFLResourceType m_resourceType;
public:
    FFLiResourceLoader(FFLiResourceManager*, FFLiResourceLoaderBuffer*, FFLResourceType); ~FFLiResourceLoader(); uint32_t Close(); bool IsExpand() const; const FFLiResourceHeader* Header() const;
    uint32_t GetTextureAlignedMaxSize(FFLiTexturePartsType) const; uint32_t GetShapeAlignedMaxSize(FFLiShapePartsType) const; bool IsCached() const;
    uint32_t GetPointerFromCache(void**, const FFLiResourcePartsInfo&); uint32_t LoadFromCache(void*, const FFLiResourcePartsInfo&); uint32_t OpenIfClosed(); int ReadWithPos(void*, unsigned int, unsigned int);
    uint32_t LoadFromFile(void*, const FFLiResourcePartsInfo&); uint32_t Load(void*, unsigned int*, const FFLiResourcePartsInfo&); uint32_t LoadTexture(void*, unsigned int*, FFLiTexturePartsType, unsigned int); uint32_t LoadShape(void*, unsigned int*, FFLiShapePartsType, unsigned int);
    uint32_t GetPointerTextureByExpandCache(void**, unsigned int*, FFLiTexturePartsType, unsigned int); bool IsValid() const; uint32_t GetPointerShapeByExpandCache(void**, unsigned int*, FFLiShapePartsType, unsigned int); uint32_t GetTextureSize(FFLiTexturePartsType, unsigned int) const; uint32_t GetShapeSize(FFLiShapePartsType, unsigned int) const; uint32_t GetTextureNum(FFLiTexturePartsType) const; uint32_t GetShapeNum(FFLiShapePartsType) const;
};
#endif
