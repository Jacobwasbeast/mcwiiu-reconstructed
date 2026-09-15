#include "game/client/render/BlockRenderer.hpp"

#include <cstring>

#include "game/client/render/TextureAtlasSprite.hpp"
#include "game/client/render/Textures.hpp"
#include "game/client/render/Tesselator.hpp"
#include "game/core/BufferBuilder.hpp"
#include "game/core/GlStateManager.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockColors.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ChorusFlowerBlock.hpp"
#include "game/world/block/ChorusPlantBlock.hpp"
#include "game/world/block/CommandBlock.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DoorBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/core/Options.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/StemBlock.hpp"
#include "game/world/block/WallBlock.hpp"
#include "game/util/Random.hpp"

BlockRenderer::BlockRenderer(BlockColors* blockColors)
    : mBlockColors(blockColors), mLevelSource(NULL), mBrightnessCache()
{
    _init();
}

BlockRenderer::BlockRenderer(LevelSource* levelSource, int x, int y, int z, unsigned int* cache)
    : mBlockColors(Minecraft::GetInstance()->getBlockColors()), mLevelSource(levelSource), mBrightnessCache()
{
    _init();
    mOriginX = x;
    mOriginY = y;
    mOriginZ = z;
    mCacheMinX = x - 2;
    mCacheMinY = y - 2;
    mCacheMinZ = z - 2;
    mCache = cache;
    std::memset(cache, 0, 0x20000);
}

BlockRenderer::BlockRenderer(LevelSource* levelSource)
    : mBlockColors(Minecraft::GetInstance()->getBlockColors()), mLevelSource(levelSource), mBrightnessCache()
{
    _init();
}

BlockRenderer::~BlockRenderer()
{
}

void BlockRenderer::_init()
{
    mFixedTexture = NULL;
    mLightOverride = -1;
    mFlipTexture = false;
    mRenderAllFaces = false;
    mUseInventoryTint = false;
    mUseAmbientOcclusion = true;
    mMinX = 0.0f;
    mMaxX = 0.0f;
    mMinY = 0.0f;
    mMaxY = 0.0f;
    mMinZ = 0.0f;
    mMaxZ = 0.0f;
    mFixedShape = false;
    mShapeOutside = false;
    mMinecraft = Minecraft::GetInstance();
    mOriginX = 0;
    mOriginY = 0;
    mOriginZ = 0;
    mCacheMinX = 0;
    mCacheMinY = 0;
    mCacheMinZ = 0;
    mCache = NULL;
    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    mRenderFromInside = false;
    mEnableAO = false;
}

bool BlockRenderer::canRender(int renderShape)
{
    if (renderShape < 0x1a)
    {
        if (renderShape < 0x0d)
        {
            if (renderShape != 0 && renderShape != 4)
            {
                if (renderShape < 10 || renderShape > 11)
                    return false;
            }
        }
        else if (renderShape != 0x0d && renderShape != 0x10)
        {
            if (renderShape < 0x15 || renderShape > 0x16)
                return false;
        }
    }
    else if (renderShape < 0x26)
    {
        if (renderShape > 0x1b)
        {
            if (renderShape < 0x1f)
                return false;
            if (renderShape > 0x20 && (renderShape < 0x22 || renderShape > 0x23))
                return false;
        }
    }
    else if (renderShape > 0x27)
    {
        if (renderShape < 0x2a)
            return false;
        if (renderShape > 0x30 && renderShape != 0x32)
        {
            if (renderShape < 0x36 || renderShape > 0x37)
                return false;
        }
    }
    return true;
}

TextureAtlasSprite* BlockRenderer::getTextureOrMissing(TextureAtlasSprite* texture)
{
    if (texture != NULL)
        return texture;
    return mMinecraft->getTextureManager()->getMissingIcon(0);
}

TextureAtlasSprite* BlockRenderer::getTexture(const BlockState* state, const Direction* direction)
{
    return getTextureOrMissing(state->getBlock()->getTexture(direction, state));
}

TextureAtlasSprite* BlockRenderer::getTexture(Block* block, const Direction* direction)
{
    return getTextureOrMissing(block->getTexture(direction));
}

TextureAtlasSprite* BlockRenderer::getTexture(const BlockState* state, LevelSource* levelSource, const BlockPos& pos, const Direction* direction)
{
    return getTextureOrMissing(state->getBlock()->getTexture(levelSource, pos, direction, state));
}

void BlockRenderer::setShape(const AABB* shape)
{
    if (mFixedShape)
        return;
    setShape((float)shape->mMinX, (float)shape->mMinY, (float)shape->mMinZ,
             (float)shape->mMaxX, (float)shape->mMaxY, (float)shape->mMaxZ);
}

void BlockRenderer::setShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
    if (mFixedShape)
        return;
    mMinX = x0;
    mMinY = y0;
    mMinZ = z0;
    mMaxX = x1;
    mMaxY = y1;
    mMaxZ = z1;
    mShapeOutside = x0 > 0.0f || y0 > 0.0f || z0 > 0.0f || x1 < 1.0f || y1 < 1.0f || z1 < 1.0f;
}

bool BlockRenderer::hasFixedTexture()
{
    return mFixedTexture != NULL;
}

void BlockRenderer::setFixedTexture(TextureAtlasSprite* texture)
{
    mFixedTexture = texture;
}

void BlockRenderer::clearFixedTexture()
{
    mFixedTexture = NULL;
}

bool BlockRenderer::hasLightOverride()
{
    return mLightOverride != -1;
}

void BlockRenderer::prepare(LevelSource* levelSource)
{
    mLevelSource = levelSource;
}

void BlockRenderer::setLightOverride(int lightColor)
{
    mLightOverride = lightColor;
}

void BlockRenderer::clearLightOverride()
{
    mLightOverride = -1;
}

void BlockRenderer::setFixedShape(float x0, float y0, float z0, float x1, float y1, float z1)
{
    mMinX = x0;
    mMinY = y0;
    mMinZ = z0;
    mMaxX = x1;
    mMaxY = y1;
    mMaxZ = z1;
    mFixedShape = true;
    mShapeOutside = x0 > 0.0f || y0 > 0.0f || z0 > 0.0f || x1 < 1.0f || y1 < 1.0f || z1 < 1.0f;
}

void BlockRenderer::clearFixedShape()
{
    mFixedShape = false;
}

int BlockRenderer::blend(int c0, int c1, int c2, int c3)
{
    if (c0 == 0)
        c0 = c3;
    if (c1 == 0)
        c1 = c3;
    if (c2 == 0)
        c2 = c3;
    return ((c0 + c1 + c2 + c3) >> 2) & 0xff00ff;
}

int BlockRenderer::blend(int c0, int c1, int c2, int c3, double w0, double w1, double w2, double w3)
{
    int red = (int)(((c0 >> 16 & 255) * w0) + ((c1 >> 16 & 255) * w1) +
                    ((c2 >> 16 & 255) * w2) + ((c3 >> 16 & 255) * w3)) & 255;
    int blue = (int)(((c0 & 255) * w0) + ((c1 & 255) * w1) +
                     ((c2 & 255) * w2) + ((c3 & 255) * w3)) & 255;
    return red << 16 | blue;
}

void BlockRenderer::AOCalculate(Block* block, LevelSource* levelSource, const BlockPos& pos,
                                Direction::EDirection direction, bool atBoundary, int lightColor,
                                bool useColor, float red, float green, float blue,
                                bool partialBounds)
{
    const Direction* face = Direction::DOWN;
    int ux = 1;
    int uy = 0;
    int uz = 0;
    int vx = 0;
    int vy = 0;
    int vz = 1;
    float faceShade = 0.5f;

    switch (direction)
    {
    case Direction::eDirection_Up:
        face = Direction::UP;
        faceShade = 1.0f;
        break;
    case Direction::eDirection_North:
        face = Direction::NORTH;
        vx = 0;
        vy = 1;
        vz = 0;
        faceShade = 0.8f;
        break;
    case Direction::eDirection_South:
        face = Direction::SOUTH;
        vx = 0;
        vy = 1;
        vz = 0;
        faceShade = 0.8f;
        break;
    case Direction::eDirection_West:
        face = Direction::WEST;
        ux = 0;
        uz = 1;
        vx = 0;
        vy = 1;
        vz = 0;
        faceShade = 0.6f;
        break;
    case Direction::eDirection_East:
        face = Direction::EAST;
        ux = 0;
        uz = 1;
        vx = 0;
        vy = 1;
        vz = 0;
        faceShade = 0.6f;
        break;
    default:
        break;
    }

    BlockPos facePos = pos.relative(face);
    BlockPos sampleBase = atBoundary ? facePos : pos;
    BlockPos leftPos = sampleBase.offset(-ux, -uy, -uz);
    BlockPos rightPos = sampleBase.offset(ux, uy, uz);
    BlockPos bottomPos = sampleBase.offset(-vx, -vy, -vz);
    BlockPos topPos = sampleBase.offset(vx, vy, vz);

    float leftAo = getShadeBrightness(levelSource, leftPos);
    float rightAo = getShadeBrightness(levelSource, rightPos);
    float bottomAo = getShadeBrightness(levelSource, bottomPos);
    float topAo = getShadeBrightness(levelSource, topPos);
    int leftLight = getLightColor(block, levelSource, leftPos);
    int rightLight = getLightColor(block, levelSource, rightPos);
    int bottomLight = getLightColor(block, levelSource, bottomPos);
    int topLight = getLightColor(block, levelSource, topPos);

    bool leftSolid = isSolidBlockingAt(levelSource, leftPos);
    bool rightSolid = isSolidBlockingAt(levelSource, rightPos);
    bool bottomSolid = isSolidBlockingAt(levelSource, bottomPos);
    bool topSolid = isSolidBlockingAt(levelSource, topPos);

    BlockPos leftBottomPos = sampleBase.offset(-ux - vx, -uy - vy, -uz - vz);
    BlockPos leftTopPos = sampleBase.offset(-ux + vx, -uy + vy, -uz + vz);
    BlockPos rightBottomPos = sampleBase.offset(ux - vx, uy - vy, uz - vz);
    BlockPos rightTopPos = sampleBase.offset(ux + vx, uy + vy, uz + vz);

    float leftBottomAo;
    float leftTopAo;
    float rightBottomAo;
    float rightTopAo;
    int leftBottomLight;
    int leftTopLight;
    int rightBottomLight;
    int rightTopLight;

    if (leftSolid && bottomSolid)
    {
        leftBottomAo = leftAo;
        leftBottomLight = leftLight;
    }
    else
    {
        leftBottomAo = getShadeBrightness(levelSource, leftBottomPos);
        leftBottomLight = getLightColor(block, levelSource, leftBottomPos);
    }
    if (leftSolid && topSolid)
    {
        leftTopAo = leftAo;
        leftTopLight = leftLight;
    }
    else
    {
        leftTopAo = getShadeBrightness(levelSource, leftTopPos);
        leftTopLight = getLightColor(block, levelSource, leftTopPos);
    }
    if (rightSolid && bottomSolid)
    {
        rightBottomAo = rightAo;
        rightBottomLight = rightLight;
    }
    else
    {
        rightBottomAo = getShadeBrightness(levelSource, rightBottomPos);
        rightBottomLight = getLightColor(block, levelSource, rightBottomPos);
    }
    if (rightSolid && topSolid)
    {
        rightTopAo = rightAo;
        rightTopLight = rightLight;
    }
    else
    {
        rightTopAo = getShadeBrightness(levelSource, rightTopPos);
        rightTopLight = getLightColor(block, levelSource, rightTopPos);
    }

    float faceAo = getShadeBrightness(levelSource, facePos);
    int faceLight = atBoundary ? getLightColor(block, levelSource, facePos) : lightColor;
    float cornerAo[4];
    int cornerLight[4];
    cornerAo[0] = (leftAo + leftTopAo + topAo + faceAo) * 0.25f;
    cornerAo[1] = (leftAo + leftBottomAo + bottomAo + faceAo) * 0.25f;
    cornerAo[2] = (rightAo + rightBottomAo + bottomAo + faceAo) * 0.25f;
    cornerAo[3] = (rightAo + rightTopAo + topAo + faceAo) * 0.25f;
    cornerLight[0] = blend(leftLight, leftTopLight, topLight, faceLight);
    cornerLight[1] = blend(leftLight, leftBottomLight, bottomLight, faceLight);
    cornerLight[2] = blend(rightLight, rightBottomLight, bottomLight, faceLight);
    cornerLight[3] = blend(rightLight, rightTopLight, topLight, faceLight);

    float outputAo[4];
    int outputLight[4];
    if (!partialBounds)
    {
        for (int i = 0; i < 4; ++i)
        {
            outputAo[i] = cornerAo[i];
            outputLight[i] = cornerLight[i];
        }
    }
    else
    {
        float u0;
        float u1;
        float v0;
        float v1;
        if (direction < Direction::eDirection_North)
        {
            u0 = mMinX;
            u1 = mMaxX;
            v0 = mMinZ;
            v1 = mMaxZ;
        }
        else if (direction < Direction::eDirection_West)
        {
            u0 = mMinX;
            u1 = mMaxX;
            v0 = mMinY;
            v1 = mMaxY;
        }
        else
        {
            u0 = mMinZ;
            u1 = mMaxZ;
            v0 = mMinY;
            v1 = mMaxY;
        }

        float cornerU[4] = { u0, u0, u1, u1 };
        float cornerV[4] = { v1, v0, v0, v1 };
        for (int i = 0; i < 4; ++i)
        {
            double w0 = (1.0f - cornerU[i]) * cornerV[i];
            double w1 = (1.0f - cornerU[i]) * (1.0f - cornerV[i]);
            double w2 = cornerU[i] * (1.0f - cornerV[i]);
            double w3 = cornerU[i] * cornerV[i];
            outputAo[i] = (float)(cornerAo[0] * w0 + cornerAo[1] * w1 +
                                  cornerAo[2] * w2 + cornerAo[3] * w3);
            outputLight[i] = blend(cornerLight[0], cornerLight[1], cornerLight[2],
                                   cornerLight[3], w0, w1, w2, w3);
        }
    }

    mAoBrightnessTopLeft = outputLight[0];
    mAoBrightnessBottomLeft = outputLight[1];
    mAoBrightnessBottomRight = outputLight[2];
    mAoBrightnessTopRight = outputLight[3];

    float redScale = useColor ? red : 1.0f;
    float greenScale = useColor ? green : 1.0f;
    float blueScale = useColor ? blue : 1.0f;
    mColorRedTopLeft = outputAo[0] * faceShade * redScale;
    mColorRedBottomLeft = outputAo[1] * faceShade * redScale;
    mColorRedBottomRight = outputAo[2] * faceShade * redScale;
    mColorRedTopRight = outputAo[3] * faceShade * redScale;
    mColorGreenTopLeft = outputAo[0] * faceShade * greenScale;
    mColorGreenBottomLeft = outputAo[1] * faceShade * greenScale;
    mColorGreenBottomRight = outputAo[2] * faceShade * greenScale;
    mColorGreenTopRight = outputAo[3] * faceShade * greenScale;
    mColorBlueTopLeft = outputAo[0] * faceShade * blueScale;
    mColorBlueBottomLeft = outputAo[1] * faceShade * blueScale;
    mColorBlueBottomRight = outputAo[2] * faceShade * blueScale;
    mColorBlueTopRight = outputAo[3] * faceShade * blueScale;
}

