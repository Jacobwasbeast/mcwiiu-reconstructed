#include "game/util/RandomizeSize.hpp"

#include <math.h>

#include "game/util/Random.hpp"

RandomizeSize::RandomizeSize(LayerSize* layerSize, float amount)
    : mLayerSize(layerSize)
    , mAmount(amount)
{
}

int RandomizeSize::GetLayerSize(Random& random, double blend) const
{
    unsigned int size = mLayerSize->GetLayerSize(random, blend);
    return size + (unsigned int)floorf((2.0f * mAmount * random.nextFloat() - 1.0f) * (float)size);
}
