#include "game/core/BufferedImage.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/client/render/TextureUtil.hpp"
#include "game/core/StringConversion.hpp"
#include "game/core/DLCPack.hpp"
#include <cstdlib>
#include <cstring>
#include <cmath>

extern char* wstringtofilename(const std::wstring& value, char (&result)[256]);

BufferedImage::BufferedImage(int width, int height, int mipLevels)
    : m_width((unsigned int)width), m_height((unsigned int)height), m_mipmapDisabled(0)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    const size_t pixelCount = (size_t)width * (size_t)height;
    m_data[0] = (int*)std::malloc(pixelCount * sizeof(int));
    if (m_data[0] != NULL) std::memset(m_data[0], 0, pixelCount * sizeof(int));
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
            file = L"res" + name;
        }
        else
        {
            file = L"res" + name + suffix;
        }

        if (!pack->doesPackContainFile(DLCManager::eDLCType_All, file, true, true))
        {
            if (level == 0)
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to find %ls", 0xf4,
                                    file.c_str());
            return;
        }

        DLCFile* imageFile = pack->getFile(DLCManager::eDLCType_All, file, true, true);
        m_mipmapDisabled = imageFile->getParameterAsBool(DLCManager::eDLCParameterType_DisableMipMap);

        unsigned int imageLength = 0;
        unsigned char* imageData = imageFile->getData(imageLength);
        if (imageData == NULL || imageLength == 0)
        {
            if (level == 0)
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load %ls", 0x101,
                                    file.c_str());
            return;
        }

        int width = 0;
        int height = 0;
        if (TextureUtil::LoadTextureData(imageData, imageLength, &width, &height,
                                         &m_data[level]) != 0)
        {
            if (level == 0)
                gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load %ls", 0x112,
                                    file.c_str());
            return;
        }
        if (level == 0)
        {
            m_width = static_cast<unsigned int>(width);
            m_height = static_cast<unsigned int>(height);
        }
    }
}

BufferedImage::BufferedImage(unsigned char* data, unsigned int length)
    : m_width(0), m_height(0), m_mipmapDisabled(0)
{
    int i;
    for (i = 0; i < 10; ++i) m_data[i] = NULL;
    int width = 0;
    int height = 0;
    if (TextureUtil::LoadTextureData(data, length, &width, &height, &m_data[0]) == 0)
    {
        m_width = (unsigned int)width;
        m_height = (unsigned int)height;
    }
    else
    {
        gApp.FatalLoadError(L"<BufferedImage@%d> Failed to load texture data", 0x137);
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
    if (mipLevel >= 10) return;
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
int* BufferedImage::getData(int mipLevel) { return mipLevel < 10 ? m_data[mipLevel] : NULL; }
void* BufferedImage::getGraphics() { return NULL; }

BufferedImage* BufferedImage::getSubimage(int x, int y, int width, int height)
{
    BufferedImage* image = new BufferedImage(width, height, 0);
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
