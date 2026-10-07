#include "game/client/render/Texture.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TextureManager.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/Buffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemSect.hpp"
#include <cstring>
#include <algorithm>

Texture::Texture(const std::wstring& name, int type, int width, int height, int wrap, int format,
                 int minFilter, int magFilter, BufferedImage* image, bool mipmap)
{
    _init(name, type, width, height, 1, wrap, format, minFilter, magFilter, image, mipmap);
}

Texture::Texture(const std::wstring& name, int type, int width, int height, int depth, int wrap,
                 int format, int minFilter, int magFilter, BufferedImage* image, bool mipmap)
{
    _init(name, type, width, height, depth, wrap, format, minFilter, magFilter, image, mipmap);
}

Texture::~Texture()
{
    delete m_rect;
    m_rect = NULL;
    for (int i = 0; i < 10; ++i)
    {
        if (m_data[i] != NULL)
        {
            delete m_data[i];
            m_data[i] = NULL;
        }
    }
    if ((int)m_glId >= 0)
        GlStateManager::deleteTexture((int)m_glId);
}

void Texture::_init(const std::wstring& name, int type, int width, int height, int depth, int wrap,
                    int format, int minFilter, int magFilter, bool mipmap)
{
    m_name = name;
    m_type = type;
    m_width = width;
    m_height = height;
    m_depth = depth;
    m_format = format;
    m_minFilter = minFilter;
    m_magFilter = magFilter;
    m_wrap = wrap;
    m_immediateUpdate = false;
    for (int i = 0; i < 10; ++i)
        m_data[i] = NULL;
    m_gpuAllocated = false;
    m_rect = new Rect2i(0, 0, width, height);
    m_target = 1;
    m_mipmap = mipmap || (minFilter != 0 && minFilter != 1) || (magFilter != 0 && magFilter != 1);
    int mipLevels = 1;
    if (m_mipmap)
    {
        int widthLevels = 1;
        int heightLevels = 1;
        while ((8 << widthLevels) < width) ++widthLevels;
        while ((8 << heightLevels) < height) ++heightLevels;
        if (heightLevels < widthLevels)
            widthLevels = heightLevels;
        if (5 < widthLevels)
            widthLevels = 5;
        mipLevels = widthLevels;
    }
    m_mipLevels = mipLevels;
    if (type == 2)
    {
        m_glId = (unsigned int)-1;
    }
    else
    {
        m_glId = (unsigned int)GlStateManager::genTexture();
        GlStateManager::bindTexture((int)m_glId);
        GlStateManager::texParameteri(m_target, 1, minFilter);
        GlStateManager::texParameteri(m_target, 2, magFilter);
        GlStateManager::texParameteri(m_target, 3, wrap);
        GlStateManager::texParameteri(m_target, 4, wrap);
    }
    m_managerId = TextureManager::getInstance()->createTextureID();
}

void Texture::_init(const std::wstring& name, int type, int width, int height, int depth, int wrap,
                    int format, int minFilter, int magFilter, BufferedImage* image, bool mipmap)
{
    _init(name, type, width, height, depth, wrap, format, minFilter, magFilter, mipmap);
    if (image == NULL)
    {
        if (width != -1 && height != -1)
        {
            unsigned int levelCount = width * height * depth;
            unsigned int size = levelCount * 4;
            arrayWithLength<unsigned char> bytes(size, true);
            for (unsigned int i = 0; i < size; ++i)
                bytes.data[i] = 0;
            m_data[0] = ByteBuffer::allocateDirect(size);
            m_data[0]->clear();
            m_data[0]->put(bytes);
            m_data[0]->position(0)->limit(size);
            delete[] bytes.data;
            if (m_mipmap)
            {
                m_mipLevels = 5;
                for (int level = 1; level < m_mipLevels; ++level)
                {
                    unsigned int mipCount = (width >> level) * (height >> level) * depth;
                    unsigned int mipSize = mipCount * 4;
                    arrayWithLength<unsigned char> mipBytes(mipSize, true);
                    for (unsigned int i = 0; i < mipSize; ++i)
                        mipBytes.data[i] = 0;
                    m_data[level] = ByteBuffer::allocateDirect(mipSize);
                    m_data[level]->clear();
                    m_data[level]->put(mipBytes);
                    m_data[level]->position(0)->limit(mipSize);
                    delete[] mipBytes.data;
                }
            }
            if (m_immediateUpdate)
                updateOnGPU();
            else
                m_updated = false;
        }
        else
        {
            m_valid = false;
        }
    }
    else
    {
        m_valid = true;
        transferFromImage(image);
        if (type != 2)
        {
            updateOnGPU();
            m_immediateUpdate = false;
        }
    }
}

