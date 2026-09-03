#ifndef PLATFORM_WIIU_FFLIRESOURCETEXTUREFOOTER_HPP
#define PLATFORM_WIIU_FFLIRESOURCETEXTUREFOOTER_HPP

#include <stdint.h>

struct _GX2Surface;

struct FFLiResourceTextureFooter
{
    uint32_t mipOffset;
    uint8_t imageSize[5];
    uint8_t textureFormat;
    uint8_t mipLevel;
    uint8_t flags;

    unsigned int SurfaceFormat() const;
    static void* GetFooterImpl(const void* resource, unsigned int resourceSize);
    void* GetImagePtrImpl(unsigned int imageSize) const;
    void* GetMipPtrImpl(unsigned int mipOffset) const;
    void Init(const _GX2Surface* surface);
    void SwapEndian();
    void Dump(unsigned int size) const;
};

#endif