void BlockRenderer::renderFaceDown(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(mMinZ * 16.0f);
    float v1 = (float)texture->getV(mMaxZ * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderFaceDown(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderFaceDown(const BlockState*, double x, double y, double z, TextureAtlasSprite*, float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float yy = (float)(y + mMinY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    if (mFaceRotation5 == 1)
    {
        float value = u0;
        u0 = u1;
        u1 = value;
    }
    else if (mFaceRotation5 == 2)
    {
        float value = v0;
        v0 = v1;
        v1 = value;
    }
    builder->bucket(6);
    if (!mEnableAO)
    {
        builder->vertexUV(x0, yy, z1, u0, v1);
        builder->vertexUV(x0, yy, z0, u0, v0);
        builder->vertexUV(x1, yy, z0, u1, v0);
        builder->vertexUV(x1, yy, z1, u1, v1);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x0, yy, z1, u0, v1);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x0, yy, z0, u0, v0);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x1, yy, z0, u1, v0);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x1, yy, z1, u1, v1);
}

void BlockRenderer::renderFaceUp(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(mMinZ * 16.0f);
    float v1 = (float)texture->getV(mMaxZ * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderFaceUp(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderFaceUp(const BlockState*, double x, double y, double z, TextureAtlasSprite*, float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float yy = (float)(y + mMaxY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    builder->bucket(1);
    if (!mEnableAO)
    {
        builder->vertexUV(x1, yy, z1, u1, v1);
        builder->vertexUV(x1, yy, z0, u1, v0);
        builder->vertexUV(x0, yy, z0, u0, v0);
        builder->vertexUV(x0, yy, z1, u0, v1);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x1, yy, z1, u1, v1);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x1, yy, z0, u1, v0);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x0, yy, z0, u0, v0);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x0, yy, z1, u0, v1);
}

void BlockRenderer::renderNorth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderNorth(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderNorth(const BlockState*, double x, double y, double z, TextureAtlasSprite*, float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float zz = (float)(z + mMinZ);
    if (mRenderFromInside)
    {
        float value = x0;
        x0 = x1;
        x1 = value;
    }
    builder->bucket(2);
    if (!mEnableAO)
    {
        builder->vertexUV(x0, y1, zz, u1, v0);
        builder->vertexUV(x1, y1, zz, u0, v0);
        builder->vertexUV(x1, y0, zz, u0, v1);
        builder->vertexUV(x0, y0, zz, u1, v1);
        return;
    }
    builder->color(mColorRedTopLeft, mColorGreenTopLeft, mColorBlueTopLeft);
    builder->tex2(mAoBrightnessTopLeft);
    builder->vertexUV(x0, y1, zz, u1, v0);
    builder->color(mColorRedBottomLeft, mColorGreenBottomLeft, mColorBlueBottomLeft);
    builder->tex2(mAoBrightnessBottomLeft);
    builder->vertexUV(x1, y1, zz, u0, v0);
    builder->color(mColorRedBottomRight, mColorGreenBottomRight, mColorBlueBottomRight);
    builder->tex2(mAoBrightnessBottomRight);
    builder->vertexUV(x1, y0, zz, u0, v1);
    builder->color(mColorRedTopRight, mColorGreenTopRight, mColorBlueTopRight);
    builder->tex2(mAoBrightnessTopRight);
    builder->vertexUV(x0, y0, zz, u1, v1);
}

void BlockRenderer::renderSouth(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinX * 16.0f);
    float u1 = (float)texture->getU(mMaxX * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderSouth(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderSouth(const BlockState*, double x, double y, double z, TextureAtlasSprite*, float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float x0 = (float)(x + mMinX);
    float x1 = (float)(x + mMaxX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float zz = (float)(z + mMaxZ);
    if (mRenderFromInside)
    {
        float value = x0;
        x0 = x1;
        x1 = value;
    }
    builder->bucket(3);
    builder->vertexUV(x0, y1, zz, u0, v0);
    builder->vertexUV(x0, y0, zz, u0, v1);
    builder->vertexUV(x1, y0, zz, u1, v1);
    builder->vertexUV(x1, y1, zz, u1, v0);
}

void BlockRenderer::renderWest(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinZ * 16.0f);
    float u1 = (float)texture->getU(mMaxZ * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderWest(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderWest(const BlockState*, double x, double y, double z, TextureAtlasSprite*, float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float xx = (float)(x + mMinX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    if (mRenderFromInside)
    {
        float value = z0;
        z0 = z1;
        z1 = value;
    }
    builder->bucket(4);
    builder->vertexUV(xx, y1, z1, u1, v0);
    builder->vertexUV(xx, y1, z0, u0, v0);
    builder->vertexUV(xx, y0, z0, u0, v1);
    builder->vertexUV(xx, y0, z1, u1, v1);
}

void BlockRenderer::renderEast(const BlockState* state, double x, double y, double z, TextureAtlasSprite* texture)
{
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u0 = (float)texture->getU(mMinZ * 16.0f);
    float u1 = (float)texture->getU(mMaxZ * 16.0f);
    float v0 = (float)texture->getV(16.0f - mMaxY * 16.0f);
    float v1 = (float)texture->getV(16.0f - mMinY * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    renderEast(state, x, y, z, texture, u0, v0, u1, v1);
}

void BlockRenderer::renderEast(const BlockState*, double x, double y, double z, TextureAtlasSprite*, float u0, float v0, float u1, float v1)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float xx = (float)(x + mMaxX);
    float y0 = (float)(y + mMinY);
    float y1 = (float)(y + mMaxY);
    float z0 = (float)(z + mMinZ);
    float z1 = (float)(z + mMaxZ);
    if (mRenderFromInside)
    {
        float value = z0;
        z0 = z1;
        z1 = value;
    }
    builder->bucket(5);
    builder->vertexUV(xx, y0, z1, u0, v1);
    builder->vertexUV(xx, y0, z0, u1, v1);
    builder->vertexUV(xx, y1, z0, u1, v0);
    builder->vertexUV(xx, y1, z1, u0, v0);
}

void BlockRenderer::tesselateCrossTexture(TextureAtlasSprite* texture, TextureAtlasSprite* overlay, float x, float y, float z, float scale)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(true);
    float u1 = (float)texture->getU1(true);
    float v0 = (float)texture->getV0(true);
    float v1 = (float)texture->getV1(true);
    float overlayU0 = (float)overlay->getU0(true);
    float overlayU1 = (float)overlay->getU1(true);
    float overlayV0 = (float)overlay->getV0(true);
    float overlayV1 = (float)overlay->getV1(true);
    float y1 = y + scale;
    float radius = 0.45f * scale;
    float x0 = x + 0.5f - radius;
    float x1 = x + 0.5f + radius;
    float z0 = z + 0.5f - radius;
    float z1 = z + 0.5f + radius;
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y, z0, u0, v1);
    builder->vertexUV(x1, y, z1, u1, v1);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y, z1, u0, v1);
    builder->vertexUV(x0, y, z0, u1, v1);
    builder->vertexUV(x0, y1, z0, u1, v0);
    builder->vertexUV(x0, y1, z1, overlayU0, overlayV0);
    builder->vertexUV(x0, y, z1, overlayU0, overlayV1);
    builder->vertexUV(x1, y, z0, overlayU1, overlayV1);
    builder->vertexUV(x1, y1, z0, overlayU1, overlayV0);
    builder->vertexUV(x1, y1, z0, overlayU0, overlayV0);
    builder->vertexUV(x1, y, z0, overlayU0, overlayV1);
    builder->vertexUV(x0, y, z1, overlayU1, overlayV1);
    builder->vertexUV(x0, y1, z1, overlayU1, overlayV0);
}

void BlockRenderer::tesselateCrossTexture(const BlockState* state, float x, float y, float z, float scale)
{
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;
    tesselateCrossTexture(texture, texture, x, y, z, scale);
}

void BlockRenderer::tesselateStemTexture(const BlockState* state, float x, float y, float z, float height)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = hasFixedTexture() ? mFixedTexture : getTexture(state, Direction::UP);
    float u0 = (float)texture->getU0(false);
    float u1 = (float)texture->getU1(false);
    float v0 = (float)texture->getV0(false);
    float v1 = (float)texture->getV(height * 16.0f);
    texture->adjustUVs(u0, u1, v0, v1);
    float y1 = y + height;
    float x0 = x + 0.05f;
    float x1 = x + 0.95f;
    float z0 = z + 0.05f;
    float z1 = z + 0.95f;
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y, z0, u0, v1);
    builder->vertexUV(x1, y, z1, u1, v1);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x1, y1, z1, u0, v0);
    builder->vertexUV(x1, y, z1, u0, v1);
    builder->vertexUV(x0, y, z0, u1, v1);
    builder->vertexUV(x0, y1, z0, u1, v0);
    builder->vertexUV(x0, y1, z1, u0, v0);
    builder->vertexUV(x0, y, z1, u0, v1);
    builder->vertexUV(x1, y, z0, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y1, z0, u0, v0);
    builder->vertexUV(x1, y, z0, u0, v1);
    builder->vertexUV(x0, y, z1, u1, v1);
    builder->vertexUV(x0, y1, z1, u1, v0);
}

void BlockRenderer::fjRenderCubes(const BlockState* state, const BoxDesc* boxes, unsigned int boxCount, const BoxUV* uvs)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::pushMatrix();
    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    for (unsigned int i = 0; i < boxCount; ++i)
    {
        const BoxDesc& box = boxes[i];
        setShape(box.x0, box.y0, box.z0, box.x1, box.y1, box.z1);
        const BoxUV* uv = uvs == NULL ? NULL : &uvs[i];
        if ((box.hiddenFaces & 1) == 0)
        {
            builder->begin();
            builder->normal(0.0f, -1.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
            if (uv == NULL)
                renderFaceDown(state, 0.0, 0.0, 0.0, texture);
            else
                renderFaceDown(state, 0.0, 0.0, 0.0, texture, uv->down[0], uv->down[1], uv->down[2], uv->down[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 2) == 0)
        {
            builder->begin();
            builder->normal(0.0f, 1.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::UP);
            if (uv == NULL)
                renderFaceUp(state, 0.0, 0.0, 0.0, texture);
            else
                renderFaceUp(state, 0.0, 0.0, 0.0, texture, uv->up[0], uv->up[1], uv->up[2], uv->up[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 4) == 0)
        {
            builder->begin();
            builder->normal(0.0f, 0.0f, -1.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::NORTH);
            if (uv == NULL)
                renderNorth(state, 0.0, 0.0, 0.0, texture);
            else
                renderNorth(state, 0.0, 0.0, 0.0, texture, uv->north[0], uv->north[1], uv->north[2], uv->north[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 8) == 0)
        {
            builder->begin();
            builder->normal(0.0f, 0.0f, 1.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::SOUTH);
            if (uv == NULL)
                renderSouth(state, 0.0, 0.0, 0.0, texture);
            else
                renderSouth(state, 0.0, 0.0, 0.0, texture, uv->south[0], uv->south[1], uv->south[2], uv->south[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 0x20) == 0)
        {
            builder->begin();
            builder->normal(-1.0f, 0.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::WEST);
            if (uv == NULL)
                renderWest(state, 0.0, 0.0, 0.0, texture);
            else
                renderWest(state, 0.0, 0.0, 0.0, texture, uv->west[0], uv->west[1], uv->west[2], uv->west[3]);
            builder->end();
        }
        if ((box.hiddenFaces & 0x10) == 0)
        {
            builder->begin();
            builder->normal(1.0f, 0.0f, 0.0f);
            TextureAtlasSprite* texture = getTexture(state, Direction::EAST);
            if (uv == NULL)
                renderEast(state, 0.0, 0.0, 0.0, texture);
            else
                renderEast(state, 0.0, 0.0, 0.0, texture, uv->east[0], uv->east[1], uv->east[2], uv->east[3]);
            builder->end();
        }
    }
    GlStateManager::popMatrix();
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

int BlockRenderer::getLightColor(Block* block, LevelSource* levelSource, const BlockPos& pos)
{
    if (mCache != NULL)
    {
        unsigned int index = (unsigned int)((pos.x - mCacheMinX) * 0x400 +
                                            (pos.y - mCacheMinY) * 0x20 +
                                            pos.z - mCacheMinZ);
        if ((index & 0xffff8000) == 0 && block != Blocks::FLOWING_WATER && block != Blocks::WATER &&
            block != Blocks::flowing_lava && block != Blocks::lava)
        {
            unsigned int cached = mCache[index];
            if ((cached & 0x80000000) != 0)
                return cached & 0xf000f0;
            int lightColor = levelSource->getBlockState(pos)->getLightColor(levelSource, pos);
            mCache[index] |= (unsigned int)(lightColor & 0xf000f0) | 0x80000000;
            return lightColor;
        }
    }
    return levelSource->getBlockState(pos)->getLightColor(levelSource, pos);
}

bool BlockRenderer::isSolidBlockingAt(LevelSource* levelSource, const BlockPos& pos)
{
    if (mCache != NULL)
    {
        unsigned int index = (unsigned int)((pos.x - mCacheMinX) * 0x400 +
                                            (pos.y - mCacheMinY) * 0x20 +
                                            pos.z - mCacheMinZ);
        if ((index & 0xffff8000) == 0)
        {
            unsigned int cached = mCache[index];
            if ((cached & 0x40000000) != 0)
                return (cached & 1) != 0;
            const BlockState* state = levelSource->getBlockState(pos);
            bool solid = state->isCubeShaped() && !state->isTransparent();
            mCache[index] |= solid ? 0x40000001 : 0x40000000;
            return solid;
        }
    }
    const BlockState* state = levelSource->getBlockState(pos);
    return state->isCubeShaped() && !state->isTransparent();
}

float BlockRenderer::getShadeBrightness(LevelSource* levelSource, const BlockPos& pos)
{
    return isSolidBlockingAt(levelSource, pos) ? 0.2f : 1.0f;
}

bool BlockRenderer::tesselateBlockInWorld(const BlockState* state, const BlockPos& pos,
                                          float red, float green, float blue)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    float downRed = block == Blocks::grass ? 0.5f : red * 0.5f;
    float downGreen = block == Blocks::grass ? 0.5f : green * 0.5f;
    float downBlue = block == Blocks::grass ? 0.5f : blue * 0.5f;
    float northRed = block == Blocks::grass ? 0.8f : red * 0.8f;
    float northGreen = block == Blocks::grass ? 0.8f : green * 0.8f;
    float northBlue = block == Blocks::grass ? 0.8f : blue * 0.8f;
    float westRed = block == Blocks::grass ? 0.6f : red * 0.6f;
    float westGreen = block == Blocks::grass ? 0.6f : green * 0.6f;
    float westBlue = block == Blocks::grass ? 0.6f : blue * 0.6f;
    int blockLight = hasLightOverride() ? mLightOverride : getLightColor(block, mLevelSource, pos);
    bool rendered = false;

    if (mRenderAllFaces || state->shouldRenderFace(mLevelSource, pos, Direction::DOWN))
    {
        BlockPos lightPos = mMinY <= 0.0f ? pos.below() : pos;
        builder->tex2(hasLightOverride() ? mLightOverride :
                      (mMinY <= 0.0f ? getLightColor(block, mLevelSource, lightPos) : blockLight));
        builder->color(downRed, downGreen, downBlue);
        renderFaceDown(state, pos.x, pos.y, pos.z,
                       getTexture(state, mLevelSource, pos, Direction::DOWN));
        rendered = true;
    }
    if (mRenderAllFaces || state->shouldRenderFace(mLevelSource, pos, Direction::UP))
    {
        BlockPos lightPos = mMaxY >= 1.0f ? pos.above() : pos;
        builder->tex2(hasLightOverride() ? mLightOverride :
                      (mMaxY >= 1.0f ? getLightColor(block, mLevelSource, lightPos) : blockLight));
        builder->color(red, green, blue);
        renderFaceUp(state, pos.x, pos.y, pos.z,
                     getTexture(state, mLevelSource, pos, Direction::UP));
        rendered = true;
    }
    if (mRenderAllFaces || state->shouldRenderFace(mLevelSource, pos, Direction::NORTH))
    {
        BlockPos lightPos = mMinZ <= 0.0f ? pos.north() : pos;
        builder->tex2(hasLightOverride() ? mLightOverride :
                      (mMinZ <= 0.0f ? getLightColor(block, mLevelSource, lightPos) : blockLight));
        builder->color(northRed, northGreen, northBlue);
        renderNorth(state, pos.x, pos.y, pos.z,
                    getTexture(state, mLevelSource, pos, Direction::NORTH));
        rendered = true;
    }
    if (mRenderAllFaces || state->shouldRenderFace(mLevelSource, pos, Direction::SOUTH))
    {
        BlockPos lightPos = mMaxZ >= 1.0f ? pos.south() : pos;
        builder->tex2(hasLightOverride() ? mLightOverride :
                      (mMaxZ >= 1.0f ? getLightColor(block, mLevelSource, lightPos) : blockLight));
        builder->color(northRed, northGreen, northBlue);
        renderSouth(state, pos.x, pos.y, pos.z,
                    getTexture(state, mLevelSource, pos, Direction::SOUTH));
        rendered = true;
    }
    if (mRenderAllFaces || state->shouldRenderFace(mLevelSource, pos, Direction::WEST))
    {
        BlockPos lightPos = mMinX <= 0.0f ? pos.west() : pos;
        builder->tex2(hasLightOverride() ? mLightOverride :
                      (mMinX <= 0.0f ? getLightColor(block, mLevelSource, lightPos) : blockLight));
        builder->color(westRed, westGreen, westBlue);
        renderWest(state, pos.x, pos.y, pos.z,
                   getTexture(state, mLevelSource, pos, Direction::WEST));
        rendered = true;
    }
    if (mRenderAllFaces || state->shouldRenderFace(mLevelSource, pos, Direction::EAST))
    {
        BlockPos lightPos = mMaxX >= 1.0f ? pos.east() : pos;
        builder->tex2(hasLightOverride() ? mLightOverride :
                      (mMaxX >= 1.0f ? getLightColor(block, mLevelSource, lightPos) : blockLight));
        builder->color(westRed, westGreen, westBlue);
        renderEast(state, pos.x, pos.y, pos.z,
                   getTexture(state, mLevelSource, pos, Direction::EAST));
        rendered = true;
    }
    return rendered;
}

bool BlockRenderer::tesselateBlockInWorldWithAmbienceOcclusionTexLighting(
    const BlockState* state, const BlockPos& pos, float red, float green, float blue,
    int faceFlags, bool partialBounds)
{
    Block* block = state->getBlock();
    if (faceFlags == 0)
    {
        if (mRenderAllFaces)
            faceFlags = 0x3f;
        else
        {
            if (state->shouldRenderFace(mLevelSource, pos, Direction::DOWN)) faceFlags |= 1;
            if (state->shouldRenderFace(mLevelSource, pos, Direction::UP)) faceFlags |= 2;
            if (state->shouldRenderFace(mLevelSource, pos, Direction::NORTH)) faceFlags |= 4;
            if (state->shouldRenderFace(mLevelSource, pos, Direction::SOUTH)) faceFlags |= 8;
            if (state->shouldRenderFace(mLevelSource, pos, Direction::WEST)) faceFlags |= 0x10;
            if (state->shouldRenderFace(mLevelSource, pos, Direction::EAST)) faceFlags |= 0x20;
        }
    }
    if (faceFlags == 0 || (faceFlags == 1 && pos.y == 0))
        return false;

    mEnableAO = true;
    int lightColor = getLightColor(block, mLevelSource, pos);
    bool useColor = block != Blocks::grass;

    if ((faceFlags & 1) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_Down,
                    mMinY <= 0.0f, lightColor, useColor, red, green, blue, false);
        renderFaceDown(state, pos.x, pos.y, pos.z,
                       getTexture(state, mLevelSource, pos, Direction::DOWN));
    }
    if ((faceFlags & 2) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_Up,
                    mMaxY >= 1.0f, lightColor, true, red, green, blue, false);
        renderFaceUp(state, pos.x, pos.y, pos.z,
                     getTexture(state, mLevelSource, pos, Direction::UP));
    }
    if ((faceFlags & 4) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_North,
                    mMinZ <= 0.0f, lightColor, useColor, red, green, blue, partialBounds);
        renderNorth(state, pos.x, pos.y, pos.z,
                    getTexture(state, mLevelSource, pos, Direction::NORTH));
    }
    if ((faceFlags & 8) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_South,
                    mMaxZ >= 1.0f, lightColor, useColor, red, green, blue, partialBounds);
        renderSouth(state, pos.x, pos.y, pos.z,
                    getTexture(state, mLevelSource, pos, Direction::SOUTH));
    }
    if ((faceFlags & 0x10) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_West,
                    mMinX <= 0.0f, lightColor, useColor, red, green, blue, partialBounds);
        renderWest(state, pos.x, pos.y, pos.z,
                   getTexture(state, mLevelSource, pos, Direction::WEST));
    }
    if ((faceFlags & 0x20) != 0)
    {
        AOCalculate(block, mLevelSource, pos, Direction::eDirection_East,
                    mMaxX >= 1.0f, lightColor, useColor, red, green, blue, partialBounds);
        renderEast(state, pos.x, pos.y, pos.z,
                   getTexture(state, mLevelSource, pos, Direction::EAST));
    }

    mEnableAO = false;
    return true;
}

bool BlockRenderer::tesselateInWorld(const BlockState* state, const BlockPos& pos,
                                     const BlockState* actualState,
                                     boost::shared_ptr<BlockEntity> blockEntity)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    const BlockState* renderState = block->fillVirtualBlockStateProperties(state, mLevelSource, pos);
    int renderShape = renderState->getRenderShape();

    if (renderShape != 0)
        setShape(block->getShape(renderState, mLevelSource, pos));

    bool oldMipmap = builder->setMipmapEnable(block->mMipmap);
    bool rendered = false;

    switch (renderShape)
    {
    case 0:
    {
        int faces = block->getFaceFlags(renderState, mLevelSource, pos, mRenderAllFaces);
        if (faces != 0)
        {
            setShape(block->getShape(renderState, mLevelSource, pos));
            rendered = tesselateBlockInWorld(renderState, pos, faces);
        }
        break;
    }
    case 1: rendered = tesselateCrossInWorld(renderState, pos); break;
    case 2: rendered = tesselateTorchInWorld(renderState, pos); break;
    case 3: rendered = tesselateFireInWorld(renderState, pos); break;
    case 4: rendered = tesselateWaterInWorld(renderState, pos); break;
    case 5: rendered = tesselateDustInWorld(renderState, pos); break;
    case 6: rendered = tesselateRowInWorld(renderState, pos); break;
    case 7: rendered = tesselateDoorInWorld(renderState, pos); break;
    case 8: rendered = tesselateLadderInWorld(renderState, pos); break;
    case 9: rendered = tesselateRailInWorld(renderState, pos); break;
    case 10: rendered = tesselateStairsInWorld(renderState, pos); break;
    case 11: rendered = tesselateFenceInWorld(renderState, pos); break;
    case 12: rendered = tesselateLeverInWorld(renderState, pos); break;
    case 13: rendered = tesselateCactusInWorld(renderState, pos); break;
    case 14: rendered = tesselateBedInWorld(renderState, pos); break;
    case 15: rendered = tesselateRepeaterInWorld(renderState, pos); break;
    case 16: rendered = tesselatePistonBaseInWorld(renderState, pos, false, actualState); break;
    case 17: rendered = tesselatePistonExtensionInWorld(renderState, pos, true, actualState); break;
    case 18:
    case 40: rendered = tesselateThinFenceInWorld(renderState, pos); break;
    case 19: rendered = tesselateStemInWorld(renderState, pos); break;
    case 20: rendered = tesselateVineInWorld(renderState, pos); break;
    case 21: rendered = tesselateFenceGateInWorld(renderState, pos); break;
    case 23: rendered = tesselateLilypadInWorld(renderState, pos); break;
    case 24: rendered = tesselateCauldronInWorld(renderState, pos); break;
    case 25: rendered = tesselateBrewingStandInWorld(renderState, pos); break;
    case 26: rendered = tesselateAirPortalFrameInWorld(renderState, pos); break;
    case 27: rendered = tesselateEggInWorld(renderState, pos); break;
    case 28: rendered = tesselateCocoaInWorld(renderState, pos); break;
    case 29: rendered = tesselateTripwireSourceInWorld(renderState, pos); break;
    case 30: rendered = tesselateTripwireInWorld(renderState, pos); break;
    case 31: rendered = tesselateTreeInWorld(renderState, pos); break;
    case 32: rendered = tesselateWallInWorld(renderState, pos); break;
    case 33: rendered = tesselateFlowerPotInWorld(renderState, pos); break;
    case 34: rendered = tesselateBeaconInWorld(renderState, pos); break;
    case 35: rendered = tesselateAnvilInWorld(renderState, pos); break;
    case 36: rendered = tesselateDiodeInWorld(renderState, pos); break;
    case 37: rendered = tesselateComparatorInWorld(renderState, pos); break;
    case 38: rendered = tesselateHopperInWorld(renderState, pos); break;
    case 39: rendered = tesselateQuartzInWorld(renderState, pos); break;
    case 41: rendered = tesselateDoublePlantInWorld(renderState, pos); break;
    case 42: rendered = tesselateSlimeBlockInWorld(renderState, pos); break;
    case 43: rendered = tesselateTrapDoorInWorld(renderState, pos); break;
    case 44: rendered = tesselateEndRodInWorld(renderState, pos); break;
    case 45: rendered = tesselateChorusFlowerInWorld(renderState, pos); break;
    case 46: rendered = tesselateChorusPlantInWorld(renderState, pos); break;
    case 47: rendered = tesselateHorizontalDirectionalInWorld(renderState, pos); break;
    case 48: rendered = tesselateObserverInWorld(renderState, pos); break;
    case 49: rendered = tesselateKelpInWorld(renderState, pos); break;
    case 50: rendered = tesselateTurtleEggInWorld(renderState, pos); break;
    case 51:
        if (block == Blocks::coral_fan || block == Blocks::coral_fan_dead)
            rendered = tessellateCoralFanInWorld(renderState, pos);
        else
            rendered = tessellateCoralFanHangInWorld(renderState, pos);
        break;
    case 52: rendered = tessellateSeagrassInWorld(renderState, pos); break;
    case 53: rendered = tessellateSeaPickleInWorld(renderState, pos); break;
    case 55: rendered = tesselateCommandBlockInWorld(renderState, pos); break;
    }

    builder->setMipmapEnable(oldMipmap);
    return rendered;
}

void BlockRenderer::tesselateInWorldFixedTexture(const BlockState* state, const BlockPos& pos,
                                                 TextureAtlasSprite* texture)
{
    setFixedTexture(texture);
    tesselateInWorld(state, pos, NULL, boost::shared_ptr<BlockEntity>());
    clearFixedTexture();
}

bool BlockRenderer::tesselateInWorldNoCulling(const BlockState* state, const BlockPos& pos,
                                              const BlockState* actualState,
                                              boost::shared_ptr<BlockEntity> blockEntity)
{
    mRenderAllFaces = true;
    bool rendered = tesselateInWorld(state, pos, actualState, blockEntity);
    mRenderAllFaces = false;
    return rendered;
}

void BlockRenderer::tesselatePistonBaseForceExtended(const BlockState* state, const BlockPos& pos,
                                                     const BlockState* actualState)
{
    mRenderAllFaces = true;
    tesselatePistonBaseInWorld(state, pos, true, actualState);
    mRenderAllFaces = false;
}

void BlockRenderer::tesselatePistonArmNoCulling(const BlockState* state, const BlockPos& pos,
                                               bool shortHead, const BlockState* actualState)
{
    mRenderAllFaces = true;
    tesselatePistonExtensionInWorld(state, pos, shortHead, actualState);
    mRenderAllFaces = false;
}

void BlockRenderer::renderAllFaces(const BlockState* state)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    builder->begin();
    builder->normal(0.0f, -1.0f, 0.0f);
    renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
    builder->end();

    builder->begin();
    builder->normal(0.0f, 1.0f, 0.0f);
    renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
    builder->end();

    builder->begin();
    builder->normal(0.0f, 0.0f, -1.0f);
    renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
    builder->end();

    builder->begin();
    builder->normal(0.0f, 0.0f, 1.0f);
    renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
    builder->end();

    builder->begin();
    builder->normal(-1.0f, 0.0f, 0.0f);
    renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
    builder->end();

    builder->begin();
    builder->normal(1.0f, 0.0f, 0.0f);
    renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
    builder->end();
}

