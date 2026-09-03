#include "game/client/render/DynamicTexture.hpp"

#include "game/client/render/TextureUtil.hpp"
#include "game/core/BufferedImage.hpp"

DynamicTexture::DynamicTexture(BufferedImage* image)
    : textureId(-1), blur(false), mipmap(false), oldBlur(false), oldMipmap(false),
      pixels(image->getWidth() * image->getHeight(), true),
      width(image->getWidth()), height(image->getHeight())
{
    image->getRGB(0, 0, width, height, pixels, 0, width, 0);
}

DynamicTexture::DynamicTexture(int width, int height)
    : textureId(-1), blur(false), mipmap(false), oldBlur(false), oldMipmap(false),
      pixels(), width(width), height(height)
{
    _init(width, height);
}

void DynamicTexture::_init(int width, int height)
{
    this->width = width;
    this->height = height;

    pixels = arrayWithLength<int>(width * height, true);

    if (textureId == -1)
    {
        textureId = TextureUtil::generateTextureId();
    }
    TextureUtil::prepareImage(textureId, width, height);
}

void DynamicTexture::upload()
{
    if (textureId == -1)
    {
        textureId = TextureUtil::generateTextureId();
    }
    TextureUtil::uploadImage(textureId, pixels, width, height);
}

void DynamicTexture::load(ResourceManager* resourceManager)
{
}

arrayWithLength<int> DynamicTexture::getPixels()
{
    return pixels;
}
