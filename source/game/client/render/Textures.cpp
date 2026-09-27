#include "game/client/render/Textures.hpp"

#include <cstring>
#include <vector>

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
#include "game/client/render/Renderer.hpp"
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
    mBlocks = new TextureAtlas(0, L"terrain", L"textures/blocks/", mMissingImage, true);
    mItems = new TextureAtlas(1, L"items", L"textures/items/", mMissingImage, true);
    loadIndexedTextures();
}

bool Textures::IsTUImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    static const int textureIds[] = {
        47, 51, 52, 32, 16, 57, 58, 59, 62, 63, 64, 65,
        66, 67, 71, 72, 73, 75, 74, 70, 240, 241, 14, 82,
        56, 4, 27, 83, 20, 84, 85, 86, 87, 88, 89, 90,
        91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102,
        103, 104, 105, 106, 107, 108, 109, 110, 111, 37, 112, 113,
        114, 115, 116, 117, 118, 119, 120, 122, 121, 124, 125, 126,
        127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138,
        139, 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150,
        151, 152, 153, 154, 34, 155, 156, 157, 158, 159, 160, 161,
        162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173,
        174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184, 185,
        186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
        198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209,
        210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221,
        222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
        234, 235, 236, 237, 238, 239,
    };
    for (unsigned int i = 0; i < sizeof(textureIds) / sizeof(textureIds[0]); ++i)
    {
        if (static_cast<int>(textureName) == textureIds[i]) return true;
    }
    static const wchar_t* const textureNames[] = {
        L"font/Default",
        L"font/Mojangles_7",
        L"font/Mojangles_11",
        L"armor/cloth_1",
        L"armor/cloth_1_b",
        L"armor/cloth_2",
        L"armor/cloth_2_b",
        L"armor/turtle_1",
    };
    for (unsigned int i = 0; i < sizeof(textureNames) / sizeof(textureNames[0]); ++i)
    {
        if (name == textureNames[i]) return true;
    }
    return false;
}

