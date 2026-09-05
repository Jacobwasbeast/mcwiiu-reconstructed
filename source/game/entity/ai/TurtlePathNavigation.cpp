#include "game/entity/ai/TurtlePathNavigation.hpp"

#include "game/core/PathFinder.hpp"
#include "game/core/Turtle.hpp"
#include "game/entity/ai/AmphibiousNodeEvaluator.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"

TurtlePathNavigation::TurtlePathNavigation(Turtle* turtle, Level* level)
    : WaterBoundPathNavigation(turtle, level, false, false)
{
    mPathFinder = createPathFinder();
}

TurtlePathNavigation::~TurtlePathNavigation()
{
}

PathFinder* TurtlePathNavigation::createPathFinder()
{
    AmphibiousNodeEvaluator* evaluator = new AmphibiousNodeEvaluator(mMob);
    return new PathFinder(evaluator);
}

bool TurtlePathNavigation::canUpdatePath()
{
    return true;
}

bool TurtlePathNavigation::isStableDestination(const BlockPos& pos)
{
    if (mMob->GetType() == 0x2D680007ULL &&
        static_cast<Turtle*>(mMob)->isTravelling() &&
        mMob->getRidingHeight() != 0.0)
        return !mLevel->getBlockState(pos)->isSolid();

    return mLevel->getBlockState(pos.below())->isSolid();
}
