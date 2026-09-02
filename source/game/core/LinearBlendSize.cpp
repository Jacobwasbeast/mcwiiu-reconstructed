#include "game/core/LinearBlendSize.hpp"

#include "game/util/Random.hpp"

LinearBlendSize::LinearBlendSize(unsigned int minSize, unsigned int maxSize)
    : mMinSize(minSize)
    , mMaxSize(maxSize)
{
}

int LinearBlendSize::GetLayerSize(Random& random, double blend) const
{
    return mMinSize + (int)(blend * (mMaxSize - mMinSize));
}
