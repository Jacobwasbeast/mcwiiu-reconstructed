#include "game/core/Scale.hpp"

Scale::~Scale()
{
}

double Scale::getValue(double x, double y)
{
    return mSource->getValue(x * mScaleX, y * mScaleY);
}
