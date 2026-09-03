#include "game/ui/FJ_Helpers.hpp"

#include <cstddef>

static FJ_Helpers* sHelpersInstance = NULL;

FJ_Helpers::FJ_Helpers()
{
    mRndSeed = 0;
}

FJ_Helpers* FJ_Helpers::RetInstance()
{
    if (sHelpersInstance == NULL)
    {
        sHelpersInstance = new FJ_Helpers();
    }
    return sHelpersInstance;
}

double FJ_Helpers::nextRandomNumber()
{
    mRndSeed = (mRndSeed * 9301 + 49297) % 233280;
    return (double)(float)(((double)mRndSeed) / 233280.0);
}

int FJ_Helpers::nextInt(int maxValue)
{
    if (maxValue > 0)
    {
        return (int)(nextRandomNumber() * (double)(float)maxValue);
    }
    return (int)nextRandomNumber();
}

void FJ_Helpers::SetRndSeed(int seed)
{
    mRndSeed = seed;
}
