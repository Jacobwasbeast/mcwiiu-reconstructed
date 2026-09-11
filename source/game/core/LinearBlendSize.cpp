#include "game/core/LinearBlendSize.hpp"

#include "game/util/Random.hpp"

LinearBlendSize::LinearBlendSize(unsigned int minSize, unsigned int maxSize)
{
    LinearBlendSize* obj = this;
    if (obj != NULL || (obj = static_cast<LinearBlendSize*>(operator new(0xc)), obj != NULL))
    {
        obj->mMaxSize = maxSize;
        obj->mMinSize = minSize;
    }
}

int LinearBlendSize::GetLayerSize(Random& random, double blend) const
{
    return mMinSize + (int)(blend * (mMaxSize - mMinSize));
}
