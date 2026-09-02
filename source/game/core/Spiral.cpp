#include "game/core/Spiral.hpp"

bool Spiral::Step(int& x, int& y)
{
    if (mCount >= mSize * mSize)
    {
        return false;
    }

    x = mX + mOriginX;
    y = mY + mOriginY;

    if (mX == mY || (mX > 0 && mX == 1 - mY) || (mX < 0 && mX == -mY))
    {
        int stepX = mStepX;
        mStepX = -mStepY;
        mStepY = stepX;
    }

    mX += mStepX;
    mY += mStepY;
    ++mCount;

    return true;
}

bool Spiral::GetNext(int& x, int& y)
{
    return Step(x, y);
}
