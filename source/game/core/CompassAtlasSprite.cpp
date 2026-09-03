#include "game/core/CompassAtlasSprite.hpp"

#include <cmath>

#include "game/client/render/Texture.hpp"
#include "game/core/Minecraft.hpp"

CompassAtlasSprite::CompassAtlasSprite()
    : TextureAtlasSprite(L"compass", L"compass", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mFrameCount(255), mParent(0), mAngle(1.0), mTargetAngle(1.0)
{
}

CompassAtlasSprite::CompassAtlasSprite(int frameCount, CompassAtlasSprite* parent)
    : TextureAtlasSprite(L"compass", L"compass", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mFrameCount(frameCount), mParent(parent), mAngle(1.0), mTargetAngle(1.0)
{
}

CompassAtlasSprite::~CompassAtlasSprite()
{
}

void CompassAtlasSprite::updateFromPosition(Level* level, double x, double y, double z,
                                             bool hasLevel, bool force)
{
    double target = 0.0;
    if (hasLevel)
        target = std::atan2(z, x);
    if (!force)
    {
        double delta = target - mAngle;
        while (delta < -3.1415927410125732)
            delta += 6.2831854820251465;
        while (delta >= 3.1415927410125732)
            delta -= 6.2831854820251465;
        if (delta < -1.0)
            delta = -1.0;
        if (delta > 1.0)
            delta = 1.0;
        mTargetAngle = (mAngle + delta * 0.2);
        mAngle = mTargetAngle;
    }
    else
        mAngle = target;

    Texture* destination = getSource();
    if (!destination)
        return;
    int count = getFrameCount();
    if (count <= 0)
        return;
    int frame = (int)((mAngle / 6.2831854820251465 + 1.0) * (double)count);
    frame %= count;
    if (frame < 0)
        frame += count;
    Texture* image = getFrame(frame);
    if (image)
        destination->blit((int)getX(), (int)getY(), image, false);
}

void CompassAtlasSprite::cycleFrames()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (mFrameCount < 4 && minecraft)
    {
        updateFromPosition(0, 0.0, 0.0, 0.0, false, false);
        return;
    }
    updateFromPosition(0, 0.0, 0.0, 0.0, false, true);
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

bool CompassAtlasSprite::hasOwnData()
{
    return mParent == 0;
}
