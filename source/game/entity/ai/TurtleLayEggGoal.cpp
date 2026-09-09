#include "game/entity/ai/TurtleLayEggGoal.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

TurtleLayEggGoal::TurtleLayEggGoal(Turtle* turtle, double speedModifier,
                                   double searchDistance)
    : MoveToBlockGoal(turtle, speedModifier, (int)std::ceil(searchDistance), 1, 1.0),
      mTurtle(turtle),
      mLayEggCounter(0),
      mSearchDistanceSquared(searchDistance * searchDistance)
{
}

TurtleLayEggGoal::~TurtleLayEggGoal()
{
}

bool TurtleLayEggGoal::isNearHome()
{
    BlockPos position(mTurtle->getPos(1.65f));
    BlockPos home = mTurtle->getHomePos();
    return position.distSqr((double)home.x, (double)home.y, (double)home.z) < mSearchDistanceSquared;
}

bool TurtleLayEggGoal::homeIsValid()
{
    if (mVerticalSearchStart < 0)
    {
        return false;
    }

    for (int y = 0; y <= mVerticalSearchStart; y = y <= 0 ? 1 - y : -y)
    {
        for (int radius = 0; radius < mSearchRange; ++radius)
        {
            for (int x = 0; x <= radius; x = x <= 0 ? 1 - x : -x)
            {
                int firstZ = x == radius || x == -radius ? 0 : radius;
                for (int z = firstZ; z <= radius; z = z <= 0 ? 1 - z : -z)
                {
                    BlockPos candidate = mTurtle->getHomePos().offset(x, y - 1, z);
                    if (isValidTarget(mTurtle->mLevel, candidate))
                    {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool TurtleLayEggGoal::canUse()
{
    if (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg() || !isNearHome())
        return false;

    if (MoveToBlockGoal::canUse())
        return true;

    if (mNextStartTick == GetNextStartTick(mTurtle) && mBlockPos == BlockPos::ZERO && !homeIsValid())
        mTurtle->GiveUpOnEgg();

    return false;
}

bool TurtleLayEggGoal::canContinueToUse()
{
    if (!mTurtle->hasEgg() || mTurtle->hasGivenUpOnEgg() || !isNearHome())
        return false;

    if (mLayEggCounter < 1)
        return MoveToBlockGoal::canContinueToUse();

    return true;
}

void TurtleLayEggGoal::tick()
{
    MoveToBlockGoal::tick();

    Level* level = mTurtle->mLevel;
    BlockPos position(mTurtle->getPos(1.65f));
    if (!isNearHome())
        return;

    if (!isReachedTarget() && !isValidTarget(level, position.below()))
        return;

    if (mLayEggCounter < 1)
    {
        mTurtle->setIsLayingEgg(true);
    }
    else if (mLayEggCounter > 200)
    {
        int eggs = level->mRandom->nextInt(4);
        level->playSound(boost::shared_ptr<Player>(), position, SoundEvents::TURTLE_LAY_EGG,
                         SoundSource::eSoundSource_Blocks, 0.3f,
                         level->mRandom->nextFloat() * 0.2f + 0.9f, 16.0f);
        const BlockState* state = Blocks::turtle_egg->getBlockState(eggs);
        level->setBlock(position, state, 3, false);
        mTurtle->setHasEgg(false);
        mTurtle->setIsLayingEgg(false);
    }

    if (mTurtle->isLayingEgg())
        ++mLayEggCounter;
}

void TurtleLayEggGoal::start()
{
    MoveToBlockGoal::start();
    mLayEggCounter = 0;
}

bool TurtleLayEggGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    int radius = (int)std::floor(mTurtle->mBbWidth);
    for (int x = -radius; x <= radius; ++x)
    {
        for (int z = -radius; z <= radius; ++z)
        {
            BlockPos candidate(pos.x + x, pos.y, pos.z + z);
            if (!level->isEmptyBlock(candidate.above()) ||
                level->getBlock(candidate) != Blocks::sand)
                return false;
        }
    }

    return true;
}

int TurtleLayEggGoal::GetNextStartTick(PathfinderMob*)
{
    return 20;
}

std::wstring TurtleLayEggGoal::getName()
{
    return std::wstring(L"Turtle_Lay_Egg");
}
