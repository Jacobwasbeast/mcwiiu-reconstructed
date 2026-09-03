#include "game/world/generation/FunctionLayerFill.hpp"

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"

FunctionLayerFill::FunctionLayerFill(int (*function)(float))
    : mFunction(function)
{
}

int FunctionLayerFill::UNIFORM(float)
{
    return 1;
}

int FunctionLayerFill::ISLAND(float value)
{
    if (value < 0.1f)
        return 3;
    if (value >= 0.5f)
        return 1;
    return 2;
}

void FunctionLayerFill::ExtrudeLayer(Random& random, const Shape& shape, const Vec3i&,
                                     BlockTransaction* transaction, int* minY, int* maxY) const
{
    for (unsigned int x = 0; x < shape.width; ++x)
    {
        for (unsigned int z = 0; z < shape.depth; ++z)
        {
            float value = shape.at(x, z);
            if (value >= 0.0f && value < 1.0f)
            {
                int count = random.nextInt(mFunction(value));
                int y = transaction->mSize.y - 3;
                for (int layer = 0; layer < count; ++layer, --y)
                {
                    if (*maxY < y)
                        *maxY = y;
                    if (*minY > y)
                        *minY = y;
                    transaction->setBlock(Vec3i((int)x, y, (int)z), 1, 0);
                }
            }
        }
    }
}
