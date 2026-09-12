#include "game/client/render/Texture.hpp"
#include "game/client/render/Renderer.hpp"
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
    if (type == 2) m_glId = (unsigned int)-1;
    else m_glId = (unsigned int)GlStateManager::genTexture();
    GlStateManager::bindTexture((int)m_glId);
    GlStateManager::texParameteri(m_target, 1, filter);
    GlStateManager::texParameteri(m_target, 2, wrap);
    GlStateManager::texParameteri(m_target, 3, filter);
    GlStateManager::texParameteri(m_target, 4, filter);
    if (image != NULL) transferFromImage(image);
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
    unsigned int count = (unsigned int)(m_width * m_height);
    arrayWithLength<unsigned char> bytes(count * 4, false);
    arrayWithLength<int> pixels(count, false);
    image->getRGB(0, 0, (unsigned int)m_width, m_height, pixels, 0, m_width, 0);
    std::memcpy(bytes.data, pixels.data, count * 4);
    delete m_data[0];
    m_data[0] = ByteBuffer::allocateDirect((int)(count * 4));
    m_data[0]->put(bytes);
    m_data[0]->limit(count * 4);
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
