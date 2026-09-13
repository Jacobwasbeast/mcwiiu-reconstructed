#include "game/client/render/TextureUtil.hpp"

#include <cstring>
#include <stb_image.h>
#include <zlib.h>

#include "game/core/GlStateManager.hpp"
#include "game/core/IntBuffer.hpp"
#include "game/core/BufferedImage.hpp"
#include "game/core/StbiFile.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class IntBuffer;
class Minecraft;
class StbiFile;

int ProcessLoadedTextureData(unsigned char* data, int* width, int* height, int** pixels);
int CreateFileA(const char* fileName, unsigned int desiredAccess, unsigned int shareMode, void* securityAttributes, unsigned int creationDisposition, unsigned int flagsAndAttributes, int templateFile);
unsigned int GetFileSize(int file, unsigned int* fileSizeHigh);
void CloseHandle(int file);

int ProcessLoadedTextureData(unsigned char* data, int* width, int* height, int** pixels)
{
    if (data == NULL)
    {
        return -1;
    }

    int pixelCount = *width * *height;
    unsigned int* convertedPixels = (unsigned int*)malloc(pixelCount * sizeof(unsigned int));
    *pixels = (int*)convertedPixels;
    memcpy(convertedPixels, data, pixelCount * sizeof(unsigned int));
    stbi_image_free(data);

    for (int i = 0; i < pixelCount; ++i)
    {
        TextureUtil::ByteFlipRGBAToARGB(convertedPixels[i]);
    }

    return 0;
}

static IntBuffer* dataBuffer = NULL;
static bool textureAnaglyph = false;

void TextureUtil::ByteFlipRGBAToARGB(unsigned int& colour)
{
    colour = (colour >> 8) | (colour << 24);
}

int TextureUtil::generateTextureId()
{
    return GlStateManager::genTexture();
}

void TextureUtil::releaseTextureId(int textureId)
{
    GlStateManager::deleteTexture(textureId);
}

void TextureUtil::bind(int textureId)
{
    GlStateManager::bindTexture(textureId);
}

void TextureUtil::prepareImage(int textureId, int mipLevels, int width, int height)
{
    releaseTextureId(textureId);
    bind(textureId);

    for (int level = 0; level <= mipLevels; level++)
    {
    }
}

void TextureUtil::prepareImage(int textureId, int width, int height)
{
    prepareImage(textureId, 0, width, height);
}

int TextureUtil::anaglyph(int colour)
{
    int red = (colour >> 16) & 0xff;
    int green = (colour >> 8) & 0xff;
    int blue = colour & 0xff;
    int weightedRed = red * 30;

    return (colour & 0xff000000)
        | (((weightedRed + green * 59 + blue * 11) / 100) << 16)
        | (((weightedRed + green * 70) / 100) << 8)
        | ((weightedRed + blue * 70) / 100);
}

arrayWithLength<int> TextureUtil::anaglyph(arrayWithLength<int> pixels)
{
    arrayWithLength<int> converted(pixels.length, true);

    for (unsigned int i = 0; i < pixels.length; i++)
    {
        converted.data[i] = anaglyph(pixels.data[i]);
    }

    return converted;
}

void TextureUtil::copyToBuffer(arrayWithLength<int> pixels, int offset, int length)
{
    arrayWithLength<int> source = pixels;

    if (textureAnaglyph)
    {
        source = anaglyph(pixels);
    }

    dataBuffer->clear();
    dataBuffer->put(&source, offset, length);
    dataBuffer->position(0);
    dataBuffer->limit(length);
}

void TextureUtil::uploadImageChunked(int mipLevel, arrayWithLength<int> pixels, int width, int height, int xOffset, int yOffset, bool blur, bool clamp, bool mipmap)
{
    int rowsPerChunk = 4194304 / width;
    int position = 0;

    while (position < width * height)
    {
        int remainingRows = height - position / width;
        int rows = remainingRows < rowsPerChunk ? remainingRows : rowsPerChunk;
        int count = width * rows;

        copyToBuffer(pixels, position, count);

        position += count;
    }
}

void TextureUtil::uploadImage(int textureId, arrayWithLength<int> pixels, int width, int height)
{
    bind(textureId);
    uploadImageChunked(0, pixels, width, height, 0, 0, false, false, false);
}

void* TextureUtil::decodeJpeg(void* data, int length, int* width, int* height)
{
    int imageWidth;
    int imageHeight;
    int components;
    unsigned char* pixels = stbi_load_from_memory((const unsigned char*)data, length, &imageWidth, &imageHeight, &components, 4);

    if (pixels != NULL)
    {
        *width = imageWidth;
        *height = imageHeight;

        for (int y = 0; y < imageHeight; y++)
        {
            for (int x = 0; x < imageWidth; x++)
            {
                unsigned char* pixel = pixels + (y * imageWidth + x) * 4;
                unsigned int colour = *(unsigned int*)pixel;

                pixel[0] = (unsigned char)colour;
                pixel[1] = (unsigned char)(colour >> 8);
                pixel[2] = (unsigned char)(colour >> 16);
                pixel[3] = (unsigned char)(colour >> 24);
            }
        }
    }

    return pixels;
}

void TextureUtil::insertZlibAlphaChannelData(void* pixels, int width, int height, void* compressedData, int compressedLength)
{
    unsigned char* alphaData = new unsigned char[width * height];
    unsigned long alphaLength = width * height;

    if (uncompress(alphaData, &alphaLength, (const unsigned char*)compressedData, compressedLength) == Z_OK)
    {
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                unsigned char alpha = alphaData[y * width + x];
                unsigned char* pixel = (unsigned char*)pixels + (y * width + x) * 4;
                float alphaScale = alpha / 255.0f;
                float red = pixel[1] / alphaScale;
                float green = pixel[2] / alphaScale;
                float blue = pixel[3] / alphaScale;

                if (red > 255.0f)
                {
                    red = 255.0f;
                }

                if (green > 255.0f)
                {
                    green = 255.0f;
                }

                if (blue > 255.0f)
                {
                    blue = 255.0f;
                }

                pixel[0] = alpha;
                pixel[1] = (unsigned char)red;
                pixel[2] = (unsigned char)green;
                pixel[3] = (unsigned char)blue;
            }
        }
    }

    delete[] alphaData;
}

int TextureUtil::LoadTextureData(const char* fileName, int* width, int* height, int** pixels)
{
    static stbi_io_callbacks callbacks;
    StbiFile file;

    callbacks.read = StbiFile::read;
    callbacks.skip = StbiFile::skip;
    callbacks.eof = StbiFile::eof;

    file.mFilePosition = 0;
    file.mFile = CreateFileA(fileName, 0x80000000, 0, NULL, 3, 0x80, 0);
    file.mFileSize = 0;

    if (file.mFile != -1)
    {
        file.mFileSize = GetFileSize(file.mFile, NULL);
    }

    if (file.mFile == -1)
    {
        return -1;
    }

    int components = 0;
    unsigned char* data = stbi_load_from_callbacks(&callbacks, &file, width, height, &components, 4);
    int result = ProcessLoadedTextureData(data, width, height, pixels);

    if (file.mFile != -1)
    {
        CloseHandle(file.mFile);
    }

    return result;
}

void TextureUtil::LoadTextureData(unsigned char* data, unsigned int length, int* width, int* height, int** pixels)
{
    int components = 0;
    unsigned char* imageData = stbi_load_from_memory(data, length, width, height, &components, 4);

    ProcessLoadedTextureData(imageData, width, height, pixels);
}
