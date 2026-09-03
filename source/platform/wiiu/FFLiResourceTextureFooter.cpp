#include "platform/wiiu/FFLiResourceTextureFooter.hpp"

unsigned int FFLiGetResourceSurfaceFormat(int textureFormat);

static uint32_t FFLiSwapTextureFooterWord(uint32_t value)
{
    return ((value & 0x000000ffU) << 24) |
           ((value & 0x0000ff00U) << 8) |
           ((value & 0x00ff0000U) >> 8) |
           ((value & 0xff000000U) >> 24);
}

unsigned int FFLiResourceTextureFooter::SurfaceFormat() const
{
    return FFLiGetResourceSurfaceFormat(textureFormat);
}

void* FFLiResourceTextureFooter::GetFooterImpl(const void* resource, unsigned int resourceSize)
{
    return (void*)((const uint8_t*)resource + resourceSize - 12U);
}

void* FFLiResourceTextureFooter::GetImagePtrImpl(unsigned int imageSize) const
{
    return (void*)((uint8_t*)this + 12U - imageSize);
}

void* FFLiResourceTextureFooter::GetMipPtrImpl(unsigned int offset) const
{
    if (mipOffset == 0U)
    {
        return 0;
    }
    return (void*)((uint8_t*)this + 12U + mipOffset - offset);
}

void FFLiResourceTextureFooter::Init(const _GX2Surface* surface)
{
    const uint8_t* values = (const uint8_t*)surface;
    mipOffset = 0U;
    imageSize[0] = values[32];
    imageSize[1] = values[33];
    imageSize[2] = values[34];
    imageSize[3] = values[35];
    imageSize[4] = 0;
    textureFormat = values[20];
    mipLevel = values[16];
    flags = values[17];
}

void FFLiResourceTextureFooter::SwapEndian()
{
    mipOffset = FFLiSwapTextureFooterWord(mipOffset);
    uint8_t temporary = imageSize[0];
    imageSize[0] = imageSize[3];
    imageSize[3] = temporary;
    temporary = imageSize[1];
    imageSize[1] = imageSize[2];
    imageSize[2] = temporary;
}

void FFLiResourceTextureFooter::Dump(unsigned int size) const
{
    volatile unsigned int value = size;
    value += mipOffset;
    value += textureFormat;
}
