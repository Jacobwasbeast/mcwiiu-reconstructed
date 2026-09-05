#include "game/core/ShowdownTimeCondition.hpp"

#include "game/core/MasterGameMode.hpp"

ShowdownTimeCondition::ShowdownTimeCondition(const XmlObject& object)
    : TimerBasedCondition(object)
{
}

ShowdownTimeCondition::~ShowdownTimeCondition()
{
}

int ShowdownTimeCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetShowdownTime();
}

unsigned int ShowdownTimeCondition::GetTimerID() const
{
    return 8;
}

bool ShowdownTimeCondition::ShouldBlock() const
{
    return false;
}
