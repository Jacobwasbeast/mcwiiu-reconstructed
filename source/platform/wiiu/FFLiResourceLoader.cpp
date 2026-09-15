#include "platform/wiiu/FFLiResourceLoader.hpp"
#include "platform/wiiu/FFLiFsCommand.hpp"
#include "platform/wiiu/FFLiResourceUncompressBuffer.hpp"
#include <string.h>

extern unsigned int FFLiRoundUp(unsigned int value, unsigned int alignment);
extern bool FFLiResourceLoaderUncompress(void* destination, const void* source,
                                         FFLiResourceUncompressBuffer* buffer,
                                         const FFLiResourcePartsInfo& parts);
extern const FFLiResourcePartsInfo* FFLiGetTextureResoucePartsInfos__FPUiP25FFLiResourceTextureHeader20FFLiTexturePartsType(
    unsigned int*, const void*, unsigned int);
extern const FFLiResourcePartsInfo* FFLiGetShapeResoucePartsInfos__FPUiP23FFLiResourceShapeHeader18FFLiShapePartsType(
    unsigned int*, const void*, unsigned int);

FFLiResourceLoader::FFLiResourceLoader(FFLiResourceManager* resourceManager, FFLiResourceLoaderBuffer* buffer, FFLResourceType resourceType)
    : m_resourceManager(resourceManager), m_buffer(buffer), m_command(FFLiFsCommand::PlacementNew(reinterpret_cast<FFLiFsCommandBuffer*>(buffer->mResourceBuffer), resourceManager->m_client)), m_file(m_command), m_resourceType(resourceType)
{
}

FFLiResourceLoader::~FFLiResourceLoader()
{
    Close();
    FFLiFsCommand::PlacementDelete(m_command);
}

uint32_t FFLiResourceLoader::Close()
{
    if (m_file.IsOpened())
    {
        return static_cast<uint32_t>(m_file.Close(0));
    }
    return 0;
}

bool FFLiResourceLoader::IsExpand() const
{
    return m_resourceManager->IsExpand(m_resourceType);
}

const FFLiResourceHeader* FFLiResourceLoader::Header() const
{
    return m_resourceManager->Header(m_resourceType);
}

uint32_t FFLiResourceLoader::GetTextureAlignedMaxSize(FFLiTexturePartsType parts) const
{
    const FFLiResourceHeader* header = Header();
    return FFLiRoundUp(header->resourceWords[2 + static_cast<unsigned int>(parts)], 64);
}

uint32_t FFLiResourceLoader::GetShapeAlignedMaxSize(FFLiShapePartsType parts) const
{
    const FFLiResourceHeader* header = Header();
    return FFLiRoundUp(header->resourceWords[1281 + static_cast<unsigned int>(parts)], 64);
}

bool FFLiResourceLoader::IsCached() const
{
    return m_resourceManager->IsCached();
}

uint32_t FFLiResourceLoader::GetPointerFromCache(void** destination, const FFLiResourcePartsInfo& parts)
{
    if (!IsCached())
    {
        return 5;
    }
    *destination = reinterpret_cast<unsigned char*>(const_cast<FFLiResourceHeader*>(m_resourceManager->HeaderFromCache(m_resourceType))) + parts.offset;
    return 0;
}

uint32_t FFLiResourceLoader::LoadFromCache(void* destination, const FFLiResourcePartsInfo& parts)
{
    void* source = 0;
    const uint32_t result = GetPointerFromCache(&source, parts);
    if (result != 0)
    {
        return result;
    }
    if (parts.strategy == 5)
    {
        memcpy(destination, source, parts.uncompressedSize);
        return 0;
    }
    if (!FFLiResourceLoaderUncompress(destination, source, m_buffer, parts))
    {
        return 5;
    }
    return 0;
}

uint32_t FFLiResourceLoader::OpenIfClosed()
{
    if (m_file.IsOpened())
    {
        return 0;
    }
    return static_cast<uint32_t>(m_file.Open(m_resourceManager->GetPath(m_resourceType), "r", 0));
}

int FFLiResourceLoader::ReadWithPos(void* destination, unsigned int position, unsigned int size)
{
    const unsigned int alignedSize = FFLiRoundUp(size, 64);
    return m_file.ReadWithPos(destination, alignedSize, 1, position, 0, 0);
}

uint32_t FFLiResourceLoader::LoadFromFile(void* destination, const FFLiResourcePartsInfo& parts)
{
    if (OpenIfClosed() != 0)
    {
        return 9;
    }
    if (parts.strategy == 5)
        return ReadWithPos(destination, parts.offset, parts.uncompressedSize) == 1 ? 0 : 9;
    void* compressed = reinterpret_cast<void*>(m_buffer->Buffer());
    if (ReadWithPos(compressed, parts.offset, parts.compressedSize) != 1)
        return 9;
    return FFLiResourceLoaderUncompress(destination, compressed, m_buffer, parts) ? 0 : 5;
}

uint32_t FFLiResourceLoader::Load(void* destination, unsigned int* size, const FFLiResourcePartsInfo& parts)
{
    *size = parts.uncompressedSize;
    if (parts.uncompressedSize == 0)
        return 0;
    if (IsCached())
    {
        return LoadFromCache(destination, parts);
    }
    return LoadFromFile(destination, parts);
}

uint32_t FFLiResourceLoader::LoadTexture(void* destination, unsigned int* size, FFLiTexturePartsType parts, unsigned int index)
{
    unsigned int count;
    const FFLiResourcePartsInfo* infos = FFLiGetTextureResoucePartsInfos__FPUiP25FFLiResourceTextureHeader20FFLiTexturePartsType(
        &count, Header()->resourceWords + 2, static_cast<unsigned int>(parts));
    if (infos == 0 || index >= count)
        return 1;
    return Load(destination, size, infos[index]);
}

uint32_t FFLiResourceLoader::LoadShape(void* destination, unsigned int* size, FFLiShapePartsType parts, unsigned int index)
{
    unsigned int count;
    const FFLiResourcePartsInfo* infos = FFLiGetShapeResoucePartsInfos__FPUiP23FFLiResourceShapeHeader18FFLiShapePartsType(
        &count, Header()->resourceWords + 1281, static_cast<unsigned int>(parts));
    if (infos == 0 || index >= count)
        return 1;
    return Load(destination, size, infos[index]);
}

uint32_t FFLiResourceLoader::GetPointerTextureByExpandCache(void** destination, unsigned int* size, FFLiTexturePartsType parts, unsigned int index)
{
    if (!IsExpand())
        return 5;
    unsigned int count;
    const FFLiResourcePartsInfo* infos = FFLiGetTextureResoucePartsInfos__FPUiP25FFLiResourceTextureHeader20FFLiTexturePartsType(
        &count, Header()->resourceWords + 2, static_cast<unsigned int>(parts));
    if (infos == 0 || index >= count)
        return 1;
    *size = infos[index].uncompressedSize;
    return GetPointerFromCache(destination, infos[index]);
}
