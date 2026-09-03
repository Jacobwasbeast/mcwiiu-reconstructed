#include "platform/wiiu/FFLiResourceManager.hpp"

#include "platform/wiiu/FFLiFsClient.hpp"
#include "platform/wiiu/FFLiFsCommand.hpp"
#include "platform/wiiu/FFLiFsFile.hpp"

#include <string.h>

extern int FFLiGetResourcePath(char* path, unsigned int pathSize, const char* relativePath);
extern uint32_t FFLiConvertFSStatusToFFLResult(int status);
extern uint32_t FFLiIsVaildResourceHeader(const FFLiResourceHeader* header);
extern unsigned int FFLiRoundUp(unsigned int value, unsigned int alignment);

FFLiResourceManager::FFLiResourceManager(FFLiResourceMultiHeader* headers, FFLiFsClient* client)
    : m_headers(headers), m_client(client)
{
    memset(m_headers, 0, sizeof(FFLiResourceMultiHeader));
    memset(m_paths, 0, sizeof(m_paths));
}

FFLiResourceManager::~FFLiResourceManager()
{
}

const char* FFLiResourceManager::GetRelativeResourcePath(FFLResourceType type, bool expanded)
{
    static const char* paths[2][2] =
    {
        { "FFLResMiddle.dat", "FFLResHigh.dat" },
        { "FFLResMiddleLG.dat", "FFLResHighLG.dat" }
    };
    return paths[expanded ? 1 : 0][static_cast<unsigned int>(type)];
}

int32_t FFLiResourceManager::GetResourcePath(char* path, unsigned int pathSize, FFLResourceType type, bool expanded)
{
    const int status = FFLiGetResourcePath(path, pathSize, GetRelativeResourcePath(type, expanded));
    return static_cast<int32_t>(FFLiConvertFSStatusToFFLResult(status));
}

int32_t FFLiResourceManager::AfterConstruct()
{
    unsigned int type = 0;
    do
    {
        const int32_t result = GetResourcePath(m_paths[type], 256, static_cast<FFLResourceType>(type), false);
        if (result != 0)
        {
            return result;
        }
        ++type;
    }
    while (type < 2);
    return 0;
}

const char* FFLiResourceManager::GetPath(FFLResourceType type) const
{
    return m_paths[static_cast<unsigned int>(type)];
}

int32_t FFLiResourceManager::LoadResourceHeaderImpl(FFLiFsCommand* command)
{
    FFLiFsFile file(command);
    unsigned int type = 0;
    while (true)
    {
        FFLiResourceHeader* header = &((m_headers + type)->header);
        if (file.Open(GetPath(static_cast<FFLResourceType>(type)), "r", 0) != 0)
        {
            break;
        }
        if (file.Read(header, 0x4a00, 1, 0, 0) != 1)
        {
            file.Close(0);
            return 3;
        }
        if (file.Close(0) != 0)
        {
            break;
        }
        const int32_t result = static_cast<int32_t>(FFLiIsVaildResourceHeader(header));
        if (result != 0)
        {
            return result;
        }
        ++type;
        if (type > 1)
        {
            return 0;
        }
    }
    return 3;
}

uint32_t FFLiResourceManager::LoadResourceHeader(FFLiFsCommandBuffer* buffer)
{
    FFLiFsCommand* command = FFLiFsCommand::PlacementNew(buffer, m_client);
    const uint32_t result = static_cast<uint32_t>(LoadResourceHeaderImpl(command));
    FFLiFsCommand::PlacementDelete(command);
    return result;
}

void FFLiResourceManager::AttachCache(const void* header, unsigned int size, FFLResourceType type)
{
    m_cache.Attach(header, size, type);
}

bool FFLiResourceManager::IsCached() const
{
    return m_cache.IsAttached() != 0;
}

const FFLiResourceHeader* FFLiResourceManager::HeaderFromCache(FFLResourceType type) const
{
    return (const FFLiResourceHeader*)m_cache.Header(type);
}

const FFLiResourceHeader* FFLiResourceManager::HeaderFromFile(FFLResourceType type) const
{
    return &((m_headers + static_cast<unsigned int>(type))->header);
}

const FFLiResourceHeader* FFLiResourceManager::Header(FFLResourceType type) const
{
    if (IsCached())
    {
        return HeaderFromCache(type);
    }
    return HeaderFromFile(type);
}

uint32_t FFLiResourceManager::GetTextureAlignedMaxSize(FFLResourceType type, FFLiTexturePartsType parts) const
{
    const FFLiResourceHeader* header = Header(type);
    return FFLiRoundUp(header->resourceWords[5 + static_cast<unsigned int>(parts)], 64);
}

uint32_t FFLiResourceManager::GetShapeAlignedMaxSize(FFLResourceType type, FFLiShapePartsType parts) const
{
    const FFLiResourceHeader* header = Header(type);
    return FFLiRoundUp(header->resourceWords[1284 + static_cast<unsigned int>(parts)], 64);
}

bool FFLiResourceManager::IsValid(FFLResourceType type) const
{
    if (IsCached())
    {
        return m_cache.IsValid(type);
    }
    return true;
}

bool FFLiResourceManager::IsExpand(FFLResourceType type) const
{
    return IsValid(type) && IsCached() && Header(type)->resourceWords[0] == 1;
}

uint32_t FFLiResourceManager::GetUncompressBufferSize(FFLResourceType type) const
{
    return FFLiRoundUp(Header(type)->uncompressBufferSize + 64, 64);
}

void FFLiResourceManager::DetachCache(FFLResourceType type)
{
    m_cache.Detach(type);
}
