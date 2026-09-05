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
        addIcon(true, 0, 1, L"stone");
        addIcon(true, 0, 2, L"dirt");
        addIcon(true, 0, 3, L"grass_side");
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
