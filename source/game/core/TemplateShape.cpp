#include "game/core/TemplateShape.hpp"

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"

void TemplateShape::GenerateShape(Random& random, Shape& shape) const
{
    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            shape.at(x, z) = 0.5f;
        }
    }
}
