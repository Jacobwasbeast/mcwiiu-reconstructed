#include "game/core/UniformSize.hpp"

#include "game/util/Random.hpp"

UniformSize::UniformSize(unsigned int size)
    : mSize(size)
{
}

int UniformSize::GetLayerSize(Random& random, double blend) const
{
    return mSize;
}
