#include "game/world/generation/LakeFeature.hpp"

#include <vector>

#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Random.hpp"

LakeFeature::LakeFeature(Block* block)
    : Feature(false), mBlock(block)
{
}

LakeFeature::~LakeFeature()
{
}

bool LakeFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    BlockPos lakeOrigin = origin.offset(-8, 0, -8);
    while (lakeOrigin.y > 5 && level->isEmptyBlock(lakeOrigin))
    {
        lakeOrigin = lakeOrigin.below();
    }
    if (lakeOrigin.y < 5)
    {
        return false;
    }

    lakeOrigin = lakeOrigin.below(4);

    std::vector<unsigned char> mask(16 * 8 * 16, 0);
    const int ellipsoids = random.nextInt(4) + 4;
    for (int ellipsoid = 0; ellipsoid < ellipsoids; ++ellipsoid)
    {
        const double xSize = random.nextDouble() * 6.0 + 5.0;
        const double ySize = random.nextDouble() * 4.0 + 2.0;
        const double zSize = random.nextDouble() * 6.0 + 5.0;
        const double xCenter = random.nextDouble() * (14.0 - xSize) + 1.0 + xSize * 0.5;
        const double yCenter = random.nextDouble() * (2.0 - ySize) + 1.0 + ySize * 0.5;
        const double zCenter = random.nextDouble() * (14.0 - zSize) + 1.0 + zSize * 0.5;

        for (int x = 1; x < 15; ++x)
        {
            const double dx = (x - xCenter) / (xSize * 0.5);
            for (int y = 1; y < 7; ++y)
            {
                const double dy = (y - yCenter) / (ySize * 0.5);
                for (int z = 1; z < 15; ++z)
                {
                    const double dz = (z - zCenter) / (zSize * 0.5);
                    if (dx * dx + dy * dy + dz * dz < 1.0)
                    {
                        mask[(x * 8 + y) * 16 + z] = 1;
                    }
                }
            }
        }
    }

    for (int x = 0; x < 16; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            for (int z = 0; z < 16; ++z)
            {
                if (mask[(x * 8 + y) * 16 + z] == 0)
                {
                    continue;
                }
                const BlockPos pos = lakeOrigin.offset(x, y, z);
                const BlockState* state = level->getBlockState(pos);
                if (y < 4)
                {
                    if (!state->getMaterial()->isLiquid() && level->getBlock(pos) != mBlock)
                    {
                        return false;
                    }
                }
                else if (state->getMaterial()->isSolid())
                {
                    return false;
                }
            }
        }
    }

    for (int x = 0; x < 16; ++x)
    {
        for (int y = 0; y < 8; ++y)
        {
            for (int z = 0; z < 16; ++z)
            {
                if (mask[(x * 8 + y) * 16 + z] != 0)
                {
                    const BlockPos pos = lakeOrigin.offset(x, y, z);
                    Block* block = y < 4 ? mBlock : Blocks::AIR;
                    level->setBlock(pos, block->defaultBlockState(), 2, false);
                }
            }
        }
    }

    if (mBlock->defaultBlockState()->getMaterial() == Material::WATER)
    {
        for (int x = 0; x < 16; ++x)
        {
            for (int z = 0; z < 16; ++z)
            {
                const BlockPos pos = lakeOrigin.offset(x, 4, z);
                if (mask[(x * 8 + 4) * 16 + z] != 0 && level->shouldFreezeIgnoreNeighbors(pos))
                {
                    level->setBlock(pos, Blocks::ice->defaultBlockState(), 2, false);
                }
            }
        }
    }

    return true;
}
