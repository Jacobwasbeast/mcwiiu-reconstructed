#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCEMANAGER_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCEMANAGER_HPP

#include <stdint.h>

#include "platform/wiiu/FFLiFsCommand.hpp"
#include "platform/wiiu/FFLiResourceCache.hpp"
#include "platform/wiiu/FFLiResourceHeader.hpp"

class FFLiFsClient;

struct FFLiResourceMultiHeader
{
    FFLiResourceHeader header;
};

enum FFLiTexturePartsType
{
    FFLiTexturePartsType_0 = 0
};

enum FFLiShapePartsType
{
    FFLiShapePartsType_0 = 0
};

class FFLiResourceManager
{
private:
    FFLiResourceMultiHeader* m_headers;
    FFLiFsClient* m_client;
    FFLiResourceCache m_cache;
    char m_paths[2][256];

public:
    FFLiResourceManager(FFLiResourceMultiHeader* headers, FFLiFsClient* client);
    ~FFLiResourceManager();
    static const char* GetRelativeResourcePath(FFLResourceType type, bool expanded);
    static int32_t GetResourcePath(char* path, unsigned int pathSize, FFLResourceType type, bool expanded);
    int32_t AfterConstruct();
    const char* GetPath(FFLResourceType type) const;
    int32_t LoadResourceHeaderImpl(FFLiFsCommand* command);
    uint32_t LoadResourceHeader(FFLiFsCommandBuffer* buffer);
    int32_t AttachCache(const void* header, unsigned int size, FFLResourceType type);
    bool IsCached() const;
    const FFLiResourceHeader* HeaderFromCache(FFLResourceType type) const;
    const FFLiResourceHeader* HeaderFromFile(FFLResourceType type) const;
    const FFLiResourceHeader* Header(FFLResourceType type) const;
    uint32_t GetTextureAlignedMaxSize(FFLResourceType type, FFLiTexturePartsType parts) const;
    uint32_t GetShapeAlignedMaxSize(FFLResourceType type, FFLiShapePartsType parts) const;
    bool IsValid(FFLResourceType type) const;
    bool IsExpand(FFLResourceType type) const;
    uint32_t GetUncompressBufferSize(FFLResourceType type) const;
    void DetachCache(FFLResourceType type);
};

#endif
