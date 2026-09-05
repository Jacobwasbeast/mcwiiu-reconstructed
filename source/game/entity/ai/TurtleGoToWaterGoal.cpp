#include "game/entity/ai/TurtleGoToWaterGoal.hpp"

#include "game/core/Turtle.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"

TurtleGoToWaterGoal::TurtleGoToWaterGoal(Turtle* turtle, double speedModifier)
    : MoveToBlockGoal(turtle, speedModifier, 16, 1, 1.0),
      mTurtle(turtle)
{
}

TurtleGoToWaterGoal::~TurtleGoToWaterGoal()
{
}

bool TurtleGoToWaterGoal::canUse()
{
    if ((!mTurtle->isBaby() && mTurtle->isGoingHome()) ||
        mTurtle->isInWater())
    {
        return false;
    }

    return MoveToBlockGoal::canUse();
}

bool TurtleGoToWaterGoal::canContinueToUse()
{
    return !mTurtle->isInWater() &&
           mTicksStayingStill < 1200 &&
           isValidTarget(mTurtle->mLevel, mBlockPos);
}

bool TurtleGoToWaterGoal::isValidTarget(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos)->getMaterial() == Material::WATER;
}

std::wstring TurtleGoToWaterGoal::getName()
{
    return std::wstring(L"Turtle_Goto_Water");
}
