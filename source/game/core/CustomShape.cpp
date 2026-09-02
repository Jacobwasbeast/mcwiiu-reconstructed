#include "game/core/CustomShape.hpp"

#include <math.h>

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"

void CustomShape::GenerateShape(Random& random, Shape& shape) const
{
    float sourceSize = (float)size;
    float xScale = sourceSize / (float)shape.width;
    float zScale = sourceSize / (float)shape.depth;

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            int sourceX = (int)floorf((float)x * xScale);
            int sourceZ = (int)floorf((float)z * zScale);

            shape.at(x, z) = values.data[size * sourceX + sourceZ];
        }
    }
}
