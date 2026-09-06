#include "game/client/render/Textures.hpp"

#include <cstring>

#include "game/core/BufferedImage.hpp"
#include "game/core/ByteBuffer.hpp"
#include "game/core/FoliageColor.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/GrassColor.hpp"
#include "game/core/MemoryTracker.hpp"
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
static ResourceLocation sTerrainLocation((_TEXTURE_NAME)0);
static ResourceLocation sItemsLocation((_TEXTURE_NAME)1);
static bool sTextureMipmap = true;

Textures::Textures(TexturePackRepository* texturePackRepository, Options* options)
    : mOptions(options), mTexturePackRepository(texturePackRepository), mMissingImage(NULL),
      mBlocks(NULL), mItems(NULL), mLastBoundId(-1), mTextureFormat(false),
      mReloading(false), mLoaded(false)
{
    InitializeCriticalSection(&mMutex);
    mMissingImage = new BufferedImage(3, 16, 16);
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
    std::wstring resource = L"res/" + name;
    TexturePack* selected = mTexturePackRepository->getSelected();
    TexturePack* defaultPack = TexturePackRepository::defaultTexturePack;
    bool useSelected = false;
    if (selected != defaultPack)
    {
        useSelected = selected->hasFile(resource, false);
    }
    if (useSelected)
    {
        std::wstring path = selected->getPath(false, "");
        return selected->getImageResource(resource, IsTUImage(textureName, name), false, path);
    }
    if (!IsOriginalImage(textureName, name) && !IsTUImage(textureName, name))
    {
        resource = L"1.2.2/" + name;
    }
    return defaultPack->getImageResource(resource, IsTUImage(textureName, name), false, L"res/" + name);
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
    bind(texture);
    GlStateManager::texParameteri(1, 0);
    GlStateManager::texParameteri(2, 0);
    if (blur)
    {
        GlStateManager::texParameteri(1, 1);
        GlStateManager::texParameteri(2, 1);
    }
    GlStateManager::texParameteri(3, clamp ? 0 : 1);
    GlStateManager::texParameteri(4, clamp ? 0 : 1);
    arrayWithLength<int> pixels = loadTexturePixels(image);
    ByteBuffer* buffer = MemoryTracker::createByteBuffer((int)(pixels.length * 4));
    arrayWithLength<unsigned char> bytes((unsigned int)(pixels.length * 4), true);
    std::memcpy(bytes.data, pixels.data, bytes.length);
    buffer->put(bytes);
    buffer->position(0);
    buffer->limit(bytes.length);
    GlStateManager::texImage2D(0, 0, 0, (int)image->getWidth(), (int)image->getHeight(), 0, 0, 0, buffer);
    delete buffer;
}

int Textures::loadTexture(_TEXTURE_NAME textureName, const std::wstring& name)
{
    iterator found = find(name);
    if (found != end()) return found->second;
    setTextureFormat(name);
    int texture = MemoryTracker::genTextures();
    BufferedImage* image = readImage(textureName, name);
    if (image->getWidth() > 0 && image->getHeight() > 0)
    {
        loadTexture(image, texture, false, !sTextureMipmap);
    }
    (*this)[name] = texture;
    delete image;
    mTextureFormat = false;
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
        Texture* texture = mBlocks->getStitchedTexture();
        texture->bind(0);
        return (int)texture->getGlId();
    }
    if (textureName == 0xf0)
    {
        Texture* texture = mItems->getStitchedTexture();
        texture->bind(0);
        return (int)texture->getGlId();
    }
    if (textureName < 0 || textureName >= 256) return 0;
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
    if (location == NULL) return;
    int i;
    if (!location->isPreloaded())
    {
        bind(loadTexture((_TEXTURE_NAME)0xf2, location->getPath()));
        return;
    }
    for (i = 0; i < location->getTextureCount(); ++i) bind(loadTexture(location->getTexture(i)));
}

ResourceLocation* Textures::getTextureLocation(int textureName)
{
    if (textureName == 0) return &sTerrainLocation;
    if (textureName == 1) return &sItemsLocation;
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
    int texture = addMemTexture(name, NULL);
    if (texture != 0) return texture;
    if (path.empty()) return -1;
    return loadTexture((_TEXTURE_NAME)0xf2, path);
}

int Textures::loadMemTexture(const std::wstring& name, int texture)
{
    int result = addMemTexture(name, NULL);
    if (result != 0) return result;
    if (texture >= 0)
    {
        return loadTexture(texture);
    }
    return loadTexture((_TEXTURE_NAME)0xf2, name);
}

void Textures::removeMemTexture(const std::wstring& name)
{
    EnterCriticalSection(&mMutex);
    mMemTextures.erase(name);
    LeaveCriticalSection(&mMutex);
}

void Textures::tick(bool force, bool unused, bool reload)
{
    if (reload) reloadAll();
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

void Textures::getMissingIcon(int atlas)
{
    if (atlas != 0)
    {
        if (mItems != NULL) mItems->getMissingIcon();
    }
    else if (mBlocks != NULL) mBlocks->getMissingIcon();
}
