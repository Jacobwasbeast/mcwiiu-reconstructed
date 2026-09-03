#include "game/core/ClockAtlasSprite.hpp"
#include "game/client/render/Texture.hpp"
#include "game/util/Math.hpp"

static double clampClockValue(double value, double minimum, double maximum)
{
    if (value < minimum)
        return minimum;
    if (value > maximum)
        return maximum;
    return value;
}

ClockAtlasSprite::ClockAtlasSprite()
    : TextureAtlasSprite(L"clock", L"clock", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mLastTime(1.0), mRotation(1.0), mDimension(255), mParent(0)
{
}

ClockAtlasSprite::ClockAtlasSprite(int frame, ClockAtlasSprite* parent)
    : TextureAtlasSprite(L"clock", L"clock", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mLastTime(1.0), mRotation(1.0), mDimension(frame), mParent(parent)
{
}

ClockAtlasSprite::~ClockAtlasSprite()
{
}

void ClockAtlasSprite::cycleFrames()
{
    double time = Math::random() - mLastTime;
    while (time < -0.5)
        time += 1.0;
    while (time >= 0.5)
        time -= 1.0;
    time = clampClockValue(time, -1.0, 1.0);
    mRotation = (time * 0.1 + mRotation) * 0.8;
    mLastTime += mRotation;

    ClockAtlasSprite* frameSource = mParent != 0 ? mParent : this;
    int frameCount = frameSource->getFrames();
    if (frameCount <= 0)
        return;

    double frameValue = (mLastTime + 1.0) * (double)frameCount;
    int frame = (int)frameValue % frameCount;
    if (frame < 0)
        frame += frameCount;
    Texture* source = getSource();
    Texture* texture = frameSource->getFrame(frame);
    if (source != 0 && texture != 0)
        source->blit((int)getX(), (int)getY(), texture, false);
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
