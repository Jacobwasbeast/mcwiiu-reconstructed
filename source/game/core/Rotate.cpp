#include "game/core/Rotate.hpp"

Rotate::~Rotate()
{
}

double Rotate::getValue(double x, double y)
{
    return mSource->getValue(x * mCos + y * mSin, y * mCos - x * mSin);
}
