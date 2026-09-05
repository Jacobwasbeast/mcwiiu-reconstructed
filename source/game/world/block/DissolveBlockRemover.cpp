#include "game/world/block/DissolveBlockRemover.hpp"

#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MaterialColor.hpp"

DissolveBlockRemover::DissolveBlockRemover(Level* level, std::vector<AABB*>& areas, int ticks, int interval)
    : BlockRemover(level, areas),
      mDissolveInterval(ticks)
{
    mTicksPerRemoval = interval;
    mBlocksPerRemoval = 1;
}

DissolveBlockRemover::~DissolveBlockRemover()
{
}

void DissolveBlockRemover::Tick()
{
    BlockRemover::Tick();

    if (mDissolveInterval <= mRemovalCount)
    {
        mFinished = true;
    }
}

bool DissolveBlockRemover::RemoveBlocks(int areaIndex, int maxBlocks)
{
    Area& area = mAreas[areaIndex];
    int phase = -mRemovalCount;

    for (int y = (int)area.mBounds->mMinY; y < (int)area.mBounds->mMaxY; ++y)
    {
        for (int z = (int)area.mBounds->mMinZ; z < (int)area.mBounds->mMaxZ; ++z)
        {
            for (int x = (int)area.mBounds->mMinX; x < (int)area.mBounds->mMaxX; ++x)
            {
                if (phase >= 0 && phase % mDissolveInterval == 0)
                {
                    BlockPos pos(x, y, z);
                    const BlockState* state = mLevel->getBlockState(pos);
                    int colour = state->getMapColor(mLevel, pos)->calculateRGBColor(2);
                    mLevel->setBlock(pos, Blocks::AIR->defaultBlockState(), 3, false);
                    CreateParticles(pos, colour);
                }

                ++phase;
            }
        }
    }

    return true;
}
