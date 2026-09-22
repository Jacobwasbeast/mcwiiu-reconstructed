#include "platform/wiiu/FFLiResourceCache.hpp"

uint32_t FFLiIsVaildResourceHeader(const FFLiResourceHeader* header);

FFLiResourceCache::FFLiResourceCache()
{
    resources[0].header = 0;
    resources[0].size = 0;
    resources[1].header = 0;
    resources[1].size = 0;
}

FFLiResourceCache::~FFLiResourceCache()
{
}

int32_t FFLiResourceCache::Attach(const void* header, uint32_t size, FFLResourceType type)
{
    const uint32_t index = static_cast<uint32_t>(type);
    const uint32_t result = FFLiIsVaildResourceHeader(static_cast<const FFLiResourceHeader*>(header));
    if (result == 0)
    {
        resources[index].header = header;
        resources[index].size = size;
    }
    return static_cast<int32_t>(result);
}

void FFLiResourceCache::Detach(FFLResourceType type)
{
    const uint32_t index = static_cast<uint32_t>(type);
    resources[index].header = 0;
    resources[index].size = 0;
}

bool FFLiResourceCache::IsValid(FFLResourceType type) const
{
    return resources[static_cast<uint32_t>(type)].header != 0;
}

uint32_t FFLiResourceCache::IsAttached() const
{
    uint32_t type = 0;
    do
    {
        if (IsValid(static_cast<FFLResourceType>(type)))
        {
            return 1;
        }
        ++type;
    }
    while (type < 2);
    return 0;
}

uint32_t FFLiResourceCache::Header(FFLResourceType type) const
{
    return static_cast<uint32_t>(reinterpret_cast<uintptr_t>(resources[static_cast<uint32_t>(type)].header));
}
