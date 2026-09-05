#include "game/core/RoundTimeCondition.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/TimeCondition.hpp"
#include "game/core/XmlObject.hpp"

RoundTimeCondition::RoundTimeCondition(const XmlObject& object)
    : TimerBasedCondition(object)
{
}

RoundTimeCondition::~RoundTimeCondition()
{
}

int RoundTimeCondition::GetValue(MasterGameMode* gameMode) const
{
    return gameMode->GetRoundTime();
}

unsigned int RoundTimeCondition::GetThreshold(MasterGameMode* gameMode) const
{
    return TimeCondition::GetThreshold(gameMode);
}

unsigned int RoundTimeCondition::GetTimerID() const
{
    return 7;
}

bool RoundTimeCondition::ShouldBlock() const
{
    return true;
}