unsigned int Texture::crispBlend(int first, int second)
{
    int firstWeight = (unsigned int)first >> 24;
    int secondWeight = (unsigned int)second >> 24;
    int alpha = 255;
    if (firstWeight + secondWeight < 255)
    {
        alpha = 0;
        firstWeight = 1;
        secondWeight = 1;
    }
    else if (secondWeight < firstWeight)
    {
        firstWeight = 255;
        secondWeight = 1;
    }
    else
    {
        firstWeight = 1;
        secondWeight = 255;
    }
    int red = ((first >> 16 & 255) * firstWeight + (second >> 16 & 255) * secondWeight) / (firstWeight + secondWeight);
    int green = ((first >> 8 & 255) * firstWeight + (second >> 8 & 255) * secondWeight) / (firstWeight + secondWeight);
    int blue = ((first & 255) * firstWeight + (second & 255) * secondWeight) / (firstWeight + secondWeight);
    return (alpha << 24) | (red << 16) | (green << 8) | blue;
}

void Texture::transferFromImage(BufferedImage* image)
{
    int imageWidth = (int)image->getWidth();
    int imageHeight = (int)image->getHeight();
    if (imageWidth > m_width || imageHeight > m_height) return;

    int byteOrders[2][4] = { { 3, 0, 1, 2 }, { 3, 2, 1, 0 } };
    int* order = byteOrders[0];
    if (m_format == 1) order = byteOrders[1];

    arrayWithLength<int> pixels(m_width * m_height, true);
    image->getRGB(0, 0, m_width, m_height, pixels, 0, imageWidth, 0);

    unsigned int byteCount = m_width * m_height * 4;
    arrayWithLength<unsigned char> bytes(byteCount, true);
    for (int y = 0; y < m_height; ++y)
    {
        for (int x = 0; x < m_width; ++x)
        {
            int index = (y * m_width + x) * 4;
            bytes[index + order[0]] = (unsigned char)((unsigned int)pixels[y * m_width + x] >> 24);
            bytes[index + order[1]] = (unsigned char)((unsigned int)pixels[y * m_width + x] >> 16);
            bytes[index + order[2]] = (unsigned char)((unsigned int)pixels[y * m_width + x] >> 8);
            bytes[index + order[3]] = (unsigned char)pixels[y * m_width + x];
        }
    }

    for (int i = 0; i < 10; ++i)
    {
        if (m_data[i] != NULL)
        {
            delete m_data[i];
            m_data[i] = NULL;
        }
    }

    MemSect(0x33);
    m_data[0] = ByteBuffer::allocateDirect((int)byteCount);
    MemSect(0);
    m_data[0]->clear();
    m_data[0]->put(bytes);
    m_data[0]->limit(byteCount);
    delete[] bytes.data;

    if (m_mipmap || image->getData(1) != NULL)
    {
        m_mipmap = true;
        for (unsigned int level = 1; level < 5; ++level)
        {
            int levelWidth = m_width >> level;
            int levelHeight = m_height >> level;
            unsigned int levelCount = levelWidth * levelHeight;
            arrayWithLength<unsigned char> levelBytes(levelCount * 4, true);
            int* levelPixels = new int[levelCount];
            if (image->getData(level) == NULL)
            {
                int previousWidth = m_width >> (level - 1);
                for (int x = 0; x < levelWidth; ++x)
                {
                    for (int y = 0; y < levelHeight; ++y)
                    {
                        unsigned int topLeft = m_data[level - 1]->getInt((y * 2 * previousWidth + x * 2) * 4);
                        unsigned int topRight = m_data[level - 1]->getInt((y * 2 * previousWidth + x * 2 + 1) * 4);
                        unsigned int bottomRight = m_data[level - 1]->getInt(((y * 2 + 1) * previousWidth + x * 2 + 1) * 4);
                        unsigned int bottomLeft = m_data[level - 1]->getInt(((y * 2 + 1) * previousWidth + x * 2) * 4);
                        topLeft = (topLeft << 24) | (topLeft >> 8);
                        topRight = (topRight << 24) | (topRight >> 8);
                        bottomRight = (bottomRight << 24) | (bottomRight >> 8);
                        bottomLeft = (bottomLeft << 24) | (bottomLeft >> 8);
                        unsigned int packed;
                        if (((topLeft & 0xff000000u) == 0xff000000u || (topLeft & 0xff000000u) == 0) &&
                            ((topRight & 0xff000000u) == 0xff000000u || (topRight & 0xff000000u) == 0) &&
                            ((bottomRight & 0xff000000u) == 0xff000000u || (bottomRight & 0xff000000u) == 0) &&
                            ((bottomLeft & 0xff000000u) == 0xff000000u || (bottomLeft & 0xff000000u) == 0))
                        {
                            unsigned int bottom = crispBlend((int)bottomRight, (int)bottomLeft);
                            unsigned int top = crispBlend((int)topLeft, (int)topRight);
                            packed = crispBlend((int)top, (int)bottom);
                        }
                        else
                        {
                            int alpha = (int)((topLeft >> 24) + (topRight >> 24) + (bottomRight >> 24) + (bottomLeft >> 24)) >> 2;
                            int red = (int)(((topLeft >> 16) & 0xff) + ((topRight >> 16) & 0xff) +
                                            ((bottomRight >> 16) & 0xff) + ((bottomLeft >> 16) & 0xff)) >> 2;
                            int green = (int)(((topLeft >> 8) & 0xff) + ((topRight >> 8) & 0xff) +
                                              ((bottomRight >> 8) & 0xff) + ((bottomLeft >> 8) & 0xff)) >> 2;
                            int blue = (int)((topLeft & 0xff) + (topRight & 0xff) + (bottomRight & 0xff) + (bottomLeft & 0xff)) >> 2;
                            packed = (unsigned int)((alpha << 24) | (red << 16) | (green << 8) | blue);
                        }
                        int destination = (y * levelWidth + x) * 4;
                        levelBytes[destination + order[0]] = (unsigned char)(packed >> 24);
                        levelBytes[destination + order[1]] = (unsigned char)(packed >> 16);
                        levelBytes[destination + order[2]] = (unsigned char)(packed >> 8);
                        levelBytes[destination + order[3]] = (unsigned char)packed;
                    }
                }
            }
            else
            {
                memcpy(levelPixels, image->getData(level), levelCount * 4);
                for (int y = 0; y < levelHeight; ++y)
                {
                    for (int x = 0; x < levelWidth; ++x)
                    {
                        int destination = (y * levelWidth + x) * 4;
                        levelBytes[destination + order[0]] = (unsigned char)((unsigned int)levelPixels[y * levelWidth + x] >> 24);
                        levelBytes[destination + order[1]] = (unsigned char)((unsigned int)levelPixels[y * levelWidth + x] >> 16);
                        levelBytes[destination + order[2]] = (unsigned char)((unsigned int)levelPixels[y * levelWidth + x] >> 8);
                        levelBytes[destination + order[3]] = (unsigned char)levelPixels[y * levelWidth + x];
                    }
                }
            }
            MemSect(0x33);
            m_data[level] = ByteBuffer::allocateDirect((int)(levelCount * 4));
            MemSect(0);
            m_data[level]->clear();
            m_data[level]->put(levelBytes);
            m_data[level]->limit(levelCount * 4);
            delete[] levelBytes.data;
            delete[] levelPixels;
        }
    }

    delete[] pixels.data;

    if (m_immediateUpdate)
    {
        updateOnGPU();
    }
    else
    {
        m_updated = false;
    }
}

