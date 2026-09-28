#include "game/core/ClockAtlasSprite.hpp"
#include "game/client/render/Texture.hpp"
#include "game/util/Math.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/generation/MultiPlayerLevel.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"

ClockAtlasSprite::ClockAtlasSprite()
    : TextureAtlasSprite(L"clock", L"clock", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mLastTime(0.0), mRotation(0.0), mPlayerIndex(255), mParent(0)
{
}

ClockAtlasSprite::ClockAtlasSprite(int playerIndex, ClockAtlasSprite* parent)
    : TextureAtlasSprite(L"clock", L"clock", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mLastTime(0.0), mRotation(0.0), mPlayerIndex(playerIndex), mParent(parent)
{
}

ClockAtlasSprite::~ClockAtlasSprite()
{
}

void ClockAtlasSprite::cycleFrames()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    double time = 0.0;
    if (mPlayerIndex < 4 && minecraft->mLevel != NULL && minecraft->mLocalPlayers[mPlayerIndex] != NULL)
    {
        boost::shared_ptr<MultiplayerLocalPlayer> player = minecraft->mLocalPlayers[mPlayerIndex];
        time = player->mLevel->getTimeOfDay(1.0f);
        if (!player->mLevel->mDimension->isNaturalDimension())
        {
            time = Math::random();
        }
    }
    else
    {
        mFrame = 1;
    }

    double delta = time - mLastTime;
    while (delta < -0.5)
    {
        delta += 1.0;
    }
    while (delta >= 0.5)
    {
        delta -= 1.0;
    }
    delta = Mth::clamp(delta, -1.0, 1.0);
    mRotation = (delta * 0.1 + mRotation) * 0.8;
    mLastTime += mRotation;

    ClockAtlasSprite* frameSource = mParent != NULL ? mParent : this;
    unsigned int frameCount = frameSource->mFrames == NULL ? 0 : frameSource->mFrames->size();
    int frame = static_cast<unsigned int>((mLastTime + 1.0) * static_cast<double>(frameCount)) % frameCount;
    if (frame != mFrame)
    {
        mFrame = frame;
        frameSource->mSource->blit(mX, mY, frameSource->mFrames->at(frame), mRotated);
    }
}

unsigned int ClockAtlasSprite::getSourceWidth() const
{
    Texture* source = const_cast<ClockAtlasSprite*>(this)->getSource();
    return source->getWidth();
}

unsigned int ClockAtlasSprite::getSourceHeight() const
{
    Texture* source = const_cast<ClockAtlasSprite*>(this)->getSource();
    return source->getHeight();
}

int ClockAtlasSprite::getFrames()
{
    if (mParent == 0)
        return TextureAtlasSprite::getFrames();
    return mParent->getFrames();
}

void ClockAtlasSprite::freeFrameTextures()
{
    if (mParent != 0)
        return;
    TextureAtlasSprite::freeFrameTextures();
}

unsigned int ClockAtlasSprite::hasOwnData()
{
    return mParent == 0;
}
