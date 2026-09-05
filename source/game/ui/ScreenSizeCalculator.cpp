#define private public
#include "game/core/Options.hpp"
#undef private

#include <cmath>

#include "game/core/Minecraft.hpp"
#include "game/ui/ScreenSizeCalculator.hpp"

ScreenSizeCalculator::ScreenSizeCalculator(Options* options, int guiScale)
{
    Minecraft* minecraft = Minecraft::GetInstance();
    _init(options, minecraft->mWidth, minecraft->mHeight, guiScale);
}

ScreenSizeCalculator::ScreenSizeCalculator(Options* options, int width, int height, int guiScale)
{
    _init(options, width, height, guiScale);
}

void ScreenSizeCalculator::_init(Options* options, int width, int height, int guiScale)
{
    mWidth = width;
    mHeight = height;

    if (guiScale == -1)
    {
        int maxScale = options->mGuiScale;
        mScale = 1;
        if (maxScale == 0)
        {
            maxScale = 1000;
        }

        do
        {
            if (maxScale <= mScale)
            {
                break;
            }
            ++mScale;
        }
        while (width / mScale >= 320 && height / mScale >= 240);
    }
    else
    {
        mScale = guiScale;
    }

    mWidthScale = (double)width / (double)mScale;
    mHeightScale = (double)height / (double)mScale;
    mWidth = (uint32_t)std::ceil(mWidthScale);
    mHeight = (uint32_t)std::ceil(mHeightScale);
}

uint32_t ScreenSizeCalculator::getWidth() const
{
    return mWidth;
}

uint32_t ScreenSizeCalculator::getHeight() const
{
    return mHeight;
}