void Texture::updateOnGPU()
{
    int level;

    m_data[0]->flip();
    if (m_mipmap && m_mipLevels > 1) {
        for (level = 1; level < m_mipLevels; ++level) {
            if (m_data[level] == NULL)
                break;
            m_data[level]->flip();
        }
    }
    if (!m_gpuAllocated) {
        gRenderer->TextureSetTextureLevels(m_mipLevels);
        gRenderer->TextureData(m_width, m_height, m_data[0]->getBuffer(), 0, C4JRender::eTextureFormat_RGBA8);
        if (m_mipmap && m_mipLevels > 1) {
            for (level = 1; level < m_mipLevels; ++level) {
                gRenderer->TextureData(m_width >> level, m_height >> level, m_data[level]->getBuffer(), level,
                                       C4JRender::eTextureFormat_RGBA8);
            }
        }
        m_updated = true;
        m_gpuAllocated = true;
    } else {
        gRenderer->TextureDataUpdate(0, 0, m_width, m_height, m_data[0]->getBuffer(), 0);
        if (m_mipmap && gRenderer->TextureGetTextureLevels() > 1 && m_mipLevels > 1) {
            for (level = 1; level < m_mipLevels; ++level) {
                gRenderer->TextureDataUpdate(0, 0, m_width >> level, m_height >> level, m_data[level]->getBuffer(),
                                             level);
            }
        }
        m_updated = true;
    }
}

