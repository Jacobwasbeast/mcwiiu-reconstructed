#include "game/world/block/BlockRemover.hpp"

#include "game/client/render/ParticleType.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BlockPos.hpp"

BlockRemover::Area::Area(AABB* bounds)
    : mBounds(bounds),
      mX((int)bounds->mMinX),
      mY((int)bounds->mMaxY),
      mZ((int)bounds->mMinZ)
{
}

BlockRemover::BlockRemover(Level* level, std::vector<AABB*>& areas)
{
    for (std::vector<AABB*>::iterator it = areas.begin(); it != areas.end(); ++it)
    {
        mAreas.push_back(Area(*it));
    }

    mLevel = level;
    mFinished = false;
    mTicks = 0;
    mRemovalCount = 0;
    mBlocksPerRemoval = 5;
    mTicksPerRemoval = 20;
}

BlockRemover::~BlockRemover()
{
}

bool BlockRemover::Finished()
{
    return mFinished;
}

void BlockRemover::Tick()
{
    if (!Finished())
    {
        if (mTicks % mTicksPerRemoval == 0)
        {
            bool removing = false;

            for (unsigned int areaIndex = 0; areaIndex < mAreas.size(); ++areaIndex)
            {
                removing |= RemoveBlocks(areaIndex, mBlocksPerRemoval);
            }

            if (!removing)
            {
                mFinished = true;
            }

            ++mRemovalCount;
        }

        ++mTicks;
    }
}

void BlockRemover::CreateParticles(const BlockPos& pos, int colour)
{
    arrayWithLength<int> params;

    static_cast<ServerLevel*>(mLevel)->sendParticles(PARTICLE_MOB_SPELL,
                                                     pos.x,
                                                     pos.y,
                                                     pos.z,
                                                     5,
                                                     3.0f,
                                                     3.0f,
                                                     3.0f,
                                                     0.2f,
                                                     params,
                                                     ((colour >> 16) & 0xFF) / 255.0f,
                                                     ((colour >> 8) & 0xFF) / 255.0f,
                                                     (colour & 0xFF) / 255.0f);
}
