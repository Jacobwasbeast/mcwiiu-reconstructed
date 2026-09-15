#include "game/client/render/Textures.hpp"

#include <cstring>

#include "game/core/BufferedImage.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/MemoryTracker.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/client/render/MemTextureProcessor.hpp"
#include "game/client/render/MemTexture.hpp"
#include "game/client/render/Texture.hpp"
#include "game/util/ResourceLocation.hpp"
#include "game/client/render/TextureAtlas.hpp"
#include "game/client/render/TexturePackRepository.hpp"
#include "game/entity/mob/ItemEntity.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

static int sIndexedTextures[256];
ResourceLocation BLOCKS_LOCATION((_TEXTURE_NAME)241);
ResourceLocation ITEM_LOCATION((_TEXTURE_NAME)240);
static bool sTextureMipmap = true;

namespace
{
struct MemTextureFields
{
    BufferedImage* m_image;
    int m_referenceCount;
    int m_textureId;
    bool m_loaded;
    int m_textureFormat;
    bool m_textureReady;
};

MemTextureFields& memTextureFields(MemTexture* texture)
{
    return *reinterpret_cast<MemTextureFields*>(texture);
}

unsigned int rotateChannelsRight(unsigned int value)
{
    return (value >> 8) | (value << 24);
}

unsigned int rotateChannelsLeft(unsigned int value)
{
    return (value << 8) | (value >> 24);
}

int mipLevelsFor(int dimension)
{
    int levels = 1;
    if (dimension > 16)
    {
        do
        {
            ++levels;
        } while ((8 << levels) < dimension);
    }
    return levels;
}
}

Textures::Textures(TexturePackRepository* texturePackRepository, Options* options)
    : mOptions(options), mTexturePackRepository(texturePackRepository), mMissingImage(NULL),
      mBlocks(NULL), mItems(NULL), mLastBoundId(-1), mTextureFormat(false),
      mReloading(false), mLoaded(false)
{
    InitializeCriticalSection(&mMutex);
    mMissingImage = new BufferedImage(16, 16, 3);
    mBlocks = new TextureAtlas(0, L"terrain", L"textures_blocks_", mMissingImage, true);
    mItems = new TextureAtlas(1, L"items", L"textures_items_", mMissingImage, true);
    loadIndexedTextures();
}

bool Textures::IsTUImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    static const int names[] = { 0x2f, 0x33, 0x34, 0x20 };
    unsigned int i;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
    {
        if ((int)textureName == names[i]) return true;
    }
    static const wchar_t* const suffixes[] = { L"terrain", L"items" };
    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i)
    {
        if (name == suffixes[i]) return true;
    }
    return false;
}

bool Textures::IsOriginalImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    static const int names[] = { 0x26, 0x27, 0x28, 0x29 };
    unsigned int i;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
    {
        if ((int)textureName == names[i]) return true;
    }
    static const wchar_t* const suffixes[] = { L"gui_gui", L"gui_icons", L"misc_footprint" };
    for (i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); ++i)
    {
        if (name == suffixes[i]) return true;
    }
    return false;
}

BufferedImage* Textures::readImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    MemSect(32);
    bool titleUpdate = IsTUImage(textureName, name);
    std::wstring path = L"";
    bool useSelected = false;
    if (mTexturePackRepository->getSelected() != TexturePackRepository::defaultTexturePack)
    {
        std::wstring resource = L"res/" + name;
        useSelected = mTexturePackRepository->getSelected()->hasFile(resource, false);
    }
    BufferedImage* image;
    if (useSelected)
    {
        path = mTexturePackRepository->getSelected()->getPath(titleUpdate, NULL);
        image = mTexturePackRepository->getSelected()->getImageResource(name, false, titleUpdate, path);
    }
    else
    {
        TexturePack* defaultPack = TexturePackRepository::defaultTexturePack;
        path = defaultPack->getPath(titleUpdate, NULL);
        if (!IsOriginalImage(textureName, name) && !titleUpdate)
        {
            image = defaultPack->getImageResource(L"1_2_2/" + name, false, false, path);
        }
        else
        {
            image = defaultPack->getImageResource(name, false, titleUpdate, path);
        }
    }
    MemSect(0);
    return image;
}

