#include "game/client/render/Texture.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TextureManager.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/Buffer.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/MemSect.hpp"
#include <cstring>
#include <algorithm>

namespace {

void texturePackedByteOrder(int target, unsigned int order[4])
{
    order[0] = 3;
    if (target == 1)
    {
        order[1] = 2;
        order[2] = 1;
        order[3] = 0;
    }
    else
    {
        order[1] = 0;
        order[2] = 1;
        order[3] = 2;
    }
}

void texturePackArgbPixels(const int* pixels, unsigned int count, const unsigned int order[4], unsigned char* bytes)
{
    unsigned int i;
    for (i = 0; i < count; ++i)
    {
        unsigned int argb = (unsigned int)pixels[i];
        bytes[i * 4 + order[0]] = (unsigned char)(argb >> 24);
        bytes[i * 4 + order[1]] = (unsigned char)(argb >> 16);
        bytes[i * 4 + order[2]] = (unsigned char)(argb >> 8);
        bytes[i * 4 + order[3]] = (unsigned char)argb;
    }
}

unsigned int textureAverageChannel(unsigned char a, unsigned char b, unsigned char c, unsigned char d)
{
    return (unsigned int)(((unsigned int)a + b + c + d) >> 2);
}

unsigned int texturePackedArgb(const unsigned char* pixel, const unsigned int order[4])
{
    return ((unsigned int)pixel[order[0]] << 24) | ((unsigned int)pixel[0] << 16) |
           ((unsigned int)pixel[1] << 8) | (unsigned int)pixel[2];
}

void textureGenerateMipLevel(ByteBuffer* previous, unsigned int previousWidth, unsigned char* levelBytes,
                              unsigned int levelWidth, unsigned int levelHeight, const unsigned int order[4])
{
    unsigned char* previousBuffer = previous->getBuffer();
    unsigned int y;
    for (y = 0; y < levelHeight; ++y)
    {
        unsigned int x;
        for (x = 0; x < levelWidth; ++x)
        {
            const unsigned int topLeft = ((y * 2) * previousWidth + x * 2) * 4;
            const unsigned int topRight = topLeft + 4;
            const unsigned int bottomLeft = topLeft + previousWidth * 4;
            const unsigned int bottomRight = bottomLeft + 4;
            const unsigned char* pTopLeft = previousBuffer + topLeft;
            const unsigned char* pTopRight = previousBuffer + topRight;
            const unsigned char* pBottomLeft = previousBuffer + bottomLeft;
            const unsigned char* pBottomRight = previousBuffer + bottomRight;
            const unsigned char alphaTopLeft = pTopLeft[order[0]];
            const unsigned char alphaTopRight = pTopRight[order[0]];
            const unsigned char alphaBottomLeft = pBottomLeft[order[0]];
            const unsigned char alphaBottomRight = pBottomRight[order[0]];
            const unsigned int dst = (y * levelWidth + x) * 4;
            const bool uniform = (alphaTopLeft == 0 || alphaTopLeft == 0xff) &&
                                  (alphaTopRight == 0 || alphaTopRight == 0xff) &&
                                  (alphaBottomLeft == 0 || alphaBottomLeft == 0xff) &&
                                  (alphaBottomRight == 0 || alphaBottomRight == 0xff);
            if (uniform)
            {
                levelBytes[dst + order[0]] = (unsigned char)textureAverageChannel(alphaTopLeft, alphaTopRight, alphaBottomLeft, alphaBottomRight);
                levelBytes[dst + order[1]] = (unsigned char)textureAverageChannel(pTopLeft[0], pTopRight[0], pBottomLeft[0], pBottomRight[0]);
                levelBytes[dst + order[2]] = (unsigned char)textureAverageChannel(pTopLeft[1], pTopRight[1], pBottomLeft[1], pBottomRight[1]);
                levelBytes[dst + order[3]] = (unsigned char)textureAverageChannel(pTopLeft[2], pTopRight[2], pBottomLeft[2], pBottomRight[2]);
            }
            else
            {
                const unsigned int argbTopLeft = texturePackedArgb(pTopLeft, order);
                const unsigned int argbTopRight = texturePackedArgb(pTopRight, order);
                const unsigned int argbBottomLeft = texturePackedArgb(pBottomLeft, order);
                const unsigned int argbBottomRight = texturePackedArgb(pBottomRight, order);
                const unsigned int top = Texture::crispBlend((int)argbTopLeft, (int)argbTopRight);
                const unsigned int bottom = Texture::crispBlend((int)argbBottomLeft, (int)argbBottomRight);
                const unsigned int blended = Texture::crispBlend((int)top, (int)bottom);
                levelBytes[dst + order[0]] = (unsigned char)(blended >> 24);
                levelBytes[dst + order[1]] = (unsigned char)(blended >> 16);
                levelBytes[dst + order[2]] = (unsigned char)(blended >> 8);
                levelBytes[dst + order[3]] = (unsigned char)blended;
            }
        }
    }
}

}

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
    unsigned int imageWidth = image->getWidth();
    unsigned int imageHeight = image->getHeight();
    if (imageWidth > (unsigned int)m_width || imageHeight > (unsigned int)m_height) return;

    unsigned int order[4];
    texturePackedByteOrder(m_format, order);

    unsigned int count = (unsigned int)(m_width * m_height);
    arrayWithLength<int> pixels(count, false);
    image->getRGB(0, 0, (unsigned int)m_width, m_height, pixels, 0, (int)imageWidth, 0);

    arrayWithLength<unsigned char> bytes(count * 4, false);
    texturePackArgbPixels(pixels.data, count, order, bytes.data);

    int i;
    for (i = 0; i < 10; ++i) { delete m_data[i]; m_data[i] = NULL; }

    MemSect(0x33);
    m_data[0] = ByteBuffer::allocateDirect((int)(count * 4));
    MemSect(0);
    m_data[0]->clear();
    m_data[0]->put(bytes);
    m_data[0]->limit(count * 4);
    delete[] bytes.data;

    if (m_mipmap || image->getData(1) != NULL)
    {
        m_mipmap = true;
        unsigned int level;
        for (level = 1; level <= 4; ++level)
        {
            unsigned int levelWidth = (unsigned int)m_width >> level;
            unsigned int levelHeight = (unsigned int)m_height >> level;
            unsigned int levelCount = levelWidth * levelHeight;
            arrayWithLength<unsigned char> levelBytes(levelCount * 4, false);
            int* providedData = image->getData((int)level);
            if (providedData == NULL)
            {
                unsigned int previousWidth = (unsigned int)m_width >> (level - 1);
                textureGenerateMipLevel(m_data[level - 1], previousWidth, levelBytes.data, levelWidth, levelHeight, order);
            }
            else
            {
                texturePackArgbPixels(providedData, levelCount, order, levelBytes.data);
            }
            MemSect(0x33);
            m_data[level] = ByteBuffer::allocateDirect((int)(levelCount * 4));
            MemSect(0);
            m_data[level]->clear();
            m_data[level]->put(levelBytes);
            m_data[level]->limit(levelCount * 4);
            delete[] levelBytes.data;
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
