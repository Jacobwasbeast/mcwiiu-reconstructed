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
    if ((turtle->GetType() != 0x2d680007 || !turtle->hasEgg() || turtle->hasGivenUpOnEgg())
        && turtle->getRidingHeight() == 0.0)
    {
        return RandomStrollGoal::canUse();
    }
    return false;
}

std::wstring TurtleRandomStrollGoal::getName()
{
    return std::wstring(L"Turtle_Random_Stroll");
}
