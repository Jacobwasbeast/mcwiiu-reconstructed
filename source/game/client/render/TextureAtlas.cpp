#include "game/client/render/TextureAtlas.hpp"

#include <cmath>

TextureAtlas::TextureAtlas(int iconType, const std::wstring& name,
                           const std::wstring& path, BufferedImage* image,
                           bool singleImage)
{
    mIconType = iconType;
    mName = name;
    mPath = path;
    mSpritePrefix = L"buckets_";
    mSingleImage = singleImage;
    mImage = image;
    mStitchedTexture = 0;
    mMissingIcon = TextureAtlasSprite::create(std::wstring());
}

TextureAtlas::~TextureAtlas()
{
    mAnimatedSprites.clear();
    mSprites.clear();
    mSpritePrefix.clear();
    mPath.clear();
    mName.clear();
}

TextureAtlasSprite* TextureAtlas::addIcon(bool singleImage, int x, int y,
                                           const std::wstring& name)
{
    const float scale = 0.0625f;
    const float vScale = singleImage ? 0.0294117648f : 0.0588235296f;
    const float vOffset = singleImage ? 272.0f : 544.0f;
    TextureAtlasSprite* sprite = new TextureAtlasSprite(
        name, name, scale * static_cast<float>(y),
        vScale * static_cast<float>(x), scale * static_cast<float>(y + 1),
        vScale * static_cast<float>(x + 1), 256.0f, vOffset);
    mSprites[name] = sprite;
    return sprite;
}

TextureAtlasSprite* TextureAtlas::addIconWithName(bool singleImage, int x,
                                                   int y,
                                                   const std::wstring& name,
                                                   const std::wstring& spriteName)
{
    const float scale = 0.0625f;
    const float vScale = singleImage ? 0.0294117648f : 0.0588235296f;
    const float vOffset = singleImage ? 272.0f : 544.0f;
    TextureAtlasSprite* sprite = new TextureAtlasSprite(
        name, spriteName, scale * static_cast<float>(y),
        vScale * static_cast<float>(x), scale * static_cast<float>(y + 1),
        vScale * static_cast<float>(x + 1), 256.0f, vOffset);
    mSprites[name] = sprite;
    return sprite;
}

void TextureAtlas::addIconSize(bool singleImage, int x, int y,
                               const std::wstring& name, int width, int height)
{
    const float scale = 0.0625f;
    const float vScale = singleImage ? 0.0294117648f : 0.0588235296f;
    const float vOffset = singleImage ? 272.0f : 544.0f;
    mSprites[name] = new TextureAtlasSprite(
        name, name, scale * static_cast<float>(y),
        vScale * static_cast<float>(x),
        scale * static_cast<float>(y + width),
        vScale * static_cast<float>(x + height), 256.0f, vOffset);
}

