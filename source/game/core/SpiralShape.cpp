#include "game/core/SpiralShape.hpp"

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"

SpiralShape::SpiralShape(int minRadius, int maxRadius, double angle)
    : minRadius(minRadius)
    , maxRadius(maxRadius)
    , angle(angle)
{
}

void SpiralShape::GenerateShape(Random& random, Shape& shape) const
{
    random.nextInt(this->maxRadius - this->minRadius);
}
