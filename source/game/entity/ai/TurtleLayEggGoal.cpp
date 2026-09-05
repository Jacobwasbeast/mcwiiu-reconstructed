#include "game/entity/ai/TurtleLayEggGoal.hpp"

#include <cmath>

#include "game/core/Turtle.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

TurtleLayEggGoal::TurtleLayEggGoal(Turtle* turtle, double speedModifier,
                                   double maxStayDistance)
    : MoveToBlockGoal(turtle, speedModifier, 16, 3, maxStayDistance),
      mTurtle(turtle),
      mLayEggCounter(0),
      mSpeedModifierSquared(speedModifier * speedModifier)
{
}

TurtleLayEggGoal::~TurtleLayEggGoal()
{
}

bool TurtleLayEggGoal::isNearHome()
{
    BlockPos position(mTurtle->getPos(1.65f));
    BlockPos home = mTurtle->getHomePos();
    return home.distSqr(position) < mSpeedModifierSquared;
}

bool TurtleLayEggGoal::homeIsValid()
{
    if (mVerticalSearchStart < 0)
        return false;

    for (int y = 0; y <= mVerticalSearchStart; ++y)
    {
        for (int x = -mSearchRange; x <= mSearchRange; ++x)
        {
            for (int z = -mSearchRange; z <= mSearchRange; ++z)
            {
                BlockPos candidate = mTurtle->getHomePos().offset(x, y - 1, z);
                if (isValidTarget(mTurtle->mLevel, candidate))
                    return true;
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

    if (mBlockPos == BlockPos::ZERO && !homeIsValid())
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

    BlockPos position(mTurtle->getPos(1.65f));
    if (!isNearHome())
        return;

    if (!isReachedTarget() && !mTurtle->mLevel->isEmptyBlock(position.below()))
        return;

    if (mLayEggCounter < 1)
    {
        mTurtle->setIsLayingEgg(true);
    }
    else if (mLayEggCounter > 200)
    {
        int eggs = mTurtle->mRandom->nextInt(4) + 1;
        const BlockState* state = Blocks::turtle_egg->getBlockState(eggs);
        mTurtle->mLevel->setBlock(position, state, 3, false);
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
    int radius = (int)std::floor(mTurtle->mY);
    for (int x = -radius; x <= radius; ++x)
    {
        for (int z = -radius; z <= radius; ++z)
        {
            BlockPos candidate = pos.offset(x, 0, z);
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
