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
        delete m_data[i];
        m_data[i] = NULL;
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
    m_mipLevels = 1;
    if (m_mipmap)
    {
        unsigned int widthLevels = 1;
        unsigned int heightLevels = 1;
        while ((8 << widthLevels) < width) ++widthLevels;
        while ((8 << heightLevels) < height) ++heightLevels;
        m_mipLevels = std::min(5u, std::min(widthLevels, heightLevels));
    }
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
    if (image != NULL)
    {
        m_valid = true;
        transferFromImage(image);
        if (type != 2)
        {
            updateOnGPU();
            m_immediateUpdate = false;
        }
        return;
    }
    if (width != -1 && height != -1)
    {
        if (m_mipmap) m_mipLevels = 5;
        for (unsigned int level = 0; level < m_mipLevels; ++level)
        {
            const unsigned int size = (width >> level) * (height >> level) * depth * 4;
            arrayWithLength<unsigned char> bytes(size, false);
            std::memset(bytes.data, 0, size);
            m_data[level] = ByteBuffer::allocateDirect(size);
            m_data[level]->clear();
            m_data[level]->put(bytes);
            m_data[level]->position(0);
            m_data[level]->limit(size);
            delete[] bytes.data;
        }
        if (m_immediateUpdate)
            updateOnGPU();
        else
            m_updated = false;
        return;
    }
    m_valid = false;
}

unsigned int Texture::crispBlend(int first, int second)
{
    unsigned int firstColor = (unsigned int)first, secondColor = (unsigned int)second;
    unsigned int alpha1 = (firstColor >> 24) & 255, alpha2 = (secondColor >> 24) & 255;
    if (alpha1 + alpha2 < 255)
        return (((((firstColor >> 16) & 255) + ((secondColor >> 16) & 255)) / 2) << 16) |
               (((((firstColor >> 8) & 255) + ((secondColor >> 8) & 255)) / 2) << 8) |
               (((firstColor & 255) + (secondColor & 255)) / 2);
    if (alpha2 < alpha1)
        return 0xff000000u | (((((firstColor >> 16) & 255) * 255 + ((secondColor >> 16) & 255)) / 256) << 16) |
               (((((firstColor >> 8) & 255) * 255 + ((secondColor >> 8) & 255)) / 256) << 8) |
               (((firstColor & 255) * 255 + (secondColor & 255)) / 256);
    return 0xff000000u | (((((firstColor >> 16) & 255) + ((secondColor >> 16) & 255) * 255) / 256) << 16) |
           (((((firstColor >> 8) & 255) + ((secondColor >> 8) & 255) * 255) / 256) << 8) |
           (((firstColor & 255) + (secondColor & 255) * 255) / 256);
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
            bytes.data[index + order[0]] = (unsigned char)((unsigned int)pixels.data[y * m_width + x] >> 24);
            bytes.data[index + order[1]] = (unsigned char)((unsigned int)pixels.data[y * m_width + x] >> 16);
            bytes.data[index + order[2]] = (unsigned char)((unsigned int)pixels.data[y * m_width + x] >> 8);
            bytes.data[index + order[3]] = (unsigned char)pixels.data[y * m_width + x];
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
                        levelBytes.data[destination + order[0]] = (unsigned char)(packed >> 24);
                        levelBytes.data[destination + order[1]] = (unsigned char)(packed >> 16);
                        levelBytes.data[destination + order[2]] = (unsigned char)(packed >> 8);
                        levelBytes.data[destination + order[3]] = (unsigned char)packed;
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
                        levelBytes.data[destination + order[0]] = (unsigned char)((unsigned int)levelPixels[y * levelWidth + x] >> 24);
                        levelBytes.data[destination + order[1]] = (unsigned char)((unsigned int)levelPixels[y * levelWidth + x] >> 16);
                        levelBytes.data[destination + order[2]] = (unsigned char)((unsigned int)levelPixels[y * levelWidth + x] >> 8);
                        levelBytes.data[destination + order[3]] = (unsigned char)levelPixels[y * levelWidth + x];
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
    unsigned int level;

    m_data[0]->flip();
    if (m_mipmap && m_mipLevels > 1) {
        for (level = 1; level < m_mipLevels; ++level) {
            if (m_data[level] == NULL)
                break;
            m_data[level]->flip();
        }
    }
    if (!m_gpuAllocated) {
        gRenderer->TextureSetTextureLevels((int)m_mipLevels);
        gRenderer->TextureData(m_width, m_height, m_data[0]->getBuffer(), 0, C4JRender::eTextureFormat_RGBA8);
        if (m_mipmap && m_mipLevels > 1) {
            for (level = 1; level < m_mipLevels; ++level) {
                gRenderer->TextureData(m_width >> level, m_height >> level, m_data[level]->getBuffer(), (int)level,
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
                                             (int)level);
            }
        }
        m_updated = true;
    }
}

unsigned int Texture::getData(unsigned int mipLevel) { return (unsigned int)m_data[mipLevel]; }
unsigned int Texture::getHeight() { return (unsigned int)m_height; }
unsigned int Texture::getWidth() { return (unsigned int)m_width; }
unsigned int Texture::getManagerId() { return m_managerId; }
unsigned int Texture::getGlId() { return m_glId; }

void Texture::blit(int x, int y, Texture* texture, bool flip)
{
    if (texture == NULL)
        return;

    unsigned int level;
    for (level = 0; level < m_mipLevels; ++level)
    {
        ByteBuffer* sourceData = texture->m_data[level];
        ByteBuffer* destinationData = m_data[level];
        if (sourceData == NULL || destinationData == NULL)
            break;

        const unsigned int sourceWidth = texture->getWidth() >> level;
        const unsigned int sourceHeight = texture->getHeight() >> level;
        const unsigned int destinationWidth = (unsigned int)m_width >> level;
        const int destinationX = x >> level;
        const int destinationY = y >> level;
        unsigned char* source = sourceData->getBuffer();
        unsigned char* destination = destinationData->getBuffer();
        unsigned int row;
        for (row = 0; row < sourceHeight; ++row)
        {
            const unsigned int sourceRow = flip ? sourceHeight - row - 1 : row;
            std::memcpy(destination + ((destinationY + row) * destinationWidth + destinationX) * 4,
                        source + (sourceRow * sourceWidth) * 4, sourceWidth * 4);
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
