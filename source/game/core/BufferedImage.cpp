#include "game/core/BufferedImage.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/client/render/TextureUtil.hpp"
#include "game/core/StringConversion.hpp"
#include <cstdlib>
#include <cstring>
#include <cmath>

extern char* wstringtofilename(const std::wstring& value, char (&result)[256]);

BufferedImage::BufferedImage(int mipLevels, int width, int height)
    : m_width((unsigned int)width), m_height((unsigned int)height), m_mipmapDisabled(0)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    m_data[0] = (int*)std::malloc((size_t)width * (size_t)height * 4u);
    if (m_data[0] != NULL) std::memset(m_data[0], 0, (size_t)width * (size_t)height * 4u);
}

BufferedImage::BufferedImage(const std::wstring& name, bool mipmapDisabled, bool useTitleUpdate, const std::wstring& path)
    : m_width(0), m_height(0), m_mipmapDisabled(0)
{
    std::wstring base = path;
    if (base.empty())
    {
        base = useTitleUpdate ? L"Common\\res\\TitleUpdate\\" : L"Common\\";
    }

    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    m_mipmapDisabled = 0;

    const int levels = mipmapDisabled ? 1 : 10;
    int level;
    for (level = 0; level < levels; ++level)
    {
        std::wstring suffix;
        if (level != 0)
        {
            suffix = std::wstring(L"MipMapLevel") + _toString(level + 1);
        }

        std::wstring file;
        if (mipmapDisabled)
        {
            file = base + L"res" + name;
        }
        else
        {
            file = base + L"res" + name.substr(0, name.length() - 4) + suffix + L".png";
        }

        char filename[256];
        char* resolved = wstringtofilename(file, filename);
        int width = 0;
        int height = 0;
        if (TextureUtil::LoadTextureData(resolved, &width, &height, &m_data[level]) != 0)
        {
            if (level == 0)
            {
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load texture %ls", 0xc3,
                                    convStringToWstring(std::string(resolved)).c_str());
            }
            return;
        }
        if (level == 0)
        {
            m_width = (unsigned int)width;
            m_height = (unsigned int)height;
        }
    }
}

BufferedImage::BufferedImage(DLCPack* pack, const std::wstring& name, bool mipmapDisabled)
    : m_width(0), m_height(0), m_mipmapDisabled(0)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    m_mipmapDisabled = mipmapDisabled ? 1 : 0;
    (void)pack;
    (void)name;
}

BufferedImage::BufferedImage(unsigned char* data, unsigned int length)
    : m_width(0), m_height(0), m_mipmapDisabled(0)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    int* pixels = NULL;
    int width = 0;
    int height = 0;
    TextureUtil::LoadTextureData(data, length, &width, &height, &pixels);
    if (pixels != NULL)
    {
        m_data[0] = pixels;
        m_width = (unsigned int)width;
        m_height = (unsigned int)height;
    }
    else
    {
        gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load", 0x137);
    }
}

BufferedImage::~BufferedImage()
{
    int i;
    for (i = 0; i < 10; ++i) std::free(m_data[i]);
}

unsigned int BufferedImage::getWidth() { return m_width; }
unsigned int BufferedImage::getHeight() { return m_height; }

void BufferedImage::getRGB(int x, int y, unsigned int width, int height, arrayWithLength<int> rgbArray, int offset, int scansize, unsigned int mipLevel)
{
    int sourceWidth = (int)(m_width >> mipLevel);
    int* source = m_data[mipLevel];
    if (source == NULL || rgbArray.data == NULL) return;
    int row;
    for (row = 0; row < height; ++row)
    {
        std::memcpy(rgbArray.data + offset + row * scansize, source + (y + row) * sourceWidth + x, width * sizeof(int));
    }
}

int* BufferedImage::getData() { return m_data[0]; }
int* BufferedImage::getData(int mipLevel) { return mipLevel >= 0 && mipLevel < 10 ? m_data[mipLevel] : NULL; }
void* BufferedImage::getGraphics() { return NULL; }

BufferedImage* BufferedImage::getSubimage(int x, int y, int width, int height)
{
    BufferedImage* image = new BufferedImage(0, width, height);
    arrayWithLength<int> pixels((unsigned int)width * (unsigned int)height, false);
    getRGB(x, y, (unsigned int)width, height, pixels, 0, width, 0);
    if (image->m_data[0] != NULL && pixels.data != NULL)
        std::memcpy(image->m_data[0], pixels.data, (size_t)width * (size_t)height * sizeof(int));
    return image;
}

unsigned char BufferedImage::isMipmapDisabled() { return m_mipmapDisabled; }

void BufferedImage::preMultiplyAlpha()
{
    unsigned int count = m_width * m_height;
    unsigned int i;
    for (i = 0; i < count; ++i)
    {
        unsigned int p = (unsigned int)m_data[0][i];
        unsigned int a = (p >> 24) & 255u;
        unsigned int r = (unsigned int)((((p >> 16) & 255u) * a) / 255u);
        unsigned int g = (unsigned int)((((p >> 8) & 255u) * a) / 255u);
        unsigned int b = (unsigned int)(((p & 255u) * a) / 255u);
        m_data[0][i] = (int)((a << 24) | (r << 16) | (g << 8) | b);
    }
}

void BufferedImage::ByteFlip4(unsigned int& value)
{
    value = ((value & 0x000000ffu) << 24) | ((value & 0x0000ff00u) << 8) | ((value & 0x00ff0000u) >> 8) | ((value & 0xff000000u) >> 24);
}

int BufferedImage::getTransparency() { return 3; }