bool Textures::IsOriginalImage(_TEXTURE_NAME textureName, const std::wstring& name)
{
    static const int textureIds[] = {
        38, 39, 40, 41, 42, 43, 44, 13, 0, 1,
    };
    for (unsigned int i = 0; i < sizeof(textureIds) / sizeof(textureIds[0]); ++i)
    {
        if (static_cast<int>(textureName) == textureIds[i]) return true;
    }
    static const wchar_t* const textureNames[] = {
        L"misc/watercolor.png",
    };
    for (unsigned int i = 0; i < sizeof(textureNames) / sizeof(textureNames[0]); ++i)
    {
        if (name == textureNames[i]) return true;
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
    if (name == L"environment/clouds" || name == L"%clamp%misc/shadow" ||
        name == L"%blur%misc/pumpkinblur" || name == L"gui/icons" ||
        name == L"gui/gui" || name == L"misc/footprint" ||
        name == L"mob/shulker/endergolem" || name == L"item/bed" ||
        name == L"font/Default")
    {
        sTextureMipmap = false;
    }
    if (name.size() >= 6 && name.compare(0, 6, L"armor/") == 0 &&
        name != L"armor/power")
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
    bool blur = name.compare(0, 6, L"%blur%") == 0;
    std::wstring workingName = blur ? name.substr(6) : name;
    bool clamp = name.compare(0, 7, L"%clamp%") == 0;
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
    static const wchar_t* const textureNames[] = {
        L"misc/grasscolor",
        L"misc/foliagecolor",
        L"%blur%misc/pumpkinblur",
        L"%clamp%misc/shadow",
        L"art/kz",
        L"environment/clouds",
        L"environment/rain",
        L"environment/snow",
        L"gui/gui",
        L"gui/icons",
        L"item/arrows",
        L"item/cart",
        L"item/sign",
        L"misc/mapbg",
        L"misc/mapicons",
        L"misc/water",
        L"misc/footprint",
        L"mob/saddle",
        L"mob/sheep_fur",
        L"mob/spider_eyes",
        L"particles",
        L"mob/chicken",
        L"mob/cow",
        L"mob/pig",
        L"mob/sheep",
        L"mob/squid",
        L"mob/wolf",
        L"mob/wolf_tame",
        L"mob/wolf_angry",
        L"mob/creeper",
        L"mob/ghast",
        L"mob/ghast_fire",
        L"mob/zombie",
        L"mob/pigzombie",
        L"mob/skeleton",
        L"mob/slime",
        L"mob/spider",
        L"mob/steve",
        L"mob/char1",
        L"mob/char2",
        L"mob/char3",
        L"mob/char4",
        L"mob/char5",
        L"mob/char6",
        L"mob/char7",
        L"terrain/moon",
        L"terrain/sun",
        L"armor/power",
        L"mob/cavespider",
        L"mob/enderman",
        L"mob/silverfish",
        L"mob/enderman_eyes",
        L"misc/explosion",
        L"item/xporb",
        L"item/chest",
        L"item/largechest",
        L"item/enderchest",
        L"mob/redcow",
        L"mob/snowman",
        L"mob/enderdragon/ender",
        L"mob/fire",
        L"mob/lava",
        L"mob/villager/villager",
        L"mob/villager/farmer",
        L"mob/villager/librarian",
        L"mob/villager/priest",
        L"mob/villager/smith",
        L"mob/villager/butcher",
        L"mob/enderdragon/crystal",
        L"mob/enderdragon/shuffle",
        L"mob/enderdragon/beam",
        L"mob/enderdragon/ender_eyes",
        L"%blur%misc/glint",
        L"item/book",
        L"misc/tunnel",
        L"misc/particlefield",
        L"terrain/moon_phases",
        L"mob/ozelot",
        L"mob/cat_black",
        L"mob/cat_red",
        L"mob/cat_siamese",
        L"mob/villager_golem",
        L"mob/skeleton_wither",
        L"mob/wolf_collar",
        L"mob/zombie_villager/zombie_villager",
        L"item/lead_knot",
        L"misc/beacon_beam",
        L"mob/bat",
        L"mob/horse/donkey",
        L"mob/horse/horse_black",
        L"mob/horse/horse_brown",
        L"mob/horse/horse_chestnut",
        L"mob/horse/horse_creamy",
        L"mob/horse/horse_darkbrown",
        L"mob/horse/horse_gray",
        L"mob/horse/horse_markings_blackdots",
        L"mob/horse/horse_markings_white",
        L"mob/horse/horse_markings_whitedots",
        L"mob/horse/horse_markings_whitefield",
        L"mob/horse/horse_skeleton",
        L"mob/horse/horse_white",
        L"mob/horse/horse_zombie",
        L"mob/horse/mule",
        L"mob/horse/armor/horse_armor_diamond",
        L"mob/horse/armor/horse_armor_gold",
        L"mob/horse/armor/horse_armor_iron",
        L"mob/witch",
        L"mob/wither/wither",
        L"mob/wither/wither_armor",
        L"mob/wither/wither_invulnerable",
        L"item/trapped",
        L"item/trapped_double",
        L"mob/alex",
        L"mob/alex1",
        L"mob/alex2",
        L"mob/alex3",
        L"mob/alex4",
        L"mob/alex5",
        L"mob/alex6",
        L"mob/alex7",
        L"misc/additionalmapicons",
        L"item/armorstand/wood",
        L"font/Default",
        L"font/alternate",
        L"mob/endermite",
        L"mob/guardian",
        L"mob/guardian_elder",
        L"mob/guardian_beam",
        L"mob/rabbit/brown",
        L"mob/rabbit/white",
        L"mob/rabbit/black",
        L"mob/rabbit/gold",
        L"mob/rabbit/salt",
        L"mob/rabbit/white_splotched",
        L"mob/rabbit/toast",
        L"mob/rabbit/caerbannog",
        L"textures/destroy_stage_0",
        L"textures/destroy_stage_1",
        L"textures/destroy_stage_2",
        L"textures/destroy_stage_3",
        L"textures/destroy_stage_4",
        L"textures/destroy_stage_5",
        L"textures/destroy_stage_6",
        L"textures/destroy_stage_7",
        L"textures/destroy_stage_8",
        L"textures/destroy_stage_9",
        L"item/banner/banner_base",
        L"item/banner/Banner_Atlas",
        L"mob/bear/polarbear",
        L"item/boat/boat_oak",
        L"item/boat/boat_spruce",
        L"item/boat/boat_birch",
        L"item/boat/boat_jungle",
        L"item/boat/boat_acacia",
        L"item/boat/boat_darkoak",
        L"mob/skeleton/stray",
        L"mob/skeleton/stray_overlay",
        L"mob/zombie_villager/zombie_farmer",
        L"mob/zombie_villager/zombie_librarian",
        L"mob/zombie_villager/zombie_priest",
        L"mob/zombie_villager/zombie_smith",
        L"mob/zombie_villager/zombie_butcher",
        L"mob/zombie/husk",
        L"mob/enderdragon/dragon_fireball",
        L"mob/shulker/endergolem",
        L"mob/shulker/spark",
        L"item/spectral_arrow",
        L"item/tipped_arrow",
        L"item/elytra",
        L"mob/DevSteve",
        L"mob/DevAlex",
        L"environment/Thermal_1",
        L"environment/Thermal_2",
        L"environment/BoostArrow",
        L"mob/Glide_Ghost",
        L"mob/llama/decor/decor_white",
        L"mob/llama/decor/decor_orange",
        L"mob/llama/decor/decor_magenta",
        L"mob/llama/decor/decor_light_blue",
        L"mob/llama/decor/decor_yellow",
        L"mob/llama/decor/decor_lime",
        L"mob/llama/decor/decor_pink",
        L"mob/llama/decor/decor_gray",
        L"mob/llama/decor/decor_silver",
        L"mob/llama/decor/decor_cyan",
        L"mob/llama/decor/decor_purple",
        L"mob/llama/decor/decor_blue",
        L"mob/llama/decor/decor_brown",
        L"mob/llama/decor/decor_green",
        L"mob/llama/decor/decor_red",
        L"mob/llama/decor/decor_black",
        L"mob/llama/llama_creamy",
        L"mob/llama/llama_white",
        L"mob/llama/llama_brown",
        L"mob/llama/llama_gray",
        L"mob/llama/spit",
        L"mob/illager/evoker",
        L"mob/illager/fangs",
        L"mob/illager/vindicator",
        L"mob/illager/vex",
        L"mob/illager/vex_charging",
        L"mob/illager/illusionist",
        L"mob/parrot/parrot_red_blue",
        L"mob/parrot/parrot_blue",
        L"mob/parrot/parrot_green",
        L"mob/parrot/parrot_yellow_blue",
        L"mob/parrot/parrot_grey",
        L"item/bed",
        L"item/firework",
        L"mob/horse/armor/horse_armor_leather_1",
        L"mob/horse/armor/horse_armor_leather_1_b",
        L"mob/fish/cod",
        L"mob/fish/pufferfish",
        L"mob/fish/salmon",
        L"mob/fish/tropical_a",
        L"mob/fish/tropical_a_pattern_1",
        L"mob/fish/tropical_a_pattern_2",
        L"mob/fish/tropical_a_pattern_3",
        L"mob/fish/tropical_a_pattern_4",
        L"mob/fish/tropical_a_pattern_5",
        L"mob/fish/tropical_a_pattern_6",
        L"mob/fish/tropical_b",
        L"mob/fish/tropical_b_pattern_1",
        L"mob/fish/tropical_b_pattern_2",
        L"mob/fish/tropical_b_pattern_3",
        L"mob/fish/tropical_b_pattern_4",
        L"mob/fish/tropical_b_pattern_5",
        L"mob/fish/tropical_b_pattern_6",
        L"mob/sea_turtle",
        L"mob/dolphin",
        L"item/trident",
        L"mob/zombie/drowned",
        L"item/trident_riptide",
        L"item/conduit/conduit_base",
        L"item/conduit/conduit_cage",
        L"item/conduit/conduit_wind_horizontal",
        L"item/conduit/conduit_wind_vertical",
        L"item/conduit/conduit_closed",
        L"item/conduit/conduit_open",
        L"mob/phantom",
    };
    for (unsigned int i = 0; i < sizeof(textureNames) / sizeof(textureNames[0]); ++i)
    {
        sIndexedTextures[i] = loadTexture(static_cast<_TEXTURE_NAME>(i), textureNames[i]);
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
#ifdef RECON_NATIVE
    std::vector<unsigned int> nativePixels(pixels.length);
    for (unsigned int i = 0; i < pixels.length; ++i)
    {
        unsigned int value = static_cast<unsigned int>(pixels.data[i]);
        nativePixels[i] = (value >> 24) | ((value >> 8) & 0x0000ff00u)
            | ((value << 8) & 0x00ff0000u) | (value << 24);
    }
    gRenderer->TextureDataUpdate(0, 0, width, height, nativePixels.data(), 0);
#else
    gRenderer->TextureDataUpdate(0, 0, width, height, pixels.data, 0);
#endif
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
            result = getTexture(fields.m_image, C4JRender::eTextureFormat_RGBA8, sTextureMipmap);
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
            result = getTexture(fields.m_image, C4JRender::eTextureFormat_RGBA8, sTextureMipmap);
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
    GrassColor::init(loadTexturePixels((_TEXTURE_NAME)0, L"misc/grasscolor"));
    FoliageColor::init(loadTexturePixels((_TEXTURE_NAME)1, L"misc/foliagecolor"));
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