ByteBuffer* Texture::getData(unsigned int mipLevel) { return m_data[mipLevel]; }
int Texture::getHeight() { return m_height; }
int Texture::getWidth() { return m_width; }
unsigned int Texture::getManagerId() { return m_managerId; }
unsigned int Texture::getGlId() { return m_glId; }

void Texture::blit(int x, int y, Texture* texture, bool flip)
{
    for (int level = 0; level < m_mipLevels; ++level)
    {
        ByteBuffer* sourceData = texture->getData(level);
        if (sourceData == NULL)
            break;

        const int destinationY = y >> level;
        const int destinationX = x >> level;
        const int destinationWidth = m_width >> level;
        const int sourceHeight = texture->getHeight() >> level;
        const unsigned int sourceWidth = texture->getWidth() >> level;
        int* source = reinterpret_cast<int*>(sourceData->getBuffer());
        int* destination = reinterpret_cast<int*>(m_data[level]->getBuffer());
        for (int row = 0; row < sourceHeight; ++row)
        {
            if (!flip)
            {
                std::memcpy(destination + (destinationY + row) * destinationWidth + destinationX,
                            source + row * sourceWidth, sourceWidth * 4);
            }
            else
            {
                for (int column = 0; column < (int)sourceWidth; ++column)
                {
                    destination[destinationX + destinationY + (sourceHeight - row) + column * destinationWidth] =
                        source[row * sourceWidth + column];
                }
            }
        }
    }

    if (m_immediateUpdate)
        updateOnGPU();
    else
        m_updated = false;
}

void Texture::bind(int textureUnit)
{
    GlStateManager::enableTexture();
    GlStateManager::activeTexture(textureUnit);
    GlStateManager::bindTexture((int)m_glId);
    if (!m_updated) updateOnGPU();
}
