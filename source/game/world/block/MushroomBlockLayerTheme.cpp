#include "game/world/block/MushroomBlockLayerTheme.hpp"

#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/block/BlockTransaction.hpp"
#include "game/world/block/HugeMushroomBlock.hpp"
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
            {
                continue;
            }

            int north = -1;
            int west = -1;
            int south = -1;
            int east = -1;
            if (z > 0)
            {
                north = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z - 1);
            }
            if (z + 1 < transaction->mSize.z)
            {
                south = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x, z + 1);
            }
            if (x > 0)
            {
                west = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x - 1, z);
            }
            if (x + 1 < transaction->mSize.x)
            {
                east = RandomPatchesLayerTheme::HighestSolidBlock(transaction, x + 1, z);
            }

            for (int y = 0; y < transaction->mSize.y; ++y)
            {
                Vec3i position(x, y, z);
                if (transaction->getBlock(position) <= 0)
                {
                    continue;
                }

                int variant = 0;
                int exposed = (north < surfaceY ? 1 : 0) |
                              (west < surfaceY ? 2 : 0) |
                              (south < surfaceY ? 4 : 0) |
                              (east < surfaceY ? 8 : 0);
                switch (exposed)
                {
                case 1: variant = 2; break;
                case 2: variant = 4; break;
                case 3: variant = 1; break;
                case 4: variant = 8; break;
                case 6: variant = 7; break;
                case 8: variant = 6; break;
                case 9: variant = 3; break;
                case 12: variant = 9; break;
                default: variant = 0; break;
                }
                if (exposed == 0 && y < surfaceY)
                {
                    variant = 12;
                }

                transaction->setBlock(position, mBlock->getBlockState(variant));
            }
        }
    }
}
