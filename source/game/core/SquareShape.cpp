#include "game/core/SquareShape.hpp"

#include <math.h>

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"

void SquareShape::GenerateShape(Random& random, Shape& shape) const
{
    float centreX = (float)shape.width * 0.5f;
    float centreZ = (float)shape.depth * 0.5f;

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            float edgeX = fabsf(((float)x - centreX) / centreX) / 0.7f;
            float edgeZ = fabsf(((float)z - centreZ) / centreZ) / 0.7f;

            shape.at(x, z) = edgeX > edgeZ ? edgeX : edgeZ;
        }
    }
}