void BlockRenderer::tesselateEndRod(const BlockState* state, float x, float y, float z)
{
    setShape(0.4375f, 0.0f, 0.4375f, 0.5625f, 1.0f, 0.5625f);
    renderAllFaces(state);
    setShape(0.375f, 0.0f, 0.375f, 0.625f, 0.0625f, 0.625f);
    renderAllFaces(state);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

bool BlockRenderer::tesselateSlimeBlockInWorld(const BlockState* state, const BlockPos& pos)
{
    if (tesselateBlockInWorld(state, pos, 0x3f))
    {
        setFixedShape(0.1875f, 0.1875f, 0.1875f, 0.8125f, 0.8125f, 0.8125f);
        tesselateBlockInWorld(state, pos, 0x3f);
        clearFixedShape();
    }
    return true;
}

bool BlockRenderer::tesselateCrossInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;
    if (!mUseInventoryTint)
    {
        Vec3* offset = state->getOffset(mLevelSource, pos);
        x += (float)offset->x;
        y += (float)offset->y;
        z += (float)offset->z;
    }
    tesselateCrossTexture(state, x, y, z, 1.0f);
    return true;
}

bool BlockRenderer::tesselateRowInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    tesselateRowTexture(state, (float)pos.x, (float)pos.y - 0.0625f, (float)pos.z);
    return true;
}

