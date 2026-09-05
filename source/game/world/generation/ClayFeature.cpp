#include "game/world/generation/ClayFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"

ClayFeature::ClayFeature(int numberOfBlocks)
    : Feature(false), mClay(Blocks::clay), mNumberOfBlocks(numberOfBlocks)
{
}

ClayFeature::~ClayFeature()
{
}

bool ClayFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    if (level->getBlockState(origin)->getMaterial() != Material::WATER)
    {
        return false;
    }

    const int radius = random.nextInt(mNumberOfBlocks - 2) + 2;
    const int minX = origin.x - radius;
    const int maxX = origin.x + radius;
    const int minZ = origin.z - radius;
    const int maxZ = origin.z + radius;

    for (int x = minX; x <= maxX; ++x)
    {
        for (int z = minZ; z <= maxZ; ++z)
        {
            if ((x - origin.x) * (x - origin.x) + (z - origin.z) * (z - origin.z)
                <= radius * radius)
            {
                for (int y = origin.y - 1; y <= origin.y + 1; ++y)
                {
                    const BlockPos pos(x, y, z);
                    Block* block = level->getBlockState(pos)->getBlock();
                    if (block == Blocks::dirt || block == Blocks::clay)
                    {
                        level->setBlock(pos, mClay->defaultBlockState(), 2, false);
                    }
                }
            }
        }
    }

    return true;
}
