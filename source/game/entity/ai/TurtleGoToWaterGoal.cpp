#include "game/entity/ai/TurtleGoToWaterGoal.hpp"

#include "game/core/Turtle.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

TurtleGoToWaterGoal::TurtleGoToWaterGoal(Turtle* turtle, double speedModifier)
    : MoveToBlockGoal(turtle, speedModifier, 16, 1, 1.0),
      mTurtle(turtle)
{
    if (turtle->isBaby())
    {
        mSpeedModifier = 2.0;
    }
}

TurtleGoToWaterGoal::~TurtleGoToWaterGoal()
{
}

bool TurtleGoToWaterGoal::canUse()
{
    if (mTurtle->isBaby() && !mTurtle->isInWater())
    {
        return MoveToBlockGoal::canUse();
    }

    if (mTurtle->isGoingHome() || mTurtle->isInWater())
    {
        return false;
    }

    return MoveToBlockGoal::canUse();
}

bool TurtleGoToWaterGoal::canContinueToUse()
{
    return !mTurtle->isInWater() &&
           mTicksStayingStill <= 1200 &&
           isValidTarget(mTurtle->mLevel, mBlockPos);
}

bool TurtleGoToWaterGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    Block* block = level->getBlockState(pos)->getBlock();
    return block == Blocks::water || block == Blocks::flowing_water;
}

std::wstring TurtleGoToWaterGoal::getName()
{
    return std::wstring(L"Turtle Goto Water");
}