bool BlockRenderer::tesselateCactusInWorld(const BlockState* state, const BlockPos& pos)
{
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    return tesselateCactusInWorld(state, pos, red, green, blue);
}

bool BlockRenderer::tesselateTurtleEggInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    builder->color(red, green, blue);
    tesselateTurtleEgg(state, pos, red, green, blue);
    return true;
}

bool BlockRenderer::tesselateEndRodInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    tesselateEndRod(state, (float)pos.x, (float)pos.y, (float)pos.z);
    return true;
}

void BlockRenderer::renderBlock(const BlockState* state, Level* level, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->begin();
    builder->tex2(getLightColor(state->getBlock(), level, pos));

    builder->color(0.5f, 0.5f, 0.5f);
    renderFaceDown(state, -0.5, -0.5, -0.5, getTexture(state, Direction::DOWN));
    builder->color(1.0f, 1.0f, 1.0f);
    renderFaceUp(state, -0.5, -0.5, -0.5, getTexture(state, Direction::UP));
    builder->color(0.8f, 0.8f, 0.8f);
    renderNorth(state, -0.5, -0.5, -0.5, getTexture(state, Direction::NORTH));
    builder->color(0.8f, 0.8f, 0.8f);
    renderSouth(state, -0.5, -0.5, -0.5, getTexture(state, Direction::SOUTH));
    builder->color(0.6f, 0.6f, 0.6f);
    renderWest(state, -0.5, -0.5, -0.5, getTexture(state, Direction::WEST));
    builder->color(0.6f, 0.6f, 0.6f);
    renderEast(state, -0.5, -0.5, -0.5, getTexture(state, Direction::EAST));
    builder->end();
}

void BlockRenderer::renderCube(const BlockState* state, float alpha)
{
    Block* block = state->getBlock();
    if (block->getRenderShape(state) != 0)
        return;

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    GlStateManager::translatef(-0.5f, -0.5f, -0.5f);
    builder->begin();
    builder->color(1.0f, 1.0f, 1.0f, alpha);
    renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
    builder->color(0.5f, 0.5f, 0.5f, alpha);
    renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
    builder->color(0.8f, 0.8f, 0.8f, alpha);
    renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
    renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
    builder->color(0.6f, 0.6f, 0.6f, alpha);
    renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
    renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
    builder->end();
    GlStateManager::translatef(0.5f, 0.5f, 0.5f);
}

void BlockRenderer::tesselateRowTexture(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float y1 = y + 1.0f;
    float xNear = x + 0.25f;
    float xFar = x + 0.75f;
    float zNear = z + 0.25f;
    float zFar = z + 0.75f;
    float xEnd = x + 1.0f;
    float zEnd = z + 1.0f;

    builder->vertexUV(xNear, y1, z, u0, v0);
    builder->vertexUV(xNear, y, z, u0, v1);
    builder->vertexUV(xNear, y, zEnd, u1, v1);
    builder->vertexUV(xNear, y1, zEnd, u1, v0);
    builder->vertexUV(xNear, y1, zEnd, u0, v0);
    builder->vertexUV(xNear, y, zEnd, u0, v1);
    builder->vertexUV(xNear, y, z, u1, v1);
    builder->vertexUV(xNear, y1, z, u1, v0);

    builder->vertexUV(xFar, y1, zEnd, u0, v0);
    builder->vertexUV(xFar, y, zEnd, u0, v1);
    builder->vertexUV(xFar, y, z, u1, v1);
    builder->vertexUV(xFar, y1, z, u1, v0);
    builder->vertexUV(xFar, y1, z, u0, v0);
    builder->vertexUV(xFar, y, z, u0, v1);
    builder->vertexUV(xFar, y, zEnd, u1, v1);
    builder->vertexUV(xFar, y1, zEnd, u1, v0);

    builder->vertexUV(x, y1, zNear, u0, v0);
    builder->vertexUV(x, y, zNear, u0, v1);
    builder->vertexUV(xEnd, y, zNear, u1, v1);
    builder->vertexUV(xEnd, y1, zNear, u1, v0);
    builder->vertexUV(xEnd, y1, zNear, u0, v0);
    builder->vertexUV(xEnd, y, zNear, u0, v1);
    builder->vertexUV(x, y, zNear, u1, v1);
    builder->vertexUV(x, y1, zNear, u1, v0);

    builder->vertexUV(xEnd, y1, zFar, u0, v0);
    builder->vertexUV(xEnd, y, zFar, u0, v1);
    builder->vertexUV(x, y, zFar, u1, v1);
    builder->vertexUV(x, y1, zFar, u1, v0);
    builder->vertexUV(x, y1, zFar, u0, v0);
    builder->vertexUV(x, y, zFar, u0, v1);
    builder->vertexUV(xEnd, y, zFar, u1, v1);
    builder->vertexUV(xEnd, y1, zFar, u1, v0);
}

void BlockRenderer::tesselateTorch(const BlockState* state, float x, float y, float z, float xTilt, float zTilt)
{
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float headU0 = (float)texture->getU(2.4375);
    float headV0 = (float)texture->getV(2.375);
    float headU1 = (float)texture->getU(2.53125);
    float headV1 = (float)texture->getV(2.5);
    texture->adjustUVs(headU0, headU1, headV0, headV1);

    float capU0 = (float)texture->getU(2.4375);
    float capV0 = (float)texture->getV(2.65625);
    float capU1 = (float)texture->getU(2.53125);
    float capV1 = (float)texture->getV(2.71875);
    texture->adjustUVs(capU0, capU1, capV0, capV1);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

    float top = y + 0.625f;
    float xc = x + 0.5f + xTilt * 0.375f;
    float zc = z + 0.5f + zTilt * 0.375f;
    float xc0 = xc - 0.0625f;
    float xc1 = xc + 0.0625f;
    float zc0 = zc - 0.0625f;
    float zc1 = zc + 0.0625f;

    builder->vertexUV(xc0, top, zc0, headU0, headV0);
    builder->vertexUV(xc0, top, zc1, headU0, headV1);
    builder->vertexUV(xc1, top, zc1, headU1, headV1);
    builder->vertexUV(xc1, top, zc0, headU1, headV0);

    float xb0 = x + 0.4375f;
    float xb1 = x + 0.5625f;
    float zb0 = z + 0.4375f;
    float zb1 = z + 0.5625f;

    builder->vertexUV(xb1 + xTilt, y, zb0 + zTilt, capU1, capV0);
    builder->vertexUV(xb1 + xTilt, y, zb1 + zTilt, capU1, capV1);
    builder->vertexUV(xb0 + xTilt, y, zb1 + zTilt, capU0, capV1);
    builder->vertexUV(xb0 + xTilt, y, zb0 + zTilt, capU0, capV0);

    float yTop = y + 1.0f;

    builder->vertexUV(xb0, yTop, z, u0, v0);
    builder->vertexUV(xb0 + xTilt, y, z + zTilt, u0, v1);
    builder->vertexUV(xb0 + xTilt, y, z + 1.0f + zTilt, u1, v1);
    builder->vertexUV(xb0, yTop, z + 1.0f, u1, v0);
    builder->vertexUV(xb1, yTop, z + 1.0f, u0, v0);
    builder->vertexUV(xb1 + xTilt, y, z + 1.0f + zTilt, u0, v1);
    builder->vertexUV(xb1 + xTilt, y, z + zTilt, u1, v1);
    builder->vertexUV(xb1, yTop, z, u1, v0);

    builder->vertexUV(x, yTop, zb1, u0, v0);
    builder->vertexUV(x + xTilt, y, zb1 + zTilt, u0, v1);
    builder->vertexUV(x + 1.0f + xTilt, y, zb1 + zTilt, u1, v1);
    builder->vertexUV(x + 1.0f, yTop, zb1, u1, v0);
    builder->vertexUV(x + 1.0f, yTop, zb0, u0, v0);
    builder->vertexUV(x + 1.0f + xTilt, y, zb0 + zTilt, u0, v1);
    builder->vertexUV(x + xTilt, y, zb0 + zTilt, u1, v1);
    builder->vertexUV(x, yTop, zb0, u1, v0);
}

