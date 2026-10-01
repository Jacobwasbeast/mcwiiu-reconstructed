#include "game/core/FlippedIcon.hpp"

FlippedIcon::FlippedIcon(TextureAtlasSprite* sprite, bool flipU, bool flipV)
    : TextureAtlasSprite(L"", L"", 0.0f, 0.0f, 0.0f, 0.0f, 256.0f, 256.0f),
      mSprite(sprite), mFlipU(flipU), mFlipV(flipV)
{
}

unsigned int FlippedIcon::getX() const
{
    return mSprite->getX();
}

unsigned int FlippedIcon::getY() const
{
    return mSprite->getY();
}

unsigned int FlippedIcon::getWidth() const
{
    return mSprite->getWidth();
}

unsigned int FlippedIcon::getHeight() const
{
    return mSprite->getHeight();
}

float FlippedIcon::getU0(bool) const
{
    return mFlipU ? mSprite->getU1(false) : mSprite->getU0(false);
}

float FlippedIcon::getU1(bool) const
{
    return mFlipU ? mSprite->getU0(false) : mSprite->getU1(false);
}

float FlippedIcon::getU(double value) const
{
    const double width = getU1(false) - getU0(false);
    return static_cast<float>(static_cast<float>(width) * (static_cast<float>(value) * 0.0625f) + static_cast<float>(getU0(false)));
}

float FlippedIcon::getV0(bool) const
{
    return mSprite->getV0(false);
}

float FlippedIcon::getV1(bool) const
{
    return mFlipV ? mSprite->getV0(false) : mSprite->getV1(false);
}

float FlippedIcon::getV(double value) const
{
    const double height = getV1(false) - getV0(false);
    return static_cast<float>(static_cast<float>(height) * (static_cast<float>(value) * 0.0625f) + static_cast<float>(getV0(false)));
}

std::wstring FlippedIcon::getName() const
{
    return mSprite->getName();
}

unsigned int FlippedIcon::getSourceWidth() const
{
    return mSprite->getSourceWidth();
}

unsigned int FlippedIcon::getSourceHeight() const
{
    return mSprite->getSourceHeight();
}

unsigned int FlippedIcon::getFlags() const
{
    return mSprite->getFlags();
}

void FlippedIcon::setFlags(int flags)
{
    mSprite->setFlags(flags);
}

