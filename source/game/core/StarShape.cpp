#include "game/core/StarShape.hpp"

#include "game/core/Mth.hpp"
#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"

StarShape::StarShape(unsigned int points)
    : points(points)
{
}

void StarShape::GenerateShape(Random& random, Shape& shape) const
{
    float radius = 0.2375f * (float)(shape.width + shape.depth);
    float innerRadius = 0.8f * radius;

    Vec3i centre((int)(shape.width / 2), 0, (int)(shape.depth / 2));

    for (unsigned int x = 0; x < shape.width; x++)
    {
        for (unsigned int z = 0; z < shape.depth; z++)
        {
            Vec3i pos((int)x, 0, (int)z);
            Vec3i offset = pos.subtract(centre);

            float angle = Mth::atan2(offset.z, offset.x);
            float arm = Mth::cos(angle * (float)this->points);

            shape.at(x, z) = (float)pos.dist(centre) / ((radius - innerRadius) * arm + innerRadius);
        }
    }
}