void Textures::setTextureFormat(const std::wstring& name)
{
    mTextureFormat = false;
    sTextureMipmap = true;
    if (name == L"environment_clouds" || name == L"misc_shadow" ||
        name == L"misc_pumpkinblur" || name == L"gui_icons" ||
        name == L"gui_gui" || name == L"misc_footprint" ||
        name == L"mob_shulker_endergolem" || name == L"item_bed" ||
        name == L"font_Default")
    {
        sTextureMipmap = false;
    }
    if (name.size() >= 6 && name.compare(0, 6, L"armor_") == 0 &&
        name != L"armor_power")
    {
        sTextureMipmap = false;
    }
}

arrayWithLength<int> Textures::anaglyph(arrayWithLength<int> pixels)
{
    arrayWithLength<int> result(pixels.length, true);
    unsigned int i;
    for (i = 0; i < pixels.length; ++i)
    {
        unsigned int value = (unsigned int)pixels.data[i];
        unsigned int red = (value >> 16) & 255;
        unsigned int green = (value >> 8) & 255;
        unsigned int blue = value & 255;
        result.data[i] = (int)((value & 0xff000000u) |
            (((red * 30 + green * 59 + blue * 11) / 100) << 16) |
            (((red * 30 + green * 70) / 100) << 8) |
            ((red * 30 + blue * 70) / 100));
    }
    return result;
}

