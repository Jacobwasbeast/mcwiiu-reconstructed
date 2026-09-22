#ifndef GAME_UI_FUIRECT_HPP
#define GAME_UI_FUIRECT_HPP

#include <utility>

struct fuiRect
{
    float xMin;
    float xMax;
    float yMin;
    float yMax;

    void EndianSwap();
    operator std::pair<float, float>() const { return std::pair<float, float>(xMin, xMax); }
};

#endif
