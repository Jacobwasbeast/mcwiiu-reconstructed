#include "game/client/render/Texture.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/client/render/TextureManager.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/Buffer.hpp"
#include "game/core/GlStateManager.hpp"
#include <cstring>

namespace {

bool& textureGpuSynced(Texture* texture)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(texture) + 0x56);
}

bool& textureGpuAllocated(Texture* texture)
{
    return *reinterpret_cast<bool*>(reinterpret_cast<char*>(texture) + 0x84);
}

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

Texture::Texture(const std::wstring& name, int type, int width, int height, int format, int filter,
                 int wrap, int target, BufferedImage* image, bool mipmap)
    : m_glId(0), m_managerId(0), m_type(0), m_width(0), m_height(0), m_format(0), m_filter(0),
      m_wrap(0), m_target(0), m_mipmap(false), m_hasMipmaps(false), m_immediateUpdate(false),
      m_name(), m_rect(NULL), m_mipLevels(1), m_dirty(false)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    m_filter = 7;
    m_rect = new Rect2i(0, 0, 0, 0);
    _init(name, type, width, height, format, filter, wrap, target, 1, image, mipmap);
}

Texture::Texture(const std::wstring& name, int type, int width, int height, int format, int filter,
                 int wrap, int target, int mipLevels, BufferedImage* image, bool mipmap)
    : m_glId(0), m_managerId(0), m_type(0), m_width(0), m_height(0), m_format(0), m_filter(0),
      m_wrap(0), m_target(0), m_mipmap(false), m_hasMipmaps(false), m_immediateUpdate(false),
      m_name(), m_rect(NULL), m_mipLevels(1), m_dirty(false)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    m_filter = 7;
    m_rect = new Rect2i(0, 0, 0, 0);
    _init(name, type, width, height, format, filter, wrap, target, mipLevels, image, mipmap);
}

Texture::Texture(const std::wstring& name, int type, int width, int height, int format, int filter,
                 int wrap, int target, bool mipmap)
    : m_glId(0), m_managerId(0), m_type(0), m_width(0), m_height(0), m_format(0), m_filter(0),
      m_wrap(0), m_target(0), m_mipmap(false), m_hasMipmaps(false), m_immediateUpdate(false),
      m_name(), m_rect(NULL), m_mipLevels(1), m_dirty(false)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    m_filter = 7;
    m_rect = new Rect2i(0, 0, 0, 0);
    _init(name, type, width, height, format, filter, wrap, 1, 1, NULL, mipmap);
}

Texture::~Texture()
{
    int i;
    if (m_glId != (unsigned int)-1) GlStateManager::deleteTexture((int)m_glId);
    for (i = 0; i < 10; ++i) {
        delete m_data[i];
        m_data[i] = NULL;
    }
    delete m_rect;
    m_rect = NULL;
}

void Texture::_init(const std::wstring& name, int type, int width, int height, int format, int filter,
                    int wrap, int target, bool mipmap)
{
    _init(name, type, width, height, format, filter, wrap, target, 1, NULL, mipmap);
}

void Texture::_init(const std::wstring& name, int type, int width, int height, int format, int filter,
                    int wrap, int target, int mipLevels, BufferedImage* image, bool mipmap)
{
    m_name = name;
    m_type = type;
    m_width = width;
    m_height = height;
    m_format = format;
    m_filter = filter;
    m_wrap = wrap;
    m_target = target;
    m_mipmap = mipmap;
    m_mipLevels = mipLevels > 10 ? 10 : (mipLevels < 1 ? 1 : (unsigned int)mipLevels);
    m_hasMipmaps = mipmap || m_mipLevels > 1;
    m_rect->width = width;
    m_rect->height = height;
    m_managerId = TextureManager::getInstance()->createTextureID();
    if (type == 2) m_glId = (unsigned int)-1;
    else m_glId = (unsigned int)GlStateManager::genTexture();
    GlStateManager::bindTexture((int)m_glId);
    GlStateManager::texParameteri(m_target, 1, filter);
    GlStateManager::texParameteri(m_target, 2, wrap);
    GlStateManager::texParameteri(m_target, 3, filter);
    GlStateManager::texParameteri(m_target, 4, filter);
    if (image != NULL)
    {
        transferFromImage(image);
        if (type != 2) updateOnGPU();
    }
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
    if (image == NULL || image->getWidth() > (unsigned int)m_width || image->getHeight() > (unsigned int)m_height) return;

    unsigned int order[4];
    texturePackedByteOrder(m_target, order);

    unsigned int count = (unsigned int)(m_width * m_height);
    arrayWithLength<unsigned char> bytes(count * 4, false);
    arrayWithLength<int> pixels(count, false);
    image->getRGB(0, 0, (unsigned int)m_width, m_height, pixels, 0, m_width, 0);
    texturePackArgbPixels(pixels.data, count, order, bytes.data);
    delete m_data[0];
    m_data[0] = ByteBuffer::allocateDirect((int)(count * 4));
    m_data[0]->put(bytes);
    m_data[0]->position(0);
    m_data[0]->limit(count * 4);

    if (m_hasMipmaps || image->getData(1) != NULL)
    {
        m_hasMipmaps = true;
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
            delete m_data[level];
            m_data[level] = ByteBuffer::allocateDirect((int)(levelCount * 4));
            m_data[level]->put(levelBytes);
            m_data[level]->position(0);
            m_data[level]->limit(levelCount * 4);
        }
    }

    m_dirty = true;
    if (m_immediateUpdate) updateOnGPU();
}

