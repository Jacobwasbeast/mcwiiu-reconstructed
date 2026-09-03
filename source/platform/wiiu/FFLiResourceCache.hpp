#ifndef SOURCE_PLATFORM_WIIU_FFLIRESOURCECACHE_HPP
#define SOURCE_PLATFORM_WIIU_FFLIRESOURCECACHE_HPP

#include <stdint.h>

struct FFLiResourceHeader;

enum FFLResourceType
{
    FFLResourceType_0 = 0,
    FFLResourceType_1 = 1
};

class FFLiResourceCache
{
private:
    struct Resource
    {
        const void* header;
        uint32_t size;
    };

    Resource resources[2];

public:
    FFLiResourceCache();
    ~FFLiResourceCache();
    int32_t Attach(const void* header, uint32_t size, FFLResourceType type);
    void Detach(FFLResourceType type);
    bool IsValid(FFLResourceType type) const;
    uint32_t IsAttached() const;
    uint32_t Header(FFLResourceType type) const;
};

#endif