void Textures::loadTexture(BufferedImage* image, int texture, bool blur, bool clamp)
{
    MemSect(0x21);
    GlStateManager::bindTexture(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    if (blur)
    {
        GlStateManager::texParameteri(1, 1);
        GlStateManager::texParameteri(2, 1);
    }
    GlStateManager::texParameteri(3, clamp ? 0 : 1);
    GlStateManager::texParameteri(4, clamp ? 0 : 1);
    int width = (int)image->getWidth();
    int height = (int)image->getHeight();
    arrayWithLength<int> pixels((unsigned int)(width * height), true);
    image->getRGB(0, 0, width, height, pixels, 0, width, 0);
    if (mOptions != NULL && *(reinterpret_cast<unsigned char*>(mOptions) + 0x15) != 0)
    {
        pixels = anaglyph(pixels);
    }
    ByteBuffer* buffer = MemoryTracker::createByteBuffer((int)(pixels.length * 4));
    buffer->clear();
    arrayWithLength<unsigned char> bytes((unsigned int)(pixels.length * 4), true);
    unsigned int i;
    for (i = 0; i < pixels.length && i * 4 + 3 < bytes.length; ++i)
    {
        unsigned int value = (unsigned int)pixels.data[i];
        bytes.data[i * 4] = (unsigned char)(value >> 16);
        bytes.data[i * 4 + 1] = (unsigned char)(value >> 8);
        bytes.data[i * 4 + 2] = (unsigned char)value;
        bytes.data[i * 4 + 3] = (unsigned char)(value >> 24);
    }
    buffer->put(bytes);
    buffer->position(0);
    buffer->limit(bytes.length);
    int levels = 1;
    if (sTextureMipmap)
    {
        int levelsW = mipLevelsFor(width);
        int levelsH = mipLevelsFor(height);
        levels = levelsH < levelsW ? levelsH : levelsW;
        if (levels > 5) levels = 5;
    }
    GlStateManager::texImage2D(0, 0, 0, width, height, 0, 0, 0, buffer);
    if (sTextureMipmap)
    {
        int* sourceData = pixels.data;
        int sourceWidth = width;
        bool ownsSourceData = false;
        int level;
        for (level = 1; level < levels; ++level)
        {
            int mipWidth = width >> level;
            int mipHeight = height >> level;
            unsigned int mipCount = (unsigned int)(mipWidth * mipHeight);
            arrayWithLength<int> mip(mipCount, true);
            int* precomputed = image->getData(level);
            if (precomputed != NULL)
            {
                memcpy(mip.data, precomputed, mipCount * 4);
                unsigned int j;
                for (j = 0; j < mipCount; ++j)
                {
                    mip.data[j] = (int)rotateChannelsLeft((unsigned int)mip.data[j]);
                }
            }
            else
            {
                int row, col;
                for (row = 0; row < mipHeight; ++row)
                {
                    for (col = 0; col < mipWidth; ++col)
                    {
                        int r0 = (row * 2) * sourceWidth;
                        int r1 = (row * 2 + 1) * sourceWidth;
                        int c0 = col * 2;
                        int c1 = col * 2 + 1;
                        unsigned int a = rotateChannelsRight((unsigned int)sourceData[r0 + c0]);
                        unsigned int b = rotateChannelsRight((unsigned int)sourceData[r0 + c1]);
                        unsigned int c = rotateChannelsRight((unsigned int)sourceData[r1 + c0]);
                        unsigned int d = rotateChannelsRight((unsigned int)sourceData[r1 + c1]);
                        unsigned int top = Texture::crispBlend((int)a, (int)b);
                        unsigned int bottom = Texture::crispBlend((int)c, (int)d);
                        unsigned int blended = Texture::crispBlend((int)top, (int)bottom);
                        mip.data[row * mipWidth + col] = (int)rotateChannelsLeft(blended);
                    }
                }
            }
            ByteBuffer* mipBuffer = MemoryTracker::createByteBuffer((int)(mipCount * 4));
            mipBuffer->clear();
            arrayWithLength<unsigned char> mipBytes(mipCount * 4, true);
            unsigned int k;
            for (k = 0; k < mipCount; ++k)
            {
                unsigned int value = (unsigned int)mip.data[k];
                mipBytes.data[k * 4] = (unsigned char)(value >> 16);
                mipBytes.data[k * 4 + 1] = (unsigned char)(value >> 8);
                mipBytes.data[k * 4 + 2] = (unsigned char)value;
                mipBytes.data[k * 4 + 3] = (unsigned char)(value >> 24);
            }
            mipBuffer->put(mipBytes);
            mipBuffer->position(0);
            mipBuffer->limit(mipBytes.length);
            GlStateManager::texImage2D(0, level, 0, mipWidth, mipHeight, 0, 0, 0, mipBuffer);
            delete mipBuffer;
            if (ownsSourceData) delete[] sourceData;
            sourceData = mip.data;
            sourceWidth = mipWidth;
            ownsSourceData = true;
        }
        if (ownsSourceData) delete[] sourceData;
    }
    delete[] pixels.data;
    delete buffer;
    MemSect(0);
}

int Textures::loadTexture(_TEXTURE_NAME textureName, const std::wstring& name)
{
    iterator found = find(name);
    if (found != end()) return found->second;
    setTextureFormat(name);
    int texture = MemoryTracker::genTextures();
    bool blur = name.compare(0, 6, L"_blur_") == 0;
    std::wstring workingName = blur ? name.substr(6) : name;
    bool clamp = name.compare(0, 7, L"_clamp_") == 0;
    if (clamp) workingName = name.substr(7);
    BufferedImage* image = readImage(textureName, workingName);
    if (image->isMipmapDisabled()) sTextureMipmap = false;
    if (image->getHeight() > 0 && image->getWidth() > 0)
    {
        loadTexture(image, texture, blur, clamp);
    }
    delete image;
    (*this)[name] = texture;
    return texture;
}

void Textures::loadIndexedTextures()
{
    int i;
    for (i = 0; i < 120; i += 2)
    {
        sIndexedTextures[i] = loadTexture((_TEXTURE_NAME)i, L"terrain");
        sIndexedTextures[i + 1] = loadTexture((_TEXTURE_NAME)(i + 1), L"items");
    }
}

arrayWithLength<int> Textures::loadTexturePixels(BufferedImage* image, arrayWithLength<int> pixels)
{
    image->getRGB(0, 0, image->getWidth(), (int)image->getHeight(), pixels, 0, (int)image->getWidth(), 0);
    return pixels;
}

arrayWithLength<int> Textures::loadTexturePixels(BufferedImage* image)
{
    arrayWithLength<int> pixels(image->getWidth() * image->getHeight(), true);
    return loadTexturePixels(image, pixels);
}

arrayWithLength<int> Textures::loadTexturePixels(_TEXTURE_NAME textureName, const std::wstring& name)
{
    arrayWithLength<int>& pixels = mTexturePixels[name];
    if (pixels.length != 0) return pixels;
    BufferedImage* image = readImage(textureName, name);
    pixels = loadTexturePixels(image);
    delete image;
    return pixels;
}

void Textures::bind(int texture)
{
    if (texture >= 0) GlStateManager::bindTexture(texture);
}

void Textures::bindTexture(const std::wstring& name)
{
    bind(loadTexture((_TEXTURE_NAME)0xf2, name));
}

int Textures::loadTexture(int textureName)
{
    if (textureName == -1) return 0;
    if (textureName == 0xf1)
    {
        mBlocks->getStitchedTexture()->bind(0);
        return (int)mBlocks->getStitchedTexture()->getGlId();
    }
    if (textureName == 0xf0)
    {
        mItems->getStitchedTexture()->bind(0);
        return (int)mItems->getStitchedTexture()->getGlId();
    }
    return sIndexedTextures[textureName];
}

void Textures::bindTexture(const ResourceLocation* location)
{
    if (location == NULL) return;
    if (location->isPreloaded()) bind(loadTexture(location->getTexture()));
    else bind(loadTexture((_TEXTURE_NAME)0xf2, location->getPath()));
}

void Textures::bindTextureLayers(const ResourceLocation* location)
{
    int count = location->getTextureCount();
    int i;
    for (i = 0; i < count; ++i)
    {
        loadTexture(location->getTexture(i));
    }
}

ResourceLocation* Textures::getTextureLocation(int textureName)
{
    if (textureName == 0) return &BLOCKS_LOCATION;
    if (textureName == 1) return &ITEM_LOCATION;
    return NULL;
}

ResourceLocation* Textures::getTextureLocation(boost::shared_ptr<Entity>& entity)
{
    boost::shared_ptr<ItemEntity> item = boost::dynamic_pointer_cast<ItemEntity>(entity);
    not_null_ptr<ItemInstance> instance = item->getItem();
    return getTextureLocation(instance->getItem()->getIconType());
}

void Textures::clearLastBoundId()
{
    mLastBoundId = -1;
}

void Textures::loadTexture(BufferedImage* image, int texture)
{
    loadTexture(image, texture, false, false);
}

int Textures::getTexture(BufferedImage* image, C4JRender::eTextureFormat format, bool blur)
{
    int texture = MemoryTracker::genTextures();
    sTextureMipmap = blur;
    loadTexture(image, texture, blur, false);
    mTextureImages[texture] = image;
    return texture;
}

void Textures::replaceTexture(arrayWithLength<int> pixels, int width, int height, int texture)
{
    bind(texture);
    if (mOptions != NULL) pixels = anaglyph(pixels);
    ByteBuffer* buffer = MemoryTracker::createByteBuffer(width * height * 4);
    arrayWithLength<unsigned char> bytes((unsigned int)(width * height * 4), true);
    unsigned int i;
    for (i = 0; i < pixels.length && i * 4 + 3 < bytes.length; ++i)
    {
        unsigned int value = (unsigned int)pixels.data[i];
        bytes.data[i * 4] = (unsigned char)(value >> 16);
        bytes.data[i * 4 + 1] = (unsigned char)(value >> 8);
        bytes.data[i * 4 + 2] = (unsigned char)value;
        bytes.data[i * 4 + 3] = (unsigned char)(value >> 24);
    }
    buffer->put(bytes);
    buffer->position(0);
    buffer->limit(bytes.length);
    GlStateManager::texImage2D(0, 0, 0, width, height, 0, 0, 0, buffer);
}

void Textures::replaceTextureDirect(arrayWithLength<int> pixels, int width, int height, int texture)
{
    bind(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
    ByteBuffer* buffer = MemoryTracker::createByteBuffer(width * height * 4);
    arrayWithLength<unsigned char> bytes((unsigned int)(width * height * 4), true);
    unsigned int i;
    for (i = 0; i < pixels.length && i * 4 + 3 < bytes.length; ++i)
    {
        unsigned int value = (unsigned int)pixels.data[i];
        bytes.data[i * 4] = (unsigned char)(value >> 16);
        bytes.data[i * 4 + 1] = (unsigned char)(value >> 8);
        bytes.data[i * 4 + 2] = (unsigned char)value;
        bytes.data[i * 4 + 3] = (unsigned char)(value >> 24);
    }
    buffer->put(bytes);
    buffer->position(0);
    buffer->limit(bytes.length);
    GlStateManager::texImage2D(0, 0, 0, width, height, 0, 0, 0, buffer);
    delete buffer;
}

void Textures::replaceTextureDirect(arrayWithLength<short> pixels, int width, int height, int texture)
{
    bind(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    GlStateManager::texParameteri(3, 1);
    GlStateManager::texParameteri(4, 1);
    GlStateManager::texImage2D(0, 0, 0, width, height, 0, 0, 0, NULL);
}

void Textures::releaseTexture(int texture)
{
    mTextureImages.erase(texture);
    GlStateManager::deleteTexture(texture);
}

int Textures::addMemTexture(const std::wstring& name, MemTextureProcessor* processor)
{
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end() && found->second != NULL)
    {
        LeaveCriticalSection(&mMutex);
        return (int)found->second;
    }
    unsigned char* data = NULL;
    unsigned int size = 0;
    g_pApp->GetMemFileDetails(name, &data, &size);
    if (size == 0)
    {
        mMemTextures[name] = NULL;
        LeaveCriticalSection(&mMutex);
        return 0;
    }
    MemTexture* texture = new MemTexture(name, data, size, processor);
    mMemTextures[name] = texture;
    LeaveCriticalSection(&mMutex);
    return (int)texture;
}

int Textures::loadMemTexture(const std::wstring& name, const std::wstring& path)
{
    MemTexture* texture = NULL;
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end()) texture = found->second;
    LeaveCriticalSection(&mMutex);
    if (texture == NULL)
    {
        if (!g_pApp->IsFileInMemoryTextures(name))
        {
            return path.empty() ? -1 : loadTexture((_TEXTURE_NAME)0xf2, path);
        }
        texture = (MemTexture*)addMemTexture(name, NULL);
        if (texture == NULL) return path.empty() ? -1 : loadTexture((_TEXTURE_NAME)0xf2, path);
    }
    MemTextureFields& fields = memTextureFields(texture);
    fields.m_textureFormat = 0;
    int result;
    if (fields.m_image == NULL || fields.m_loaded)
    {
        result = fields.m_textureId;
    }
    else
    {
        if (name == L"dlcskin" || name == L"dlccape") sTextureMipmap = false;
        if (fields.m_textureId < 0)
        {
            result = getTexture(fields.m_image, (C4JRender::eTextureFormat)0, sTextureMipmap);
            fields.m_textureId = result;
            fields.m_loaded = true;
            sTextureMipmap = true;
        }
        else
        {
            loadTexture(fields.m_image, fields.m_textureId);
            result = fields.m_textureId;
            sTextureMipmap = true;
            fields.m_loaded = true;
        }
    }
    if (result >= 0) return result;
    return path.empty() ? -1 : loadTexture((_TEXTURE_NAME)0xf2, path);
}

int Textures::loadMemTexture(const std::wstring& name, int texture)
{
    MemTexture* memTex = NULL;
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end()) memTex = found->second;
    LeaveCriticalSection(&mMutex);
    if (memTex == NULL)
    {
        if (!g_pApp->IsFileInMemoryTextures(name)) return loadTexture(texture);
        memTex = (MemTexture*)addMemTexture(name, NULL);
        if (memTex == NULL) return loadTexture(texture);
    }
    MemTextureFields& fields = memTextureFields(memTex);
    fields.m_textureFormat = 0;
    int result;
    if (fields.m_image == NULL || fields.m_loaded)
    {
        result = fields.m_textureId;
    }
    else
    {
        if (name == L"dlcskin" || name == L"dlccape") sTextureMipmap = false;
        if (fields.m_textureId < 0)
        {
            result = getTexture(fields.m_image, (C4JRender::eTextureFormat)0, sTextureMipmap);
            fields.m_textureId = result;
            fields.m_loaded = true;
            sTextureMipmap = true;
        }
        else
        {
            loadTexture(fields.m_image, fields.m_textureId);
            result = fields.m_textureId;
            sTextureMipmap = true;
            fields.m_loaded = true;
        }
    }
    if (result >= 0) return result;
    return loadTexture(texture);
}

