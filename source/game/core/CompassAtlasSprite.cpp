#include "game/core/CompassAtlasSprite.hpp"

#include <cmath>

#include "game/client/render/Texture.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Math.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

CompassAtlasSprite::CompassAtlasSprite()
    : TextureAtlasSprite(L"compass", L"compass", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mPlayerIndex(255), mParent(0), mAngle(0.0), mAngleDelta(0.0)
{
}

CompassAtlasSprite::CompassAtlasSprite(int playerIndex, CompassAtlasSprite* parent)
    : TextureAtlasSprite(L"compass", L"compass", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mPlayerIndex(playerIndex), mParent(parent), mAngle(0.0), mAngleDelta(0.0)
{
}

void CompassAtlasSprite::updateFromPosition(Level* level, double x, double z, double yRot, bool noJitter, bool instant)
{
    const double pi = Mth::PI;
    double target = 0.0;
    if (level != NULL && !noJitter)
    {
        BlockPos spawn = level->getSharedSpawnPos();
        double facing = ((static_cast<double>(static_cast<int>(yRot) % 360) - 90.0) * pi) / 180.0;
        target = -(facing - std::atan2(static_cast<double>(spawn.z) - z, static_cast<double>(spawn.x) - x));
        if (!level->mDimension->isNaturalDimension())
        {
            double random = Math::random();
            target = random * pi + random * pi;
        }
    }

    if (instant)
    {
        mAngle = target;
    }
    else
    {
        double delta = target - mAngle;
        while (delta < -pi)
        {
            delta += pi * 2.0;
        }
        while (delta >= pi)
        {
            delta -= pi * 2.0;
        }
        mAngleDelta = (Mth::clamp(delta, -1.0, 1.0) * 0.1 + mAngleDelta) * 0.8;
        mAngle += mAngleDelta;
    }

    CompassAtlasSprite* frameSource = mParent != NULL ? mParent : this;
    unsigned int frameCount = frameSource->mFrames == NULL ? 0 : frameSource->mFrames->size();
    int frame = static_cast<unsigned int>((mAngle / (pi * 2.0) + 1.0) * static_cast<double>(frameCount)) % frameCount;
    if (frame != mFrame)
    {
        mFrame = frame;
        frameSource->mSource->blit(mX, mY, frameSource->mFrames->at(frame), mRotated);
    }
}

void CompassAtlasSprite::cycleFrames()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (mPlayerIndex < 4 && minecraft->mLevel != NULL && minecraft->mLocalPlayers[mPlayerIndex] != NULL)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[mPlayerIndex];
        updateFromPosition(player->mLevel, player->mX, player->mZ, player->mYRot, false, false);
        return;
    }
    mFrame = 1;
    updateFromPosition(NULL, 0.0, 0.0, 0.0, false, true);
}

unsigned int CompassAtlasSprite::getSourceWidth() const
{
    return const_cast<CompassAtlasSprite*>(this)->getSource()->getWidth();
}

unsigned int CompassAtlasSprite::getSourceHeight() const
{
    return const_cast<CompassAtlasSprite*>(this)->getSource()->getHeight();
}

int CompassAtlasSprite::getFrames()
{
    if (!mParent)
        return TextureAtlasSprite::getFrames();
    return mParent->getFrames();
}

void CompassAtlasSprite::freeFrameTextures()
{
    if (mParent)
        return;
    TextureAtlasSprite::freeFrameTextures();
}

unsigned int CompassAtlasSprite::hasOwnData()
{
    return mParent == 0;
}
