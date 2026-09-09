#include "game/entity/ai/TurtleRandomStrollGoal.hpp"

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
    if ((turtle->GetType() != 0x2D680007ULL || !turtle->hasEgg() || turtle->hasGivenUpOnEgg())
        && !turtle->isInWater())
    {
        return RandomStrollGoal::canUse();
    }
    return false;
}

std::wstring TurtleRandomStrollGoal::getName()
{
    return std::wstring(L"Turtle_Random_Stroll");
}
