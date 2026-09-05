#include "game/core/WinRoundCountCondition.hpp"

WinRoundCountCondition::~WinRoundCountCondition()
{
}

double WinRoundCountCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    return static_cast<double>(playerStats->getTotalRoundsWon());
}
