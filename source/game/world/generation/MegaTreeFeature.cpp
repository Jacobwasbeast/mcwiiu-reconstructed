#include "game/world/generation/MegaTreeFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

MegaTreeFeature::MegaTreeFeature(bool doBlockNotify, int treeType, int height,
                                 const BlockState* trunkState,
                                 const BlockState* leafState)
    : AbstractTreeFeature(doBlockNotify, treeType, trunkState, leafState, false),
      mHeight(height)
{
}

MegaTreeFeature::~MegaTreeFeature()
{
}

int MegaTreeFeature::calcTreeHeigth(Random& random)
{
    int height = random.nextInt(3) + mTreeType;
    if (mHeight > 1)
    {
        height += random.nextInt();
    }
    return height;
}

bool MegaTreeFeature::checkIsFree(Level* level, const BlockPos& pos, int height)
{
    if (pos.y < 1 || pos.y + height + 1 > 256)
    {
        return false;
    }

    for (int y = 0; y <= height + 1; ++y)
    {
        int radius = y < 2 ? 1 : 2;
        for (int x = -radius; x <= radius; ++x)
        {
            for (int z = -radius; z <= radius; ++z)
            {
                BlockPos checkPos = pos.offset(x, y, z);
                if (!isFree(level->getBlockState(checkPos)->getBlock()))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool MegaTreeFeature::makeDirtFloor(const BlockPos& pos, Level* level)
{
    BlockPos below = pos.below();
    Block* block = level->getBlockState(below)->getBlock();
    if ((block == Blocks::grass || block == Blocks::dirt) && mTreeType > 1)
    {
        setDirtAt(level, below);
        setDirtAt(level, below.east());
        setDirtAt(level, below.south());
        setDirtAt(level, below.south().east());
        return true;
    }
    return false;
}

bool MegaTreeFeature::prepareTree(Level* level, Random&, const BlockPos& pos, int height)
{
    return checkIsFree(level, pos, height) && makeDirtFloor(pos, level);
}

void MegaTreeFeature::placeDoubleTrunkLeaves(Level* level, const BlockPos& pos, int radius)
{
    int radiusSquared = radius * radius;
    for (int x = -radius; x <= radius + 1; ++x)
    {
        for (int z = -radius; z <= radius + 1; ++z)
        {
            int x1 = x - 1;
            int z1 = z - 1;
            if (x * x + z * z <= radiusSquared ||
                x1 * x1 + z1 * z1 <= radiusSquared ||
                x * x + z1 * z1 <= radiusSquared ||
                x1 * x1 + z * z <= radiusSquared)
            {
                BlockPos leafPos = pos.offset(x, 0, z);
                Block* block = level->getBlockState(leafPos)->getBlock();
                if (block == Blocks::leaves || block == Blocks::leaves2)
                {
                    setBlock(level, leafPos, mLeafState, 0);
                }
            }
        }
    }
}

void MegaTreeFeature::placeSingleTrunkLeaves(Level* level, const BlockPos& pos, int radius)
{
    int radiusSquared = radius * radius;
    for (int x = -radius; x <= radius; ++x)
    {
        for (int z = -radius; z <= radius; ++z)
        {
            if (x * x + z * z <= radiusSquared)
            {
                BlockPos leafPos = pos.offset(x, 0, z);
                Block* block = level->getBlockState(leafPos)->getBlock();
                if (block == Blocks::leaves || block == Blocks::leaves2)
                {
                    setBlock(level, leafPos, mLeafState, 0);
                }
            }
        }
    }
}
