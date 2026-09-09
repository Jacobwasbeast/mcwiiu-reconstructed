#include "platform/wiiu/FFLiResourceLoader.hpp"
#include "platform/wiiu/FFLiFsCommand.hpp"
#include "platform/wiiu/FFLiResourceUncompressBuffer.hpp"
#include "vendor/__N_22_FFLiResourceLoader_cpp_4cb71113.hpp"

extern unsigned int FFLiRoundUp(unsigned int value, unsigned int alignment);
bool FFLiResourceLoaderUncompress(void* destination, const void* source, FFLiResourceUncompressBuffer* buffer, const FFLiResourcePartsInfo& parts);

FFLiResourceLoader::FFLiResourceLoader(FFLiResourceManager* resourceManager, FFLiResourceLoaderBuffer* buffer, FFLResourceType resourceType)
    : m_resourceManager(resourceManager), m_buffer(buffer), m_command(FFLiFsCommand::PlacementNew(0, 0)), m_file(0), m_resourceType(resourceType)
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
    return m_resourceManager != 0 && m_resourceManager->IsExpand(m_resourceType);
}

const FFLiResourceHeader* FFLiResourceLoader::Header() const
{
    return m_resourceManager == 0 ? 0 : m_resourceManager->Header(m_resourceType);
}

uint32_t FFLiResourceLoader::GetTextureAlignedMaxSize(FFLiTexturePartsType parts) const
{
    const FFLiResourceHeader* header = Header();
    return header == 0 ? 0 : FFLiRoundUp(header->resourceWords[5 + static_cast<unsigned int>(parts)], 64);
}

uint32_t FFLiResourceLoader::GetShapeAlignedMaxSize(FFLiShapePartsType parts) const
{
    const FFLiResourceHeader* header = Header();
    return header == 0 ? 0 : FFLiRoundUp(header->resourceWords[1284 + static_cast<unsigned int>(parts)], 64);
}

bool FFLiResourceLoader::IsCached() const
{
    return m_resourceManager != 0 && m_resourceManager->IsCached();
}

uint32_t FFLiResourceLoader::GetPointerFromCache(void** destination, const FFLiResourcePartsInfo& parts)
{
    if (!IsCached() || destination == 0 || Header() == 0)
    {
        return 5;
    }
    *destination = const_cast<FFLiResourceHeader*>(Header());
    if (&parts == 0)
        return 5;
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
    if (m_resourceManager == 0)
    {
        return 9;
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
    if (!FFLiResourceLoaderUncompress(destination, (const void*)m_buffer->Buffer(), m_buffer, parts))
    {
        return 5;
    }
    return 0;
}

uint32_t FFLiResourceLoader::Load(void* destination, unsigned int* size, const FFLiResourcePartsInfo& parts)
{
    if (size != 0)
    {
        *size = 0;
    }
    if (IsCached())
    {
        return LoadFromCache(destination, parts);
    }
    return LoadFromFile(destination, parts);
}

uint32_t FFLiResourceLoader::LoadTexture(void* destination, unsigned int* size, FFLiTexturePartsType parts, unsigned int index)
{
    if (Header() == 0 || index >= GetTextureNum(parts))
    {
        return 1;
    }
    if (destination == 0 && size == 0)
        return 1;
    return 1;
}

uint32_t FFLiResourceLoader::LoadShape(void* destination, unsigned int* size, FFLiShapePartsType parts, unsigned int index)
{
    if (Header() == 0 || index >= GetShapeNum(parts))
    {
        return 1;
    }
    if (destination == 0 && size == 0)
        return 1;
    return 1;
}

uint32_t FFLiResourceLoader::GetPointerTextureByExpandCache(void** destination, unsigned int* size, FFLiTexturePartsType parts, unsigned int index)
{
    if (!IsExpand() || destination == 0 || index >= GetTextureNum(parts))
    {
        return 1;
    }
    if (size != 0)
    {
        *size = GetTextureSize(parts, index);
    }
    return 5;
}

bool FFLiResourceLoader::IsValid() const
{
    return m_resourceManager != 0 && m_resourceManager->IsValid(m_resourceType);
}

uint32_t FFLiResourceLoader::GetPointerShapeByExpandCache(void** destination, unsigned int* size, FFLiShapePartsType parts, unsigned int index)
{
    if (!IsExpand() || destination == 0 || index >= GetShapeNum(parts))
    {
        return 1;
    }
    if (size != 0)
    {
        *size = GetShapeSize(parts, index);
    }
    return 5;
}

uint32_t FFLiResourceLoader::GetTextureSize(FFLiTexturePartsType parts, unsigned int index) const
{
    const uint32_t count = GetTextureNum(parts);
    return index < count ? GetTextureAlignedMaxSize(parts) : 0;
}

uint32_t FFLiResourceLoader::GetShapeSize(FFLiShapePartsType parts, unsigned int index) const
{
    const uint32_t count = GetShapeNum(parts);
    return index < count ? GetShapeAlignedMaxSize(parts) : 0;
}

uint32_t FFLiResourceLoader::GetTextureNum(FFLiTexturePartsType parts) const
{
    const FFLiResourceHeader* header = Header();
    return header == 0 ? 0 : header->resourceWords[4 + static_cast<unsigned int>(parts)];
}

uint32_t FFLiResourceLoader::GetShapeNum(FFLiShapePartsType parts) const
{
    const FFLiResourceHeader* header = Header();
    return header == 0 ? 0 : header->resourceWords[1283 + static_cast<unsigned int>(parts)];
}
