#include "game/core/PatchyShape.hpp"

#include "game/core/Shape.hpp"
#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"

PatchyShape::PatchyShape(double scale)
    : scale(scale)
{
}

void PatchyShape::GenerateShape(Random& random, Shape& shape) const
{
    SimplexNoise noise(&random);

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            shape.at(x, z) = (float)(noise.getValue((double)x / this->scale, (double)z / this->scale) + 1.0);
        }
    }
}