bool BlockRenderer::tesselateBlockInWorld(const BlockState* state, const BlockPos& pos)
{
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    int faces = state->getBlock()->getFaceFlags(state, mLevelSource, pos, mRenderAllFaces);
    if (mUseAmbientOcclusion && Minecraft::useAmbientOcclusion() && !hasLightOverride())
        return tesselateBlockInWorldWithAmbienceOcclusionTexLighting(state, pos, red, green, blue, faces, mShapeOutside);
    return tesselateBlockInWorld(state, pos, red, green, blue);
}

bool BlockRenderer::tesselateBlockInWorld(const BlockState* state, const BlockPos& pos, int faces)
{
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (mUseAmbientOcclusion && Minecraft::useAmbientOcclusion() && !hasLightOverride())
        return tesselateBlockInWorldWithAmbienceOcclusionTexLighting(state, pos, red, green, blue, faces, mShapeOutside);
    return tesselateBlockInWorld(state, pos, red, green, blue);
}

void BlockRenderer::tesselateAnvilPiece(const BlockState* state, const BlockPos& pos, int part, float x0, float y0, float x1, float y1, bool rotate, bool flip)
{
    float widthA = rotate ? y1 : y0;
    float widthB = rotate ? y0 : y1;

    setShape(0.5f - widthA * 0.5f, x0, 0.5f - widthB * 0.5f, widthA * 0.5f + 0.5f, x0 + x1, widthB * 0.5f + 0.5f);

    if (!flip)
    {
        tesselateBlockInWorld(state, pos);
    }
    else
    {
        BufferBuilder* builder = Tesselator::getInstance()->getBuilder();

        builder->begin();
        builder->normal(0.0f, -1.0f, 0.0f);
        renderFaceDown(state, 0.0, 0.0, 0.0, getTexture(state, Direction::DOWN));
        builder->end();

        builder->begin();
        builder->normal(0.0f, 1.0f, 0.0f);
        renderFaceUp(state, 0.0, 0.0, 0.0, getTexture(state, Direction::UP));
        builder->end();

        builder->begin();
        builder->normal(0.0f, 0.0f, -1.0f);
        renderNorth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::NORTH));
        builder->end();

        builder->begin();
        builder->normal(0.0f, 0.0f, 1.0f);
        renderSouth(state, 0.0, 0.0, 0.0, getTexture(state, Direction::SOUTH));
        builder->end();

        builder->begin();
        builder->normal(-1.0f, 0.0f, 0.0f);
        renderWest(state, 0.0, 0.0, 0.0, getTexture(state, Direction::WEST));
        builder->end();

        builder->begin();
        builder->normal(1.0f, 0.0f, 0.0f);
        renderEast(state, 0.0, 0.0, 0.0, getTexture(state, Direction::EAST));
        builder->end();
    }
}

bool BlockRenderer::tesselateAnvilInWorld(const BlockState* state, const BlockPos& pos, bool inventory)
{
    bool allFaces = mRenderAllFaces;
    mRenderAllFaces = inventory || allFaces;
    tesselateAnvilPiece(state, pos, 0, 0.125f, 0.0f, 0.875f, 0.25f, false, false);
    tesselateAnvilPiece(state, pos, 1, 0.1875f, 0.25f, 0.8125f, 0.75f, false, false);
    tesselateAnvilPiece(state, pos, 2, 0.0f, 0.75f, 1.0f, 1.0f, true, false);
    mRenderAllFaces = allFaces;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return true;
}

bool BlockRenderer::tesselateHopperInWorld(const BlockState* state, const BlockPos& pos, bool inventory)
{
    bool allFaces = mRenderAllFaces;
    mRenderAllFaces = inventory || allFaces;
    setShape(0.0f, 0.625f, 0.0f, 1.0f, 1.0f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.25f, 0.25f, 0.25f, 0.75f, 0.625f, 0.75f);
    rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    setShape(0.375f, 0.0f, 0.375f, 0.625f, 0.25f, 0.625f);
    rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    mRenderAllFaces = allFaces;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

void BlockRenderer::tesselateEndRodCenter(const BlockState* state, float x, float y, float z)
{
    setShape(0.4375f, 0.0f, 0.4375f, 0.5625f, 1.0f, 0.5625f);
    renderAllFaces(state);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockRenderer::tesselateEndRodBase(const BlockState* state, float x, float y, float z)
{
    setShape(0.375f, 0.0f, 0.375f, 0.625f, 0.0625f, 0.625f);
    renderAllFaces(state);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

void BlockRenderer::renderBlock(const BlockState* state, float brightness, float alpha, bool tint, bool inventory)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    bool oldTint = mUseInventoryTint;
    bool oldFaces = mRenderAllFaces;
    mUseInventoryTint = tint;
    mRenderAllFaces = inventory;
    builder->color(brightness, brightness, brightness, alpha);
    renderAllFaces(state);
    mRenderAllFaces = oldFaces;
    mUseInventoryTint = oldTint;
}

bool BlockRenderer::tesselateTrapDoorInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(state->getShape(mLevelSource, pos));
    bool rendered = tesselateBlockInWorld(state, pos);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateTreeInWorld(const BlockState* state, const BlockPos& pos)
{
    mFaceRotation0 = mFaceRotation1 = mFaceRotation2 = mFaceRotation3 = 1;
    bool rendered = tesselateBlockInWorld(state, pos);
    mFaceRotation0 = mFaceRotation1 = mFaceRotation2 = mFaceRotation3 = 0;
    return rendered;
}

bool BlockRenderer::tesselateQuartzInWorld(const BlockState* state, const BlockPos& pos)
{
    bool rendered = tesselateBlockInWorld(state, pos);
    mFaceRotation0 = 0;
    mFaceRotation1 = 0;
    mFaceRotation2 = 0;
    mFaceRotation3 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    return rendered;
}

bool BlockRenderer::tesselateHorizontalDirectionalInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* facing = state->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
    if (facing == Direction::SOUTH)
    {
        mFaceTransform3 = 0;
        mFaceRotation4 = 0;
        mFaceTransform5 = 1;
        mFaceRotation1 = 2;
        mFaceRotation0 = 1;
        mFaceRotation5 = 0;
        mFaceTransform2 = 3;
        mFaceTransform0 = 3;
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform0 = 0;
        mFaceTransform3 = 0;
        mFaceTransform4 = 0;
        mFaceRotation0 = 0;
        mFaceRotation1 = 0;
        mFaceTransform5 = 0;
        mFaceRotation4 = 0;
        mFaceTransform1 = 0;
        mFaceRotation2 = 0;
        mFaceTransform2 = 0;
        mFaceRotation3 = 0;
        mFaceRotation5 = 0;
        return rendered;
    }
    else if (facing == Direction::NORTH)
    {
        mFaceTransform2 = 0;
        mFaceTransform0 = 2;
        mFaceRotation1 = 1;
        mFaceRotation5 = 3;
        mFaceRotation0 = 2;
        mFaceTransform3 = 3;
        mFaceTransform5 = 1;
        mFaceRotation4 = 3;
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform2 = 0;
        mFaceTransform4 = 0;
        mFaceRotation2 = 0;
        mFaceTransform1 = 0;
        mFaceRotation3 = 0;
        mFaceTransform0 = 0;
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
        mFaceRotation0 = 0;
        mFaceTransform3 = 0;
        mFaceTransform5 = 0;
        mFaceRotation1 = 0;
        return rendered;
    }
    else if (facing == Direction::WEST)
    {
        mFaceTransform2 = 2;
        mFaceRotation5 = 2;
        mFaceRotation3 = 2;
        mFaceTransform5 = 1;
        mFaceTransform1 = 3;
        mFaceRotation2 = 1;
        mFaceTransform0 = 0;
        mFaceRotation4 = 1;
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform0 = 0;
        mFaceRotation0 = 0;
        mFaceRotation2 = 0;
        mFaceRotation1 = 0;
        mFaceTransform2 = 0;
        mFaceTransform5 = 0;
        mFaceTransform1 = 0;
        mFaceRotation5 = 0;
        mFaceRotation4 = 0;
        mFaceRotation3 = 0;
        mFaceTransform3 = 0;
        mFaceTransform4 = 0;
        return rendered;
    }
    else
    {
        if (facing == Direction::EAST)
        {
            mFaceTransform1 = 0;
            mFaceRotation5 = 1;
            mFaceTransform0 = 3;
            mFaceTransform2 = 2;
            mFaceTransform5 = 1;
            mFaceRotation4 = 2;
            mFaceRotation3 = 1;
            mFaceRotation2 = 2;
        }
        bool rendered = tesselateBlockInWorld(state, pos);
        mFaceTransform4 = 0;
        mFaceTransform2 = 0;
        mFaceTransform1 = 0;
        mFaceRotation2 = 0;
        mFaceTransform0 = 0;
        mFaceRotation3 = 0;
        mFaceRotation5 = 0;
        mFaceRotation4 = 0;
        mFaceTransform3 = 0;
        mFaceRotation0 = 0;
        mFaceRotation1 = 0;
        mFaceTransform5 = 0;
        return rendered;
    }
}

float BlockRenderer::getWaterHeight(const BlockPos& pos, Material* material)
{
    float total = 0.0f;
    for (int x = 0; x < 2; ++x)
        for (int z = 0; z < 2; ++z)
        {
            if (mLevelSource->getBlockState(pos.offset(x, 1, z))->getMaterial() == material)
                return 1.0f;
            const BlockState* state = mLevelSource->getBlockState(pos.offset(x, 0, z));
            total += state->getMaterial() == material ? 0.875f : 1.0f;
        }
    return 1.0f - total * 0.25f;
}

bool BlockRenderer::tesselateWaterInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((float)(color >> 16 & 255) / 255.0f, (float)(color >> 8 & 255) / 255.0f, (float)(color & 255) / 255.0f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, getWaterHeight(pos, state->getMaterial()), 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateCactusInWorld(const BlockState* state, const BlockPos& pos, float red, float green, float blue)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    int lightAtPos = block->getLightColor(state, mLevelSource, pos, -1);

    if (mMinY <= 0.0f)
    {
        BlockPos lightPos = pos.offset(0, -1, 0);
        builder->tex2(block->getLightColor(state, mLevelSource, lightPos, -1));
    }
    else
    {
        builder->tex2(lightAtPos);
    }
    builder->color(red * 0.5f, green * 0.5f, blue * 0.5f);
    renderFaceDown(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::DOWN));

    if (mMaxY >= 1.0f)
    {
        BlockPos lightPos = pos.offset(0, 1, 0);
        builder->tex2(block->getLightColor(state, mLevelSource, lightPos, -1));
    }
    else
    {
        builder->tex2(lightAtPos);
    }
    builder->color(red, green, blue);
    renderFaceUp(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::UP));

    builder->tex2(lightAtPos);
    builder->color(red * 0.8f, green * 0.8f, blue * 0.8f);
    builder->addOffset(0.0f, 0.0f, 0.0625f);
    renderNorth(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::NORTH));
    builder->addOffset(0.0f, 0.0f, -0.0625f);
    builder->addOffset(0.0f, 0.0f, -0.0625f);
    renderSouth(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::SOUTH));
    builder->addOffset(0.0f, 0.0f, 0.0625f);

    builder->color(red * 0.6f, green * 0.6f, blue * 0.6f);
    builder->addOffset(0.0625f, 0.0f, 0.0f);
    renderWest(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::WEST));
    builder->addOffset(-0.0625f, 0.0f, 0.0f);
    builder->addOffset(-0.0625f, 0.0f, 0.0f);
    renderEast(state, pos.x, pos.y, pos.z, getTexture(state, mLevelSource, pos, Direction::EAST));
    builder->addOffset(0.0625f, 0.0f, 0.0f);

    return true;
}

void BlockRenderer::tesselateSunflowerCrossTexture(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;
    float u1 = (float)texture->getU1(true);
    float v0 = (float)texture->getV(8.0);
    float u0 = (float)texture->getU0(true);
    float v1 = (float)texture->getV1(true);
    float y1 = y + 0.5f;
    float x0 = x + 0.05f;
    float x1 = x + 0.95f;
    float z0 = z + 0.05f;
    float z1 = z + 0.95f;
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y, z0, u0, v1);
    builder->vertexUV(x1, y, z1, u1, v1);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x1, y1, z1, u1, v0);
    builder->vertexUV(x1, y, z1, u1, v1);
    builder->vertexUV(x0, y, z0, u0, v1);
    builder->vertexUV(x0, y1, z0, u0, v0);
    builder->vertexUV(x0, y1, z1, u0, v0);
    builder->vertexUV(x0, y, z1, u0, v1);
    builder->vertexUV(x1, y, z0, u1, v1);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y1, z0, u1, v0);
    builder->vertexUV(x1, y, z0, u1, v1);
    builder->vertexUV(x0, y, z1, u0, v1);
    builder->vertexUV(x0, y1, z1, u0, v0);
}