void TextureAtlas::loadUVs()
{
    mSprites.clear();
    if (mIconType == 0)
    {
        addIcon(true, 0, 0, L"grass_top");
        mSprites[L"grass_top"]->setFlags(2);
        addIcon(true, 0, 1, L"stone");
        addIcon(true, 0, 2, L"dirt");
        addIcon(true, 0, 3, L"grass_side");
        mSprites[L"grass_side"]->setFlags(1);
        addIcon(true, 0, 4, L"planks_oak");
        addIcon(true, 0, 5, L"stoneslab_side");
        addIcon(true, 0, 6, L"stoneslab_top");
        addIcon(true, 0, 7, L"brick");
        addIcon(true, 0, 8, L"tnt_side");
        addIcon(true, 0, 9, L"tnt_top");
        addIcon(true, 0, 10, L"tnt_bottom");
        addIcon(true, 0, 12, L"flower_rose");
        addIcon(true, 0, 13, L"flower_dandelion");
        addIcon(true, 0, 14, L"portal");
        addIcon(true, 0, 15, L"sapling");

        addIcon(true, 1, 0, L"cobblestone");
        addIcon(true, 1, 1, L"bedrock");
        addIcon(true, 1, 2, L"sand");
        addIcon(true, 1, 3, L"gravel");
        addIcon(true, 1, 4, L"log_oak");
        addIcon(true, 1, 5, L"log_oak_top");
        addIcon(true, 1, 6, L"iron_block");
        addIcon(true, 1, 7, L"gold_block");
        addIcon(true, 1, 8, L"diamond_block");
        addIcon(true, 1, 9, L"emerald_block");
        addIcon(true, 1, 10, L"redstone_block");
        addIcon(true, 1, 11, L"dropper_front_horizontal");
        addIcon(true, 1, 12, L"mushroom_red");
        addIcon(true, 1, 13, L"mushroom_brown");
        addIcon(true, 1, 14, L"sapling_jungle");
        addIcon(true, 1, 15, L"fire_0");

        addIcon(true, 2, 0, L"gold_ore");
        addIcon(true, 2, 1, L"iron_ore");
        addIcon(true, 2, 2, L"coal_ore");
        addIcon(true, 2, 3, L"bookshelf");
        addIcon(true, 2, 4, L"cobblestone_mossy");
        addIcon(true, 2, 5, L"obsidian");
        addIcon(true, 2, 6, L"grass_side_overlay");
        addIcon(true, 2, 7, L"tallgrass");
        addIcon(true, 2, 8, L"dispenser_front_vertical");
        addIcon(true, 2, 9, L"beacon");
        addIcon(true, 2, 10, L"dropper_front_vertical");
        addIcon(true, 2, 11, L"workbench_top");
        addIcon(true, 2, 12, L"furnace_front");
        addIcon(true, 2, 13, L"furnace_side");
        addIcon(true, 2, 14, L"dispenser_front");
        addIcon(true, 2, 15, L"fire_1");

        addIcon(true, 3, 0, L"sponge");
        addIcon(true, 3, 1, L"glass");
        addIcon(true, 3, 2, L"diamond_ore");
        addIcon(true, 3, 3, L"redstone_ore");
        addIcon(true, 3, 4, L"leaves");
        addIcon(true, 3, 5, L"leaves_opaque");
        addIcon(true, 3, 6, L"stonebrick");
        addIcon(true, 3, 7, L"deadbush");
        addIcon(true, 3, 8, L"fern");
        addIcon(true, 3, 9, L"daylight_detector_top");
        addIcon(true, 3, 10, L"daylight_detector_side");
        addIcon(true, 3, 11, L"workbench_side");
        addIcon(true, 3, 12, L"workbench_front");
        addIcon(true, 3, 13, L"furnace_front_lit");
        addIcon(true, 3, 14, L"furnace_top");
        addIcon(true, 3, 15, L"sapling_spruce");

        addIcon(true, 4, 0, L"wool_colored_white");
        addIcon(true, 4, 1, L"mob_spawner");
        addIcon(true, 4, 2, L"snow");
        addIcon(true, 4, 4, L"snow_side");
        addIcon(true, 4, 5, L"cactus_top");
        addIcon(true, 4, 6, L"cactus_side");
        addIcon(true, 4, 7, L"cactus_bottom");
        addIcon(true, 4, 8, L"clay");
        addIcon(true, 4, 9, L"reeds");
        addIcon(true, 4, 10, L"jukebox_side");
        addIcon(true, 4, 11, L"jukebox_top");
        addIcon(true, 4, 12, L"waterlily");
        addIcon(true, 4, 13, L"mycel_side");
        addIcon(true, 4, 14, L"mycel_top");
        addIcon(true, 4, 15, L"sapling_birch");

        addIcon(true, 5, 0, L"torch_on");
        addIcon(true, 5, 1, L"door_wood_upper");
        addIcon(true, 5, 2, L"door_iron_upper");
        addIcon(true, 5, 3, L"ladder");
        addIcon(true, 5, 4, L"trapdoor");
        addIcon(true, 5, 5, L"iron_bars");
        addIcon(true, 5, 6, L"farmland_wet");
        addIcon(true, 5, 7, L"farmland_dry");
        addIcon(true, 5, 8, L"crops_0");
        addIcon(true, 5, 9, L"crops_1");
        addIcon(true, 5, 10, L"crops_2");
        addIcon(true, 5, 11, L"crops_3");
        addIcon(true, 5, 12, L"crops_4");
        addIcon(true, 5, 13, L"crops_5");
        addIcon(true, 5, 14, L"crops_6");
        addIcon(true, 5, 15, L"crops_7");

        addIcon(true, 6, 0, L"lever");
        addIcon(true, 6, 1, L"door_wood_lower");
        addIcon(true, 6, 2, L"door_iron_lower");
        addIcon(true, 6, 3, L"redstone_torch_on");
        addIcon(true, 6, 4, L"stonebrick_mossy");
        addIcon(true, 6, 5, L"stonebrick_cracked");
        addIcon(true, 6, 6, L"pumpkin_top");
        addIcon(true, 6, 7, L"netherrack");
        addIcon(true, 6, 8, L"soul_sand");
        addIcon(true, 6, 9, L"glowstone");
        addIcon(true, 6, 10, L"piston_top_sticky");
        addIcon(true, 6, 11, L"piston_top");
        addIcon(true, 6, 12, L"piston_side");
        addIcon(true, 6, 13, L"piston_bottom");
        addIcon(true, 6, 14, L"piston_inner_top");
        addIcon(true, 6, 15, L"stem_straight");

        addIcon(true, 7, 0, L"rail_normal_turned");
        addIcon(true, 7, 1, L"wool_colored_black");
        addIcon(true, 7, 2, L"wool_colored_gray");
        addIcon(true, 7, 3, L"redstone_torch_off");
        addIcon(true, 7, 4, L"log_spruce");
        addIcon(true, 7, 5, L"log_birch");
        addIcon(true, 7, 6, L"pumpkin_side");
    }
}

