#include "game/core/TotalDamageTakenCondition.hpp"

#include "game/world/generation/PlayerStats.hpp"

TotalDamageTakenCondition::TotalDamageTakenCondition(float target, GameMedal::EOperator op, bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly)
{
}

TotalDamageTakenCondition::~TotalDamageTakenCondition()
{
}

double TotalDamageTakenCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return playerStats->getAllDamageTaken();
}