void BlockRenderer::tesselateSunflowerDirTexture(const BlockState* state, float x, float y, float z)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    DoublePlantBlock* block = static_cast<DoublePlantBlock*>(state->getBlock());
    TextureAtlasSprite* sideTexture = block->getTexture(state, DoublePlantBlock::Variant::SIDE);
    TextureAtlasSprite* inventoryTexture = block->getTexture(state, DoublePlantBlock::Variant::INVENTORY);
    float sideU0 = (float)sideTexture->getU0(true);
    float sideV1 = (float)sideTexture->getV1(true);
    float sideU1 = (float)sideTexture->getU1(true);
    float sideV0 = (float)sideTexture->getV0(true);
    float invU0 = (float)inventoryTexture->getU0(true);
    float invV1 = (float)inventoryTexture->getV1(true);
    float invU1 = (float)inventoryTexture->getU1(true);
    float invV0 = (float)inventoryTexture->getV0(true);
    float angle = 22.5f * 0.01745329238474369f;
    float cosAngle = Mth::cos(angle);
    float sinAngle = Mth::sin(angle);
    float leftX = x + 0.5f + 0.5625f * sinAngle + 0.1f * cosAngle;
    float leftY = y + 0.5f - 0.5625f * cosAngle + 0.1f * sinAngle;
    float rightX = x + 0.5f - 0.4375f * sinAngle + 0.1f * cosAngle;
    float rightY = y + 0.5f + 0.4375f * cosAngle + 0.1f * sinAngle;
    float z0 = z + 0.0625f;
    float z1 = z + 0.9375f;
    builder->vertexUV(leftX, leftY, z0, sideU0, sideV0);
    builder->vertexUV(rightX, rightY, z0, sideU0, sideV1);
    builder->vertexUV(rightX, rightY, z1, sideU1, sideV1);
    builder->vertexUV(leftX, leftY, z1, sideU1, sideV0);
    builder->vertexUV(leftX, leftY, z1, invU0, invV0);
    builder->vertexUV(rightX, rightY, z1, invU0, invV1);
    builder->vertexUV(rightX, rightY, z0, invU1, invV1);
    builder->vertexUV(leftX, leftY, z0, invU1, invV0);
}

bool BlockRenderer::tesselateDoublePlantInWorld(const BlockState* state, const BlockPos& pos)
{
    if (state->getValue(DoublePlantBlock::HALF) == DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER])
    {
        const BlockState* below = mLevelSource->getBlockState(pos.below());
        if (below->getBlock() != Blocks::double_plant)
            return false;
    }

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        float newRed = (blue * 11.0f + green * 59.0f + red * 30.0f) / 100.0f;
        float newGreen = (green * 70.0f + red * 30.0f) / 100.0f;
        float newBlue = (blue * 70.0f + red * 30.0f) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;

    DoublePlantBlock* block = static_cast<DoublePlantBlock*>(state->getBlock());
    const BlockState* virtualState = block->fillVirtualBlockStateProperties(state, mLevelSource, pos);
    if (virtualState->getValue(DoublePlantBlock::VARIANT) == DoublePlantBlock::VARIANTS[0] &&
        virtualState->getValue(DoublePlantBlock::HALF) == DoublePlantBlock::HALVES[DoublePlantBlock::Half::UPPER])
    {
        tesselateSunflowerCrossTexture(state, x, y, z);
        tesselateSunflowerDirTexture(state, x, y, z);
    }
    else
    {
        tesselateCrossTexture(state, x, y, z, 1.0f);
    }
    return true;
}

bool BlockRenderer::tesselateKelpInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        float newRed = (blue * 11.0f + green * 59.0f + red * 30.0f) / 100.0f;
        float newGreen = (green * 70.0f + red * 30.0f) / 100.0f;
        float newBlue = (blue * 70.0f + red * 30.0f) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y;
    float z = (float)pos.z;

    int aboveId = mLevelSource->getBlockId((int)pos.x, (int)pos.y + 1, (int)pos.z);
    if (aboveId == 258)
    {
        int dir1 = Mth::abs(pos.getRandomSeed()) % 4;
        int dir2 = (dir1 + Mth::abs(pos.getRandomSeed()) % 3 + 1) % 4;
        TextureAtlasSprite* texture1 = getTexture(state, Direction::from3DDataValue(dir1));
        TextureAtlasSprite* texture2 = getTexture(state, Direction::from3DDataValue(dir2));
        tesselateCrossTexture(texture1, texture2, x, y, z, 1.0f);
    }
    else
    {
        TextureAtlasSprite* texture = getTexture(state, Direction::WEST);
        tesselateCrossTexture(texture, texture, x, y, z, 1.0f);
    }
    return true;
}

bool BlockRenderer::tessellateCoralFanInWorld(const BlockState* state, const BlockPos& pos)
{
    tesselateCrossTexture(state, (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    return true;
}

bool BlockRenderer::tessellateCoralFanHangInWorld(const BlockState* state, const BlockPos& pos)
{
    tesselateStemDirTexture(state, Direction::DOWN, (float)pos.x, (float)pos.y, (float)pos.z, 0.75f);
    return true;
}

void BlockRenderer::tesselateStemDirTexture(const BlockState* state, const Direction* direction, float x, float y, float z, float scale)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    StemBlock* stemBlock = static_cast<StemBlock*>(state->getBlock());
    TextureAtlasSprite* texture = stemBlock->getAngledTexture();
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    float yPlus1 = y + 1.0f;
    float yHalf = y + 0.5f;
    float scaleHalf = scale + 0.5f;
    float scalePlus1 = scale + 1.0f;
    float xz = z + x;

    float ua = u0;
    float ub = u1;
    if (direction == Direction::NORTH || direction == Direction::EAST)
    {
        ua = u1;
        ub = u0;
    }

    if (direction == Direction::WEST || direction == Direction::EAST)
    {
        builder->vertexUV(y, xz, scaleHalf, ua, v0);
        builder->vertexUV(y, z, scaleHalf, ua, v1);
        builder->vertexUV(yPlus1, z, scaleHalf, ub, v1);
        builder->vertexUV(yPlus1, xz, scaleHalf, ub, v0);
        builder->vertexUV(yPlus1, xz, scaleHalf, ub, v0);
        builder->vertexUV(yPlus1, z, scaleHalf, ub, v1);
        builder->vertexUV(y, z, scaleHalf, ua, v1);
        builder->vertexUV(y, xz, scaleHalf, ua, v0);
    }
    else
    {
        builder->vertexUV(yHalf, xz, scalePlus1, ua, v0);
        builder->vertexUV(yHalf, z, scalePlus1, ua, v1);
        builder->vertexUV(yHalf, z, scale, ub, v1);
        builder->vertexUV(yHalf, xz, scale, ub, v0);
        builder->vertexUV(yHalf, xz, scale, ub, v0);
        builder->vertexUV(yHalf, z, scale, ub, v1);
        builder->vertexUV(yHalf, z, scalePlus1, ua, v1);
        builder->vertexUV(yHalf, xz, scalePlus1, ua, v0);
    }
}

bool BlockRenderer::tesselateStemInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float red = (float)(color >> 16 & 255) / 255.0f;
    float green = (float)(color >> 8 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    if (mMinecraft->mOptions->mAnaglyph3d)
    {
        float newRed = (blue * 11.0f + green * 59.0f + red * 30.0f) / 100.0f;
        float newGreen = (green * 70.0f + red * 30.0f) / 100.0f;
        float newBlue = (blue * 70.0f + red * 30.0f) / 100.0f;
        red = newRed;
        green = newGreen;
        blue = newBlue;
    }
    builder->color(red, green, blue);

    float x = (float)pos.x;
    float y = (float)pos.y - 0.0625f;
    float z = (float)pos.z;

    const Direction* direction = state->getValue<const Direction*>(StemBlock::FACING);
    if (direction == Direction::UP)
    {
        tesselateStemTexture(state, x, y, z, mMaxY);
    }
    else
    {
        tesselateStemTexture(state, x, y, z, 0.5f);
        tesselateStemDirTexture(state, direction, x, y, z, mMaxY);
    }
    return true;
}

bool BlockRenderer::tesselateLilypadInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    if (hasFixedTexture())
        texture = mFixedTexture;

    float u0 = (float)texture->getU0(true);
    float v0 = (float)texture->getV0(true);
    float u1 = (float)texture->getU1(true);
    float v1 = (float)texture->getV1(true);

    unsigned int hash = (unsigned int)pos.x * 0x2fc20fu ^ (unsigned int)pos.z * 0x6ebfff5u ^ (unsigned int)pos.y;
    hash = hash * hash * 0x285b825u + hash * 0xbu >> 0x10;
    unsigned int rotation = hash & 3;
    unsigned int rotation1 = rotation + 1;

    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));

    float x0 = (float)pos.x + 0.5f;
    float z0 = (float)pos.z + 0.5f;
    float y = (float)pos.y + 0.015625f;
    float xOff = (float)(int)(rotation & 1) * 0.5f * (float)((int)(rotation >> 1) * -2 + 1);
    float zOff = (float)(int)(rotation1 & 1) * 0.5f * (float)((int)((rotation1 >> 1) & 1) * -2 + 1);

    float a = x0 + xOff;
    float b = z0 + xOff;
    float c = x0 - xOff;
    float d = z0 - xOff;

    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color(color);
    builder->vertexUV(a - zOff, y, b + zOff, u0, v0);
    builder->vertexUV(a + zOff, y, d + zOff, u1, v0);
    builder->vertexUV(c + zOff, y, d - zOff, u1, v1);
    builder->vertexUV(c - zOff, y, b - zOff, u0, v1);

    color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    builder->color((color & 0xfefefe) >> 1);
    builder->vertexUV(c - zOff, y, b - zOff, u0, v1);
    builder->vertexUV(c + zOff, y, d - zOff, u1, v1);
    builder->vertexUV(a + zOff, y, d + zOff, u1, v0);
    builder->vertexUV(a - zOff, y, b + zOff, u0, v0);

    return true;
}

bool BlockRenderer::tesselateTorchInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);
    tesselateTorch(state, (float)pos.x + 0.5f, (float)pos.y, (float)pos.z + 0.5f, 0.0f, 0.0f);
    return true;
}

bool BlockRenderer::tesselateFireInWorld(const BlockState* state, const BlockPos& pos)
{
    tesselateCrossTexture(state, (float)pos.x, (float)pos.y, (float)pos.z, 1.0f);
    tesselateCrossTexture(state, (float)pos.x, (float)pos.y + 0.5f, (float)pos.z, 1.0f);
    return true;
}