void TextureAtlas::makeTextureAnimated(TexturePack* pack,
                                       TextureAtlasSprite* sprite)
{
    if (sprite->hasOwnData() == 0)
    {
        mAnimatedSprites.push_back(sprite);
        return;
    }
    const std::wstring animation = pack->getAnimationString(
        sprite->getName(), mName, mPath, true);
    std::vector<Texture*>* textures =
        TextureManager::getInstance()->createTextures(animation, mSingleImage);
    if (textures != 0 && !textures->empty())
    {
        Texture* texture = textures->at(0);
        sprite->init(mStitchedTexture, textures,
                     static_cast<int>(sprite->getU0(false)),
                     static_cast<int>(sprite->getV0(false)),
                     static_cast<int>(texture->getWidth()),
                     static_cast<int>(texture->getHeight()), false);
        if (textures->size() > 1)
        {
            mAnimatedSprites.push_back(sprite);
            sprite->loadAnimationFrames(animation);
        }
    }
}

void TextureAtlas::stitch()
{
    loadUVs();
    for (std::vector<TextureAtlasSprite*>::iterator it = mAnimatedSprites.begin();
         it != mAnimatedSprites.end(); ++it)
    {
        (*it)->initUVs(0.0f, 0.0f, 1.0f, 1.0f);
    }
    if (mStitchedTexture != 0)
        mStitchedTexture->updateOnGPU();
}

void TextureAtlas::freeStitch()
{
    if (mStitchedTexture != 0)
    {
        TextureManager::getInstance()->unregisterTexture(mName, mStitchedTexture);
        delete mStitchedTexture;
    }
    mStitchedTexture = 0;
}

void TextureAtlas::cycleAnimationFrames()
{
    for (std::vector<TextureAtlasSprite*>::iterator it = mAnimatedSprites.begin();
         it != mAnimatedSprites.end(); ++it)
        (*it)->cycleFrames();
}

Texture* TextureAtlas::getStitchedTexture()
{
    return mStitchedTexture;
}

TextureAtlasSprite* TextureAtlas::registerIcon(const std::wstring& name)
{
    TextureAtlasSprite* sprite = 0;
    if (name.empty())
        sprite = mMissingIcon;
    else
    {
        boost::unordered_map<std::wstring, TextureAtlasSprite*>::iterator it =
            mSprites.find(name);
        if (it != mSprites.end())
            sprite = it->second;
    }
    return sprite != 0 ? sprite : mMissingIcon;
}

int TextureAtlas::getIconType()
{
    return mIconType;
}

TextureAtlasSprite* TextureAtlas::getMissingIcon()
{
    return mMissingIcon;
}
