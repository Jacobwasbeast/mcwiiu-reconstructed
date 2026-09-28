#include "game/world/block/MushroomBlockLayerTheme.hpp"

#include "game/util/Random.hpp"
#include "game/core/Direction.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/RandomPatchesLayerTheme.hpp"

MushroomBlockLayerTheme::MushroomBlockLayerTheme(HugeMushroomBlock* block)
    : mBlock(block)
{
}

static int neighbourSurface(BlockTransaction* transaction, int x, int z, const Direction* direction)
{
    int neighbourX = x + direction->getStepX();
    int neighbourZ = z + direction->getStepZ();
    if (neighbourX < 0 || neighbourX >= transaction->mSize.x || neighbourZ < 0 || neighbourZ >= transaction->mSize.z)
        return -1;
    return RandomPatchesLayerTheme::HighestSolidBlock(transaction, neighbourX, neighbourZ);
}

void MushroomBlockLayerTheme::Apply(Random&, BlockTransaction* transaction, int&, int&) const
{
    for (int x = 0; x < transaction->mSize.x; ++x)
    {
        for (int z = 0; z < transaction->mSize.z; ++z)
        {
            int surfaceY = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z);
            if (surfaceY == -1)
                continue;

            int north = neighbourSurface(transaction, x, z, Direction::NORTH);
            int east = neighbourSurface(transaction, x, z, Direction::EAST);
            int south = neighbourSurface(transaction, x, z, Direction::SOUTH);
            int west = neighbourSurface(transaction, x, z, Direction::WEST);

            for (int y = 0; y < transaction->mSize.y; ++y)
            {
                BlockPos position(x, y, z);
                if (transaction->getBlock(position) <= 0)
                    continue;

                int exposed = (north < surfaceY ? 1 : 0) | (east < surfaceY ? 2 : 0) |
                              (south < surfaceY ? 4 : 0) | (west < surfaceY ? 8 : 0);
                HugeMushroomBlock::Variant* variant;
                switch (exposed)
                {
                case 0:
                    variant = y < surfaceY ? HugeMushroomBlock::Variant::ALL_INSIDE : HugeMushroomBlock::Variant::CENTER;
                    break;
                case 1: variant = HugeMushroomBlock::Variant::NORTH; break;
                case 2: variant = HugeMushroomBlock::Variant::EAST; break;
                case 3: variant = HugeMushroomBlock::Variant::NORTH_EAST; break;
                case 4: variant = HugeMushroomBlock::Variant::SOUTH; break;
                case 6: variant = HugeMushroomBlock::Variant::SOUTH_EAST; break;
                case 8: variant = HugeMushroomBlock::Variant::WEST; break;
                case 9: variant = HugeMushroomBlock::Variant::NORTH_WEST; break;
                case 12: variant = HugeMushroomBlock::Variant::SOUTH_WEST; break;
                default: variant = HugeMushroomBlock::Variant::ALL_OUTSIDE; break;
                }

                TypedBoxed<HugeMushroomBlock::Variant*> boxed(variant);
                transaction->setBlock(position, mBlock->defaultBlockState()->setBoxedValue(HugeMushroomBlock::VARIANT, &boxed));
            }
        }
    }
}