bool BlockRenderer::tesselateDustInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.03125f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateLadderInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0f, 0.9375f, 1.0f, 1.0f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateDoorInWorld(const BlockState* state, const BlockPos& pos)
{
    if (state->getValue(DoorBlock::HALF) == DoorBlock::UPPER)
    {
        if (mLevelSource->getBlockState(pos.below())->getBlock() != state->getBlock())
            return false;
    }
    else if (mLevelSource->getBlockState(pos.above())->getBlock() != state->getBlock())
    {
        return false;
    }

    Block* block = state->getBlock();
    bool rightHinge = state->getValue(DoorBlock::HINGE) == DoorBlock::RIGHT;
    const Direction* facing = state->getValue(DoorBlock::FACING);
    unsigned int direction = facing->get3DDataValue();
    bool open = state->getValue(DoorBlock::OPEN);
    bool rendered;

    if (!open)
    {
        int* transformA = NULL;
        int* transformB = NULL;
        int rotation4Alt = 0;
        int rotation4Value = 0;

        if (direction == Direction::eDirection_North || direction == Direction::eDirection_South)
        {
            transformA = &mFaceTransform2;
            rotation4Value = 3;
            transformB = &mFaceTransform3;
            mFaceRotation3 = 3;
        }
        if (direction == Direction::eDirection_West || direction == Direction::eDirection_East)
        {
            transformA = &mFaceTransform0;
            transformB = &mFaceTransform1;
            rotation4Alt = 2;
            rotation4Value = 1;
            mFaceRotation1 = 3;
        }

        direction = facing->get3DDataValue();
        if (direction > Direction::eDirection_Up)
        {
            if (direction == Direction::eDirection_North)
            {
                mFaceRotation4 = rotation4Alt;
                if (rightHinge)
                {
                    *transformA = 2;
                    *transformB = 3;
                    mFaceTransform4 = 2;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
                *transformB = 1;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
            if (direction > Direction::eDirection_South)
            {
                if (direction == Direction::eDirection_West)
                {
                    mFaceRotation4 = rotation4Alt;
                    if (rightHinge)
                    {
                        *transformA = 2;
                        *transformB = 3;
                        mFaceTransform4 = 2;
                        setShape(block->getShape(state, mLevelSource, pos));
                        rendered = tesselateBlockInWorld(state, pos);
                        goto resetAndReturn;
                    }
                    *transformB = 1;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
                if (direction != Direction::eDirection_East)
                {
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
            }
            mFaceRotation4 = rotation4Value;
            if (rightHinge)
            {
                *transformB = 1;
                mFaceTransform4 = 2;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
            *transformA = 2;
            *transformB = 3;
        }
    }
    else if (direction > Direction::eDirection_Up)
    {
        if (direction > Direction::eDirection_South)
        {
            if (direction > Direction::eDirection_East)
            {
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }

            if (rightHinge)
            {
                mFaceTransform2 = 2;
                mFaceTransform3 = 1;
                mFaceRotation3 = 3;
                mFaceTransform4 = 1;
                if (facing->get3DDataValue() == Direction::eDirection_West)
                {
                    mFaceRotation4 = 3;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
            }
            else
            {
                mFaceTransform2 = 3;
                mFaceRotation2 = 3;
                mFaceTransform4 = 3;
                if (facing->get3DDataValue() != Direction::eDirection_West)
                {
                    mFaceRotation4 = 3;
                    setShape(block->getShape(state, mLevelSource, pos));
                    rendered = tesselateBlockInWorld(state, pos);
                    goto resetAndReturn;
                }
            }
            mFaceRotation4 = 0;
            setShape(block->getShape(state, mLevelSource, pos));
            rendered = tesselateBlockInWorld(state, pos);
            goto resetAndReturn;
        }

        if (rightHinge)
        {
            mFaceTransform0 = 2;
            mFaceTransform1 = 1;
            mFaceRotation1 = 3;
            mFaceTransform4 = 1;
            if (direction != Direction::eDirection_North)
            {
                mFaceRotation4 = 1;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
        }
        else
        {
            mFaceTransform4 = 3;
            mFaceTransform0 = 3;
            mFaceRotation0 = 3;
            if (direction == Direction::eDirection_North)
            {
                mFaceRotation4 = 1;
                setShape(block->getShape(state, mLevelSource, pos));
                rendered = tesselateBlockInWorld(state, pos);
                goto resetAndReturn;
            }
        }
        mFaceRotation4 = 2;
        setShape(block->getShape(state, mLevelSource, pos));
        rendered = tesselateBlockInWorld(state, pos);
        goto resetAndReturn;
    }

    setShape(block->getShape(state, mLevelSource, pos));
    rendered = tesselateBlockInWorld(state, pos);

resetAndReturn:
    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    return rendered;
}

bool BlockRenderer::tesselateRailInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.0625f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateStairsInWorld(const BlockState* state, const BlockPos& pos)
{
    Block* block = state->getBlock();
    const arrayWithLength<const AABB*>* shapes = block->getShapes(state, mLevelSource, pos);

    for (unsigned int i = 0; i < shapes->length; i++)
    {
        if (shapes->data[i] != NULL)
        {
            setShape(shapes->data[i]);
            tesselateBlockInWorld(state, pos);
        }
    }

    return true;
}

bool BlockRenderer::tesselateEggInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.125f, 0.0f, 0.125f, 0.875f, 0.875f, 0.875f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateFenceInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.375f, 0.0f, 0.375f, 0.625f, 1.0f, 0.625f);
    return tesselateBlockInWorld(state, pos);
}

bool BlockRenderer::tesselateWallInWorld(const BlockState* state, const BlockPos& pos)
{
    WallBlock* wallBlock = static_cast<WallBlock*>(state->getBlock());
    bool east = wallBlock->connectsTo(mLevelSource, pos.offset(-1, 0, 0), Direction::EAST);
    bool west = wallBlock->connectsTo(mLevelSource, pos.offset(1, 0, 0), Direction::WEST);
    bool south = wallBlock->connectsTo(mLevelSource, pos.offset(0, 0, -1), Direction::SOUTH);
    bool north = wallBlock->connectsTo(mLevelSource, pos.offset(0, 0, 1), Direction::NORTH);

    bool northSouthThrough = south && north && !east && !west;
    bool eastWestThrough = !south && !north && east && west;
    bool aboveEmpty = mLevelSource->isEmptyBlock(pos.offset(0, 1, 0));

    if ((northSouthThrough || eastWestThrough) && aboveEmpty)
    {
        if (northSouthThrough)
            setShape(0.3125f, 0.0f, 0.0f, 0.6875f, 0.875f, 1.0f);
        else
            setShape(0.0f, 0.0f, 0.3125f, 1.0f, 0.875f, 0.6875f);
        tesselateBlockInWorld(state, pos);
    }
    else
    {
        setShape(0.25f, 0.0f, 0.25f, 0.75f, 1.0f, 0.75f);
        tesselateBlockInWorld(state, pos);
        if (east)
        {
            setShape(0.0f, 0.0f, 0.3125f, 0.25f, 0.875f, 0.6875f);
            tesselateBlockInWorld(state, pos);
        }
        if (west)
        {
            setShape(0.75f, 0.0f, 0.3125f, 1.0f, 0.875f, 0.6875f);
            tesselateBlockInWorld(state, pos);
        }
        if (south)
        {
            setShape(0.3125f, 0.0f, 0.0f, 0.6875f, 0.875f, 0.25f);
            tesselateBlockInWorld(state, pos);
        }
        if (north)
        {
            setShape(0.3125f, 0.0f, 0.75f, 0.6875f, 0.875f, 1.0f);
            tesselateBlockInWorld(state, pos);
        }
    }
    return true;
}

bool BlockRenderer::tesselateLeverInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.3125f, 0.0f, 0.3125f, 0.6875f, 0.25f, 0.6875f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    tesselateTorch(state, (float)pos.x + 0.5f, (float)pos.y + 0.25f, (float)pos.z + 0.5f, 0.0f, 0.0f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateTripwireSourceInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.25f, 0.0f, 0.25f, 0.75f, 0.625f, 0.75f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateTripwireInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0625f, 0.0f, 1.0f, 0.09375f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateBedInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.5625f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

void BlockRenderer::tesselateDiodeInWorld(const BlockState* state, const BlockPos& pos, const Direction* direction)
{
    tesselateBlockInWorld(state, pos);

    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    builder->tex2(getLightColor(state->getBlock(), mLevelSource, pos));
    builder->color(1.0f, 1.0f, 1.0f);

    TextureAtlasSprite* texture = getTexture(state, Direction::UP);
    float u0 = (float)texture->getU0(true);
    float u1 = (float)texture->getU1(true);
    float v0 = (float)texture->getV0(true);
    float v1 = (float)texture->getV1(true);

    float x0 = (float)pos.x;
    float x1 = x0 + 1.0f;
    float z0 = (float)pos.z;
    float z1 = z0 + 1.0f;
    float y = (float)pos.y + 0.125f;

    if (direction == Direction::NORTH)
    {
        builder->vertexUV(x1, y, z1, u0, v0);
        builder->vertexUV(x1, y, z0, u0, v1);
        builder->vertexUV(x0, y, z0, u1, v1);
        builder->vertexUV(x0, y, z1, u1, v0);
    }
    else if (direction == Direction::EAST)
    {
        builder->vertexUV(x0, y, z1, u0, v0);
        builder->vertexUV(x1, y, z1, u0, v1);
        builder->vertexUV(x1, y, z0, u1, v1);
        builder->vertexUV(x0, y, z0, u1, v0);
    }
    else if (direction == Direction::WEST)
    {
        builder->vertexUV(x1, y, z0, u0, v0);
        builder->vertexUV(x0, y, z0, u0, v1);
        builder->vertexUV(x0, y, z1, u1, v1);
        builder->vertexUV(x1, y, z1, u1, v0);
    }
    else
    {
        builder->vertexUV(x0, y, z0, u0, v0);
        builder->vertexUV(x0, y, z1, u0, v1);
        builder->vertexUV(x1, y, z1, u1, v1);
        builder->vertexUV(x1, y, z0, u1, v0);
    }
}

bool BlockRenderer::tesselateDiodeInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* direction = state->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
    tesselateDiodeInWorld(state, pos, direction);
    return true;
}

bool BlockRenderer::tesselateRepeaterInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* direction = state->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
    tesselateDiodeInWorld(state, pos, direction);
    float x0 = (float)pos.x + 0.5f;
    float z0 = (float)pos.z + 0.25f;
    float x1 = (float)pos.x + 0.5f;
    float z1 = (float)pos.z + 0.75f;
    if (direction == Direction::SOUTH)
    {
        z0 = (float)pos.z + 0.75f;
        z1 = (float)pos.z + 0.25f;
    }
    else if (direction == Direction::EAST)
    {
        x0 = (float)pos.x + 0.75f;
        x1 = (float)pos.x + 0.25f;
        z0 = z1 = (float)pos.z + 0.5f;
    }
    else if (direction == Direction::WEST)
    {
        x0 = (float)pos.x + 0.25f;
        x1 = (float)pos.x + 0.75f;
        z0 = z1 = (float)pos.z + 0.5f;
    }
    tesselateTorch(state, x0, (float)pos.y + 0.125f, z0, 0.0f, 0.0f);
    tesselateTorch(state, x1, (float)pos.y + 0.125f, z1, 0.0f, 0.0f);
    return true;
}

bool BlockRenderer::tesselateComparatorInWorld(const BlockState* state, const BlockPos& pos)
{
    const Direction* direction = state->getValue<const Direction*>(HorizontalDirectionalBlock::FACING_PROPERTY);
    tesselateDiodeInWorld(state, pos, direction);
    float side0x = (float)pos.x + 0.25f;
    float side1x = (float)pos.x + 0.75f;
    float sidez = (float)pos.z + 0.25f;
    float rearx = (float)pos.x + 0.5f;
    float rearz = (float)pos.z + 0.75f;
    if (direction == Direction::SOUTH)
    {
        sidez = (float)pos.z + 0.75f;
        rearz = (float)pos.z + 0.25f;
    }
    else if (direction == Direction::EAST)
    {
        side0x = side1x = (float)pos.x + 0.75f;
        sidez = (float)pos.z + 0.25f;
        rearx = (float)pos.x + 0.25f;
        rearz = (float)pos.z + 0.5f;
    }
    else if (direction == Direction::WEST)
    {
        side0x = side1x = (float)pos.x + 0.25f;
        sidez = (float)pos.z + 0.75f;
        rearx = (float)pos.x + 0.75f;
        rearz = (float)pos.z + 0.5f;
    }
    tesselateTorch(state, side0x, (float)pos.y + 0.125f, sidez, 0.0f, 0.0f);
    tesselateTorch(state, side1x, (float)pos.y + 0.125f, sidez, 0.0f, 0.0f);
    tesselateTorch(state, rearx, (float)pos.y + 0.125f, rearz, 0.0f, 0.0f);
    return true;
}

bool BlockRenderer::tesselatePistonBaseInWorld(const BlockState* state, const BlockPos& pos, bool extended, const BlockState* actualState)
{
    const BlockState* renderState = actualState == NULL ? state : actualState;
    if (extended)
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.75f, 1.0f);
    else
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    bool rendered = tesselateBlockInWorld(renderState, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

void BlockRenderer::renderPistonArmUpDown(float x, float y, float z, float x0, float y0, float z0, float x1, float z1)
{
    TextureAtlasSprite* texture = PistonBaseBlock::getTexture(L"piston_side");
    if (hasFixedTexture())
        texture = mFixedTexture;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU(z1);
    float v1 = (float)texture->getV(0.25);
    texture->adjustUVs(u0, u1, v0, v1);
    builder->color(x1, x1, x1);
    builder->vertexUV(x, x0, y0, u1, v0);
    builder->vertexUV(x, z, y0, u0, v0);
    builder->vertexUV(y, z, z0, u0, v1);
    builder->vertexUV(y, x0, z0, u1, v1);
}

void BlockRenderer::renderPistonArmNorthSouth(float x, float y, float z, float x0, float y0, float z0, float x1, float z1)
{
    TextureAtlasSprite* texture = PistonBaseBlock::getTexture(L"piston_side");
    if (hasFixedTexture())
        texture = mFixedTexture;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU(z1);
    float v1 = (float)texture->getV(0.25);
    texture->adjustUVs(u0, u1, v0, v1);
    builder->color(x1, x1, x1);
    builder->vertexUV(x, z, z0, u1, v0);
    builder->vertexUV(x, z, y0, u0, v0);
    builder->vertexUV(y, x0, y0, u0, v1);
    builder->vertexUV(y, x0, z0, u1, v1);
}

void BlockRenderer::renderPistonArmEastWest(float x, float y, float z, float x0, float y0, float z0, float x1, float z1)
{
    TextureAtlasSprite* texture = PistonBaseBlock::getTexture(L"piston_side");
    if (hasFixedTexture())
        texture = mFixedTexture;
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    float u0 = (float)texture->getU0(false);
    float v0 = (float)texture->getV0(false);
    float u1 = (float)texture->getU(z1);
    float v1 = (float)texture->getV(0.25);
    texture->adjustUVs(u0, u1, v0, v1);
    builder->color(x1, x1, x1);
    builder->vertexUV(y, z, y0, u1, v0);
    builder->vertexUV(x, z, y0, u0, v0);
    builder->vertexUV(x, x0, z0, u0, v1);
    builder->vertexUV(y, x0, z0, u1, v1);
}

bool BlockRenderer::tesselatePistonExtensionInWorld(const BlockState* state, const BlockPos& pos, bool shortHead, const BlockState* actualState)
{
    const BlockState* renderState = actualState == NULL ? state : actualState;
    float length = shortHead ? 0.5f : 0.75f;
    setShape(0.0f, length, 0.0f, 1.0f, 1.0f, 1.0f);
    bool rendered = tesselateBlockInWorld(renderState, pos, 0x3f);
    setShape(0.375f, 0.0f, 0.375f, 0.625f, length, 0.625f);
    rendered = tesselateBlockInWorld(renderState, pos, 0x3f) || rendered;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateThinFenceInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.4375f, 0.0f, 0.4375f, 0.5625f, 1.0f, 0.5625f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateVineInWorld(const BlockState* state, const BlockPos& pos)
{
    tesselateCrossTexture(state, (float)pos.x, (float)pos.y, (float)pos.z, 1.0f);
    return true;
}

bool BlockRenderer::tesselateFenceGateInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(state->getShape(mLevelSource, pos));
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateCauldronInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.3125f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.125f, 0.3125f, 0.125f, 0.875f, 1.0f, 0.875f);
    rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateFlowerPotInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.3125f, 0.0f, 0.3125f, 0.6875f, 0.375f, 0.6875f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateAnvilInWorld(const BlockState* state, const BlockPos& pos)
{
    return tesselateAnvilInWorld(state, pos, false);
}

bool BlockRenderer::tesselateBrewingStandInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.125f, 0.0f, 0.125f, 0.875f, 0.125f, 0.875f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.4375f, 0.125f, 0.4375f, 0.5625f, 0.875f, 0.5625f);
    rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateAirPortalFrameInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 0.8125f, 1.0f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.3125f, 0.8125f, 0.3125f, 0.6875f, 1.0f, 0.6875f);
    rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateCocoaInWorld(const BlockState* state, const BlockPos& pos)
{
    setShape(0.25f, 0.25f, 0.25f, 0.75f, 0.75f, 0.75f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateBeaconInWorld(const BlockState* state, const BlockPos& pos)
{
    if (!hasFixedTexture())
    {
        setFixedTexture(getTexture(Blocks::glass, Direction::UP));
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(state, pos);
        mFlipTexture = true;
        setFixedTexture(getTexture(Blocks::obsidian, Direction::UP));
        setShape(0.125f, 0.00625f, 0.125f, 0.875f, 0.1875f, 0.875f);
        tesselateBlockInWorld(state, pos);
        setFixedTexture(getTexture(Blocks::beacon, Direction::UP));
        setShape(0.1875f, 0.1875f, 0.1875f, 0.8125f, 0.875f, 0.8125f);
        tesselateBlockInWorld(state, pos);
        mFlipTexture = false;
        clearFixedTexture();
    }
    else
    {
        setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
        tesselateBlockInWorld(state, pos);
    }
    return true;
}

bool BlockRenderer::tesselateHopperInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    int lightColor = block->getLightColor(state, mLevelSource, pos, -1);
    builder->tex2(lightColor);
    int color = mBlockColors->getColor(state, mLevelSource, &pos, 0);
    float green = (float)(color >> 8 & 255) / 255.0f;
    float red = (float)(color >> 16 & 255) / 255.0f;
    float blue = (float)(color & 255) / 255.0f;
    builder->color(red, green, blue);
    return tesselateHopperInWorld(state, pos, false);
}

bool BlockRenderer::tesselateChorusFlowerInWorld(const BlockState* state, const BlockPos& pos)
{
    ChorusFlowerBlock* flower = static_cast<ChorusFlowerBlock*>(state->getBlock());

    mRenderAllFaces = true;

    flower->setCulledFace(0);
    setShape(ChorusFlowerBlock::getDownShape());
    bool renderedDown = tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getUpShape());
    flower->setCulledFace(Direction::UP);
    bool renderedUp = tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getNorthShape());
    flower->setCulledFace(Direction::NORTH);
    bool renderedNorth = tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getSouthShape());
    flower->setCulledFace(Direction::SOUTH);
    bool renderedSouth = tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getEastShape());
    flower->setCulledFace(Direction::EAST);
    bool renderedEast = tesselateBlockInWorld(state, pos);

    setShape(ChorusFlowerBlock::getWestShape());
    flower->setCulledFace(Direction::WEST);
    bool renderedWest = tesselateBlockInWorld(state, pos);

    flower->setCulledFace(0);
    mRenderAllFaces = false;

    return renderedDown | renderedUp | renderedNorth | renderedSouth | renderedEast | renderedWest;
}

