#include "game/entity/ai/TurtleRandomStrollGoal.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Turtle.hpp"

TurtleRandomStrollGoal::TurtleRandomStrollGoal(Turtle* turtle, double speedModifier, int interval)
    : RandomStrollGoal(turtle, speedModifier, interval)
{
}

TurtleRandomStrollGoal::~TurtleRandomStrollGoal()
{
}

bool TurtleRandomStrollGoal::canUse()
{
    Turtle* turtle = static_cast<Turtle*>(mMob);
    if ((turtle->GetType() != EntityType::TURTLE || !turtle->hasEgg() || turtle->hasGivenUpOnEgg())
        && !turtle->isInWater())
    {
        return RandomStrollGoal::canUse();
    }
    return false;
}