void Textures::removeMemTexture(const std::wstring& name)
{
    EnterCriticalSection(&mMutex);
    boost::unordered_map<std::wstring, MemTexture*>::iterator found = mMemTextures.find(name);
    if (found != mMemTextures.end())
    {
        MemTexture* texture = found->second;
        if (texture == NULL)
        {
            mMemTextures.erase(name);
        }
        else
        {
            MemTextureFields& fields = memTextureFields(texture);
            fields.m_referenceCount = fields.m_referenceCount - 1;
            if (fields.m_referenceCount == 0)
            {
                if (fields.m_textureId >= 0) releaseTexture(fields.m_textureId);
                mMemTextures.erase(name);
                delete texture;
            }
        }
    }
    LeaveCriticalSection(&mMutex);
}

void Textures::tick(bool force, bool unused, bool reload)
{
    boost::unordered_map<std::wstring, MemTexture*>::iterator it;

    if (unused != mReloading)
    {
        mReloading = unused;
        if (!mMemTextures.empty())
        {
            for (it = mMemTextures.begin(); it != mMemTextures.end(); ++it)
            {
                if (it->second != NULL)
                {
                    memTextureFields(it->second).m_textureReady = mReloading;
                }
            }
        }
    }
    MemSect(0x16);

    if (reload)
    {
        if (!force)
        {
            MemSect(0);
            return;
        }
        mBlocks->cycleAnimationFrames();
        mItems->cycleAnimationFrames();
    }

    EnterCriticalSection(&mMutex);
    if (mMemTextures.empty())
    {
        LeaveCriticalSection(&mMutex);
    }
    else
    {
        it = mMemTextures.begin();
        while (it != mMemTextures.end())
        {
            MemTexture* texture = it->second;
            for (;;)
            {
                if (texture == NULL) break;
                MemTextureFields& fields = memTextureFields(texture);
                if (fields.m_textureReady) break;
                fields.m_textureFormat = fields.m_textureFormat + 1;
                if (fields.m_textureFormat <= 20) break;
                if (fields.m_textureId >= 0) releaseTexture(fields.m_textureId);
                delete texture;
                it = mMemTextures.erase(it);
                if (it == mMemTextures.end()) goto leave_mutex;
                texture = it->second;
            }
            ++it;
        }
leave_mutex:
        LeaveCriticalSection(&mMutex);
    }

    MemSect(0);
}

void Textures::freeStitch()
{
    if (mBlocks != NULL) mBlocks->freeStitch();
    if (mItems != NULL) mItems->freeStitch();
}

void Textures::stitch()
{
    if (mBlocks != NULL) mBlocks->stitch();
    if (mItems != NULL) mItems->stitch();
}

void Textures::reloadAll()
{
    clear();
    mTextureImages.clear();
    freeStitch();
    loadIndexedTextures();
    mTexturePixels.clear();
    GrassColor::init(loadTexturePixels((_TEXTURE_NAME)0, L"misc_grasscolor"));
    FoliageColor::init(loadTexturePixels((_TEXTURE_NAME)1, L"misc_foliagecolor"));
    stitch();
    if (mTexturePackRepository != NULL) mTexturePackRepository->clearInvalidTexturePacks();
}

TextureAtlasSprite* Textures::getMissingIcon(int atlas)
{
    if (atlas != 0)
    {
        if (mItems != NULL) return mItems->getMissingIcon();
    }
    else if (mBlocks != NULL) return mBlocks->getMissingIcon();
    return NULL;
}
