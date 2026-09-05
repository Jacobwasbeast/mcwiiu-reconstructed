#include "game/world/block/SpiralBlockRemover.hpp"

#include "game/util/AABB.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"

SpiralBlockRemover::~SpiralBlockRemover()
{
}

void SpiralBlockRemover::Tick()
{
    BlockRemover::Tick();

    if (mTicks % mBlocksPerRemoval == 0)
    {
        ++mBlocksPerRemoval;
    }
}

bool SpiralBlockRemover::RemoveBlocks(int areaIndex, int maxBlocks)
{
    Area& area = mAreas[areaIndex];
    Spiral& spiral = mSpirals[areaIndex];
    bool alongZ = area.mBounds->mMaxX - area.mBounds->mMinX == 1.0;
    int removed = 0;
    int x;
    int y;
    BlockPos pos;

    while (removed < maxBlocks && spiral.GetNext(x, y))
    {
        if (alongZ)
        {
            pos = BlockPos((int)area.mBounds->mMinX, y, x);
        }
        else
        {
            pos = BlockPos(x, y, (int)area.mBounds->mMinZ);
        }

        if (!area.mBounds->containsIncludingLowerBound(Vec3::newTemp(&pos)))
        {
            continue;
        }

        mLevel->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
        ++removed;
    }

    if (removed > 0)
    {
        CreateParticles(pos, 0);
    }

    return maxBlocks <= removed;
}
