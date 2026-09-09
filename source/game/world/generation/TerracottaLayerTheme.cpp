#include "game/world/generation/TerracottaLayerTheme.hpp"

#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

Block* getColour(unsigned char colour);

TerracottaLayerTheme::TerracottaLayerTheme(const TerracottaLayerThemePattern& pattern,
                                           TerracottaLayerThemeFunction function)
    : mSize(pattern.size), mFunction(function)
{
    for (int i = 0; i < 16; ++i)
        mValues[i] = pattern.values[i];
}

unsigned int TerracottaLayerTheme::CONCENTRIC_RINGS(int x, int z)
{
    int value = (x < 0 ? -x : x) + (z < 0 ? -z : z);
    return (unsigned int)(value % 16) & 255u;
}

unsigned int TerracottaLayerTheme::OFFSET_RINGS(int x, int z)
{
    return CONCENTRIC_RINGS(x + 3, z + 4);
}

unsigned int TerracottaLayerTheme::HASHED(int x, int z)
{
    return (boost::hash_value(x) ^ boost::hash_value(z)) & 15u;
}

unsigned char TerracottaLayerTheme::TARTAN(int x, int z)
{
    static const unsigned char values[25] = {
        7, 1, 2, 1, 7,
        1, 1, 2, 1, 1,
        2, 2, 3, 2, 2,
        1, 1, 2, 1, 1,
        7, 1, 2, 1, 7
    };
    int xIndex = (x + 1275) % 5;
    int zIndex = (z + 1275) % 5;
    return values[xIndex * 5 + zIndex];
}

unsigned int TerracottaLayerTheme::STRIPES(int x, int z)
{
    return (unsigned int)(x % 16) & 255u;
}

unsigned int TerracottaLayerTheme::UNIFORM(int x, int z)
{
    return 0;
}

void TerracottaLayerTheme::Apply(Random& random, BlockTransaction* transaction, int& width, int& height) const
{
    unsigned char randomColour = (unsigned char)random.nextInt(16);
    int widthRings = width / mSize;
    int heightRings = height / mSize;
    int x;
    int z;
    for (x = 0; x < width; ++x)
    {
        for (z = 0; z < height; ++z)
        {
            int xIndex = x % mSize;
            int zIndex = z % mSize;
            const Direction* direction = mValues[xIndex * 4 + zIndex];
            int ringX = x / mSize;
            int ringZ = z / mSize;
            if (ringX < 0 && (x / mSize) % 2 != 0)
            {
                --ringX;
            }
            if (ringZ < 0 && (z / mSize) % 2 != 0)
            {
                --ringZ;
            }
            unsigned char colour = (unsigned char)(randomColour + mFunction(ringX - widthRings / 2,
                                                                              ringZ - heightRings / 2));
            Block* block = getColour(colour);
            TypedBoxed<const Direction*> directionValue(direction);
            const BlockState* state = block->defaultBlockState()->setBoxedValue(
                HorizontalDirectionalBlock::FACING_PROPERTY, &directionValue);
            Vec3i position(x, 0, z);
            int y;
            for (y = 0; y < transaction->mSize.y; ++y)
            {
                position.y = y;
                if (transaction->getBlock(position) != 0)
                {
                    transaction->setBlock(position, state);
                }
            }
        }
    }
}
