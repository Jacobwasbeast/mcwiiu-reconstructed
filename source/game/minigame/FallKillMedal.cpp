#include "game/minigame/FallKillMedal.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

FallKillMedal::FallKillMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, true, false)
{
}

FallKillMedal::~FallKillMedal()
{
}

double FallKillMedal::GetScore(PlayerStats* playerStats) const
{
    return (double)(float)playerStats->mFallKills->getValue();
}