void Texture::updateOnGPU()
{
    unsigned int level;

    m_data[0]->flip();
    if (m_hasMipmaps && m_mipLevels > 1) {
        for (level = 1; level < m_mipLevels; ++level) {
            if (m_data[level] == NULL)
                break;
            m_data[level]->flip();
        }
    }
    if (!textureGpuAllocated(this)) {
        gRenderer->TextureSetTextureLevels((int)m_mipLevels);
        gRenderer->TextureData(m_width, m_height, m_data[0]->getBuffer(), 0, C4JRender::eTextureFormat_RGBA8);
        if (m_hasMipmaps && m_mipLevels > 1) {
            for (level = 1; level < m_mipLevels; ++level) {
                gRenderer->TextureData(m_width >> level, m_height >> level, m_data[level]->getBuffer(), (int)level,
                                       C4JRender::eTextureFormat_RGBA8);
            }
        }
        textureGpuSynced(this) = true;
        textureGpuAllocated(this) = true;
    } else {
        gRenderer->TextureDataUpdate(0, 0, m_width, m_height, m_data[0]->getBuffer(), 0);
        if (m_hasMipmaps && gRenderer->TextureGetTextureLevels() > 1 && m_mipLevels > 1) {
            for (level = 1; level < m_mipLevels; ++level) {
                gRenderer->TextureDataUpdate(0, 0, m_width >> level, m_height >> level, m_data[level]->getBuffer(),
                                             (int)level);
            }
        }
        textureGpuSynced(this) = true;
    }
    m_dirty = false;
}

unsigned int Texture::getData(unsigned int mipLevel) { return (unsigned int)m_data[mipLevel]; }
unsigned int Texture::getHeight() { return (unsigned int)m_height; }
unsigned int Texture::getWidth() { return (unsigned int)m_width; }
unsigned int Texture::getManagerId() { return m_managerId; }
unsigned int Texture::getGlId() { return m_glId; }
Rect2i* Texture::getRect() { return m_rect; }
std::wstring Texture::getName() { return m_name; }
void Texture::setImmediateUpdate(bool immediate) { m_immediateUpdate = immediate; }

void Texture::blit(int x, int y, Texture* texture) { blit(x, y, texture, false); }
void Texture::blit(int x, int y, Texture* texture, bool flip)
{
    if (texture == NULL || m_data[0] == NULL || texture->m_data[0] == NULL) return;
    unsigned int rows = texture->getHeight();
    unsigned int cols = texture->getWidth();
    unsigned int row;
    unsigned char* src = texture->m_data[0]->getBuffer();
    unsigned char* dst = m_data[0]->getBuffer();
    unsigned int stride = (unsigned int)m_width * 4;
    for (row = 0; row < rows; ++row) {
        unsigned int sourceRow = flip ? rows - row - 1 : row;
        if (y + (int)row < 0 || y + (int)row >= m_height) continue;
        if (x < 0 || x + (int)cols > m_width) continue;
        std::memcpy(dst + (y + row) * stride + x * 4, src + sourceRow * cols * 4, cols * 4);
    }
    m_dirty = true;
    if (m_immediateUpdate) updateOnGPU();
}

void Texture::bind(int textureUnit)
{
    GlStateManager::enableTexture();
    GlStateManager::activeTexture(textureUnit);
    GlStateManager::bindTexture((int)m_glId);
    if (m_dirty) updateOnGPU();
}

void Texture::fill(const Rect2i* rect, int color)
{
    if (rect == NULL || m_data[0] == NULL) return;
    unsigned char* data = m_data[0]->getBuffer();
    int y;
    for (y = rect->y; y < rect->y + rect->height; ++y) {
        int x;
        if (y < 0 || y >= m_height) continue;
        for (x = rect->x; x < rect->x + rect->width; ++x) if (x >= 0 && x < m_width)
            std::memcpy(data + (y * m_width + x) * 4, &color, 4);
    }
    m_dirty = true;
}

void Texture::writeAsBMP(const std::wstring& fileName) { (void)fileName; }
void Texture::writeAsPNG(const std::wstring& fileName) { (void)fileName; }
void Texture::transferFromBuffer(arrayWithLength<int> pixels)
{
    if (pixels.data == NULL) return;
    arrayWithLength<unsigned char> bytes(pixels.length * 4, false);
    std::memcpy(bytes.data, pixels.data, pixels.length * 4);
    delete m_data[0];
    m_data[0] = ByteBuffer::allocateDirect((int)bytes.length);
    m_data[0]->put(bytes);
    m_data[0]->limit(bytes.length);
    m_dirty = true;
}
