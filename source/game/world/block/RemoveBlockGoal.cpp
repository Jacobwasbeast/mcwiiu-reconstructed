#include "game/world/block/RemoveBlockGoal.hpp"

#include "game/client/audio/SoundEvent.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/core/GameRules.hpp"
#include "game/entity/ai/Goal.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/server/ServerLevel.hpp"
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
    if (mMob->mLevel->getGameRules()->getBoolean(GameRules::RULE_MOBGRIEFING) &&
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
    mMob->mFallDistance = 1.0f;
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
    Level* level = mMob->mLevel;
    BlockPos origin(mMob);
    BlockPos* position = getPosWithBlock(origin, level);
    Random* random = mMob->getRandom();
    if (!isReachedTarget() || position == NULL)
    {
        delete position;
        return;
    }

    if (mTicks % 6 == 0 && mMob->mVerticalCollision)
    {
        mMob->mYd = 0.3f;
    }
    if (mTicks % 6 == 3)
    {
        mMob->mYd = -0.3f;
    }

    if (mTicks > 0)
    {
        if (!level->mIsClientSide)
        {
            int params[2] = { Block::getId(mBlock), 0 };
            if (mStartSound != NULL && mTicks % 6 == 0)
            {
                mMob->playSound(mStartSound, 0.65f, 1.0f);
            }
            double xDist = (random->nextFloat() - 0.5) * 0.08;
            double yDist = (random->nextFloat() - 0.5) * 0.08;
            double zDist = (random->nextFloat() - 0.5) * 0.08;
            static_cast<ServerLevel*>(level)->sendParticles(PARTICLE_ICON_CRACK, position->x + 0.5, position->y + 0.7,
                                                            position->z + 0.5, 3, xDist, yDist, zDist, 0.15f,
                                                            arrayWithLength<int>(params, 2));
        }

        if (mTicks > 60)
        {
            level->removeBlock(*position);
            if (!level->mIsClientSide)
            {
                if (mEndSound != NULL)
                {
                    mMob->playSound(mEndSound, 0.65f, 1.0f);
                }
                for (int i = 0; i < 20; ++i)
                {
                    double xDist = random->nextGaussian() * 0.02;
                    double yDist = random->nextGaussian() * 0.02;
                    double zDist = random->nextGaussian() * 0.02;
                    static_cast<ServerLevel*>(level)->sendParticles(PARTICLE_EXPLODE, position->x + 0.5, position->y,
                                                                    position->z + 0.5, 1, xDist, yDist,
                                                                    zDist, 0.15f, arrayWithLength<int>());
                }
            }
        }
    }
    ++mTicks;
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
