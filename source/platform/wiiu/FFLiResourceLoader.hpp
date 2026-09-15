#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCELOADER_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCELOADER_HPP
#include <stdint.h>

#include "platform/wiiu/FFLiFsFile.hpp"
#include "platform/wiiu/FFLiResourceLoaderBuffer.hpp"
#include "platform/wiiu/FFLiResourceManager.hpp"

struct FFLiResourcePartsInfo;

class FFLiResourceLoader
{
public:
    FFLiResourceLoader(FFLiResourceManager* resourceManager,
                       FFLiResourceLoaderBuffer* buffer,
                       FFLResourceType resourceType);
    ~FFLiResourceLoader();

    uint32_t Close();
    bool IsExpand() const;
    const FFLiResourceHeader* Header() const;
    uint32_t GetTextureAlignedMaxSize(FFLiTexturePartsType parts) const;
    uint32_t GetShapeAlignedMaxSize(FFLiShapePartsType parts) const;
    bool IsCached() const;
    uint32_t GetPointerFromCache(void** destination,
                                 const FFLiResourcePartsInfo& parts);
    uint32_t LoadFromCache(void* destination,
                           const FFLiResourcePartsInfo& parts);
    uint32_t OpenIfClosed();
    int ReadWithPos(void* destination, unsigned int position, unsigned int size);
    uint32_t LoadFromFile(void* destination,
                          const FFLiResourcePartsInfo& parts);
    uint32_t Load(void* destination, unsigned int* size,
                  const FFLiResourcePartsInfo& parts);
    uint32_t LoadTexture(void* destination, unsigned int* size,
                         FFLiTexturePartsType parts, unsigned int index);
    uint32_t LoadShape(void* destination, unsigned int* size,
                       FFLiShapePartsType parts, unsigned int index);
    uint32_t GetPointerTextureByExpandCache(void** destination,
                                            unsigned int* size,
                                            FFLiTexturePartsType parts,
                                            unsigned int index);
    bool IsValid() const;
    uint32_t GetPointerShapeByExpandCache(void** destination,
                                          unsigned int* size,
                                          FFLiShapePartsType parts,
                                          unsigned int index);
    uint32_t GetTextureSize(FFLiTexturePartsType parts, unsigned int index) const;
    uint32_t GetShapeSize(FFLiShapePartsType parts, unsigned int index) const;
    uint32_t GetTextureNum(FFLiTexturePartsType parts) const;
    uint32_t GetShapeNum(FFLiShapePartsType parts) const;

private:
    FFLiResourceManager* m_resourceManager;
    FFLiResourceLoaderBuffer* m_buffer;
    FFLiFsCommand* m_command;
    FFLiFsFile m_file;
    FFLResourceType m_resourceType;
};
#endif
