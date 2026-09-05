#include "game/world/block/RemoveBlockGoal.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"

RemoveBlockGoal::RemoveBlockGoal(Block* block, PathfinderMob* mob,
                                 double speedModifier, int searchRange,
                                 int verticalSearchStart,
                                 double maxStayDistance,
                                 const SoundEvent* startSound,
                                 const SoundEvent* endSound)
    : MoveToBlockGoal(mob, speedModifier, searchRange, verticalSearchStart,
                      maxStayDistance),
      mBlock(block),
      mMob(mob),
      mTicks(0),
      mStartSound(startSound),
      mEndSound(endSound)
{
}

RemoveBlockGoal::~RemoveBlockGoal()
{
}

bool RemoveBlockGoal::canUse()
{
    if (mMob->mLevel->getGameRules()->getBoolean(1) &&
        mMob->getRandom()->nextInt(20) == 0)
    {
        return MoveToBlockGoal::canUse();
    }
    return false;
}

int RemoveBlockGoal::GetNextStartTick(PathfinderMob*)
{
    return 0;
}

bool RemoveBlockGoal::canContinueToUse()
{
    return MoveToBlockGoal::canContinueToUse();
}

void RemoveBlockGoal::stop()
{
    Goal::stop();
}

void RemoveBlockGoal::start()
{
    MoveToBlockGoal::start();
    mTicks = 0;
}

BlockPos* RemoveBlockGoal::getPosWithBlock(BlockPos pos, Level* level)
{
    if (level->getBlockState(pos)->getBlock() == mBlock)
    {
        return new BlockPos(pos);
    }

    BlockPos positions[6] = {pos.below(), pos.west(), pos.east(),
                             pos.north(), pos.south(), pos.below().below()};
    for (int i = 0; i < 6; ++i)
    {
        if (level->getBlockState(positions[i])->getBlock() == mBlock)
        {
            return new BlockPos(positions[i]);
        }
    }
    return NULL;
}

void RemoveBlockGoal::tick()
{
    MoveToBlockGoal::tick();
    BlockPos origin(mMob);
    BlockPos* position = getPosWithBlock(origin, mMob->mLevel);
    if (!isReachedTarget())
    {
        delete position;
        return;
    }
    if (position != NULL)
    {
        if (mTicks % 6 == 0 && mMob->mLevel->mIsClientSide)
        {
            mMob->playSound(mStartSound, 1.0f, 1.0f);
        }
        if (mTicks % 6 == 3)
        {
            mMob->playSound(mStartSound, 1.0f, 1.0f);
        }
        if (mTicks > 0 && !mMob->mLevel->mIsClientSide)
        {
            if (mTicks % 6 == 0)
            {
                mMob->playSound(mStartSound, 1.0f, 1.0f);
            }
            if (mTicks > 60)
            {
                mMob->mLevel->destroyBlock(*position, false);
                mMob->playSound(mEndSound, 1.0f, 1.0f);
            }
        }
        ++mTicks;
    }
    delete position;
}

bool RemoveBlockGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos)->getBlock() == mBlock;
}

bool RemoveBlockGoal::isValidTargetArea(Level* level, const BlockPos& from,
                                        const BlockPos& to)
{
    int blockId = Block::getId(mBlock);
    for (int x = from.x; x < to.x; x += 4)
    {
        for (int y = from.y; y < to.y; y += 4)
        {
            for (int z = from.z; z < to.z; z += 4)
            {
                if (level->sectionContainsBlockId(x, y, z, blockId))
                {
                    return true;
                }
            }
        }
    }
    return false;
}
