#include "game/world/generation/BowlLayerFill.hpp"

#include <cmath>

#include "game/core/Shape.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"

BowlLayerFill::BowlLayerFill(int offset, float (*curve)(float), int thickness)
    : mOffset(offset)
    , mCurve(curve)
    , mThickness(thickness)
{
}

float BowlLayerFill::BOWL(float value)
{
    return 1.0f - value;
}

float BowlLayerFill::DOME(float value)
{
    return value;
}

void BowlLayerFill::ExtrudeLayer(Random&, const Shape& shape, const Vec3i&,
                                 BlockTransaction* transaction, int* minY, int* maxY) const
{
    const float height = mOffset - 0.001f;
    for (unsigned int x = 0; x < shape.width; ++x)
    {
        for (unsigned int z = 0; z < shape.depth; ++z)
        {
            float value = shape.at(x, z);
            if (value >= 0.0f && value < 1.0)
            {
                int startY = (int)std::floor(height * mCurve(value));
                for (int layer = 0; layer < mThickness; ++layer)
                {
                    Vec3i position(x, transaction->mSize.y, z);
                    Vec3i offset(0, startY + 3 + layer, 0);
                    position = position.subtract(offset);

                    if (*maxY < position.y)
                        *maxY = position.y;
                    if (*minY > position.y)
                        *minY = position.y;
                    transaction->setBlock(position, 1, 0);
                }
            }
        }
    }
}
