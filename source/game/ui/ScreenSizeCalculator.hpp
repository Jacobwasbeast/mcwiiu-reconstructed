#ifndef GAME_UI_SCREENSIZECALCULATOR_HPP
#define GAME_UI_SCREENSIZECALCULATOR_HPP

#include <stdint.h>

class Options;

class ScreenSizeCalculator
{
public:
    ScreenSizeCalculator(Options* options, int guiScale);
    ScreenSizeCalculator(Options* options, int width, int height, int guiScale);

    void _init(Options* options, int width, int height, int guiScale);
    uint32_t getWidth() const;
    uint32_t getHeight() const;

private:
    uint32_t mWidth;
    uint32_t mHeight;

public:
    double mWidthScale;
    double mHeightScale;

private:
    int mScale;
};

#endif