bool BlockRenderer::tesselateChorusPlantInWorld(const BlockState* state, const BlockPos& pos)
{
    ChorusPlantBlock* plant = static_cast<ChorusPlantBlock*>(state->getBlock());
    const BlockState* actualState = plant->fillVirtualBlockStateProperties(state, mLevelSource, pos);

    long long levelSeed = mMinecraft->mLevel->getSeed();
    long long mixedSeed = levelSeed + ((static_cast<long long>(pos.x) * pos.y + pos.y) * pos.z + pos.z);
    Random random(mixedSeed);
    int variantGate = random.nextInt(5);

    setShape(plant->getVerticalShape(actualState, mLevelSource, pos));
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    rendered = true;

    const BlockState* northNeighbor = mLevelSource->getBlockState(pos.north());
    if (northNeighbor->getBlock() == plant || northNeighbor->getBlock() == Blocks::AIR)
    {
        setShape(plant->getHorizontalShape(actualState, Direction::NORTH));
        rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    }

    const BlockState* southNeighbor = mLevelSource->getBlockState(pos.south());
    if (southNeighbor->getBlock() == plant || southNeighbor->getBlock() == Blocks::AIR)
    {
        setShape(plant->getHorizontalShape(actualState, Direction::SOUTH));
        rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    }

    const BlockState* westNeighbor = mLevelSource->getBlockState(pos.west());
    if (westNeighbor->getBlock() == plant || westNeighbor->getBlock() == Blocks::AIR)
    {
        setShape(plant->getHorizontalShape(actualState, Direction::WEST));
        rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    }

    const BlockState* eastNeighbor = mLevelSource->getBlockState(pos.east());
    if (eastNeighbor->getBlock() == plant || eastNeighbor->getBlock() == Blocks::AIR)
    {
        setShape(plant->getHorizontalShape(actualState, Direction::EAST));
        rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
    }

    if (variantGate > 1)
    {
        int northNodule = random.nextInt(3);
        int southNodule = random.nextInt(3);
        int westNodule = random.nextInt(3);
        int eastNodule = random.nextInt(3);

        if (northNeighbor->getBlock() != plant && northNeighbor->getBlock() != Blocks::AIR)
        {
            setShape(plant->getRandomNoduleShape(northNodule == 1, actualState, Direction::NORTH));
            rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
        }

        if (southNeighbor->getBlock() != plant && southNeighbor->getBlock() != Blocks::AIR)
        {
            setShape(plant->getRandomNoduleShape(southNodule == 1, actualState, Direction::SOUTH));
            rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
        }

        if (westNeighbor->getBlock() != plant && westNeighbor->getBlock() != Blocks::AIR)
        {
            setShape(plant->getRandomNoduleShape(westNodule == 1, actualState, Direction::WEST));
            rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
        }

        if (eastNeighbor->getBlock() != plant && eastNeighbor->getBlock() != Blocks::AIR)
        {
            setShape(plant->getRandomNoduleShape(eastNodule == 1, actualState, Direction::EAST));
            rendered = tesselateBlockInWorld(state, pos, 0x3f) || rendered;
        }
    }

    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tesselateObserverInWorld(const BlockState* state, const BlockPos& pos)
{
    int facing = state->getValue<const Direction*>(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
    if (facing < 3)
    {
        if (facing == 0)
        {
            mFaceRotation4 = 3;
            mFaceRotation5 = 3;
            tesselateBlockInWorld(state, pos);
        }
        else if (facing == 1)
        {
            mFaceTransform1 = 1;
            mFaceRotation4 = 3;
            mFaceTransform0 = 1;
            tesselateBlockInWorld(state, pos);
        }
        else
        {
            mFaceRotation5 = 3;
            mFaceRotation4 = 3;
            tesselateBlockInWorld(state, pos);
        }
    }
    else if (facing < 4)
    {
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
        tesselateBlockInWorld(state, pos);
    }
    else if (facing == 4)
    {
        mFaceRotation4 = 1;
        mFaceRotation5 = 2;
        tesselateBlockInWorld(state, pos);
    }
    else if (facing == 5)
    {
        mFaceRotation4 = 2;
        mFaceRotation5 = 1;
        tesselateBlockInWorld(state, pos);
    }
    else
    {
        tesselateBlockInWorld(state, pos);
    }
    mFaceTransform2 = 0;
    mFaceTransform4 = 0;
    mFaceRotation2 = 0;
    mFaceTransform1 = 0;
    mFaceRotation3 = 0;
    mFaceTransform0 = 0;
    mFaceRotation4 = 0;
    mFaceRotation5 = 0;
    mFaceRotation0 = 0;
    mFaceTransform3 = 0;
    mFaceTransform5 = 0;
    mFaceRotation1 = 0;
    return true;
}

void BlockRenderer::tesselateTurtleEgg(const BlockState* state, const BlockPos& pos, float red, float green, float blue)
{
    Tesselator::getInstance()->getBuilder()->color(red, green, blue);
    setShape(0.1875f, 0.0f, 0.1875f, 0.8125f, 0.75f, 0.8125f);
    tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
}

bool BlockRenderer::tessellateSeagrassInWorld(const BlockState* state, const BlockPos& pos)
{
    BufferBuilder* builder = Tesselator::getInstance()->getBuilder();
    Block* block = state->getBlock();
    builder->tex2(getLightColor(block, mLevelSource, pos));
    builder->color(0.5f, 0.5f, 0.5f);
    TextureAtlasSprite* texture = getTexture(state, Direction::DOWN);

    float x = (float)pos.x;
    float y = (float)pos.y - 0.0625f;
    float z = (float)pos.z;

    Block* belowBlock = mLevelSource->getBlock(pos.below());
    BlockPos seedPos = Block::getId(belowBlock) == 0x10e ? pos.below() : pos;
    Random random(seedPos.getRandomSeed());
    bool swap = random.nextBoolean();

    float u0 = (float)texture->getU0(true);
    float u1 = (float)texture->getU1(true);
    float v0 = (float)texture->getV0(true);
    float v1 = (float)texture->getV1(true);

    float uA = swap ? u1 : u0;
    float uB = swap ? u0 : u1;

    float x0 = x + 0.25f;
    float x1 = x + 0.75f;
    float z0 = z + 0.25f;
    float z1 = z + 0.75f;
    float yTop = y + 1.0f;
    float zFull1 = z + 1.0f;
    float xFull1 = x + 1.0f;

    builder->vertexUV(x0, yTop, z, uA, v0);
    builder->vertexUV(x0, y, z, uA, v1);
    builder->vertexUV(x0, y, zFull1, uB, v1);
    builder->vertexUV(x0, yTop, zFull1, uB, v0);
    builder->vertexUV(x1, yTop, zFull1, uA, v0);
    builder->vertexUV(x1, y, zFull1, uA, v1);
    builder->vertexUV(x1, y, z, uB, v1);
    builder->vertexUV(x1, yTop, z, uB, v0);

    builder->vertexUV(x, yTop, z0, uA, v0);
    builder->vertexUV(x, y, z0, uA, v1);
    builder->vertexUV(xFull1, y, z0, uB, v1);
    builder->vertexUV(xFull1, yTop, z0, uB, v0);
    builder->vertexUV(xFull1, yTop, z1, uA, v0);
    builder->vertexUV(xFull1, y, z1, uA, v1);
    builder->vertexUV(x, y, z1, uB, v1);
    builder->vertexUV(x, yTop, z1, uB, v0);

    return true;
}

bool BlockRenderer::tessellateIndividualSeaPickleInWorld(const BlockState* state, const BlockPos& pos, double height, const Vec3* offset)
{
    float x = (float)offset->x;
    float z = (float)offset->z;
    setShape(x - 0.125f, 0.0f, z - 0.125f, x + 0.125f, (float)height, z + 0.125f);
    bool rendered = tesselateBlockInWorld(state, pos, 0x3f);
    setShape(0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f);
    return rendered;
}

bool BlockRenderer::tessellateSeaPickleInWorld(const BlockState* state, const BlockPos& pos)
{
    extern AbstractProperty<int>* SeaPickle_PICKLES;
    int index = state->getValue(SeaPickle_PICKLES) - 1;
    if (index == 0)
    {
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.5, 0.0, 0.5));
    }
    else if (index == 1)
    {
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.35, 0.0, 0.35));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.245, Vec3::newTemp(0.65, 0.0, 0.65));
    }
    else if (index < 3)
    {
        tessellateIndividualSeaPickleInWorld(state, pos, 0.245, Vec3::newTemp(0.5, 0.0, 0.7));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.435, Vec3::newTemp(0.3, 0.0, 0.3));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.6, 0.0, 0.35));
    }
    else if (index == 3)
    {
        tessellateIndividualSeaPickleInWorld(state, pos, 0.375, Vec3::newTemp(0.29, 0.0, 0.31));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.245, Vec3::newTemp(0.71, 0.0, 0.76));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.305, Vec3::newTemp(0.69, 0.0, 0.27));
        tessellateIndividualSeaPickleInWorld(state, pos, 0.435, Vec3::newTemp(0.25, 0.0, 0.67));
    }
    return true;
}

bool BlockRenderer::tesselateCommandBlockInWorld(const BlockState* state, const BlockPos& pos)
{
    unsigned int direction = state->getValue(CommandBlock::FACING)->get3DDataValue();
    bool rendered;

    switch (direction)
    {
    case Direction::eDirection_Down:
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_Up:
        mFaceRotation4 = 3;
        mFaceTransform0 = 1;
        mFaceTransform1 = 1;
        mFaceTransform2 = 1;
        mFaceTransform3 = 1;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_North:
        mFaceRotation2 = 2;
        mFaceRotation3 = 1;
        mFaceRotation4 = 3;
        mFaceRotation5 = 3;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_South:
        mFaceRotation2 = 1;
        mFaceRotation3 = 2;
        mFaceRotation4 = 0;
        mFaceRotation5 = 0;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_West:
        mFaceRotation0 = 2;
        mFaceRotation1 = 1;
        mFaceRotation4 = 1;
        mFaceRotation5 = 2;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    case Direction::eDirection_East:
        mFaceRotation0 = 5;
        mFaceRotation1 = 2;
        mFaceRotation4 = 2;
        mFaceRotation5 = 1;
        rendered = tesselateBlockInWorld(state, pos);
        break;
    default:
        rendered = tesselateBlockInWorld(state, pos);
        break;
    }

    for (int i = 0; i < 6; ++i)
    {
        (&mFaceRotation0)[i] = 0;
        (&mFaceTransform0)[i] = 0;
    }
    return rendered;
}
