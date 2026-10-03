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

void MushroomBlockLayerTheme::Apply(Random&, BlockTransaction* transaction, int&, int&) const
{
    for (int x = 0; x < transaction->mSize.x; ++x)
    {
        for (int z = 0; z < transaction->mSize.z; ++z)
        {
            int surfaceY = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z);
            if (surfaceY == -1)
                continue;

            const Direction* directions[4] = { Direction::NORTH, Direction::EAST, Direction::SOUTH, Direction::WEST };
            int neighbourSurfaces[4] = { -1, -1, -1, -1 };
            for (int i = 0; i < 4; ++i)
            {
                int neighbourX = directions[i]->getStepX() + x;
                int neighbourZ = directions[i]->getStepZ() + z;
                if (neighbourX >= 0 && neighbourX < transaction->mSize.x && neighbourZ >= 0 && neighbourZ < transaction->mSize.z)
                    neighbourSurfaces[i] = RandomPatchesLayerTheme::HighestSolidBlock(transaction, neighbourX, neighbourZ);
            }

            for (int y = 0; y < transaction->mSize.y; ++y)
            {
                BlockPos position(x, y, z);
                if (transaction->getBlock(position) <= 0)
                    continue;

                int exposed = (neighbourSurfaces[0] < surfaceY ? 1 : 0) | (neighbourSurfaces[1] < surfaceY ? 2 : 0) |
                              (neighbourSurfaces[2] < surfaceY ? 4 : 0) | (neighbourSurfaces[3] < surfaceY ? 8 : 0);
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
