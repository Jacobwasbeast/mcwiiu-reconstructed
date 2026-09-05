#include "game/minigame/LifeSpanMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

LifeSpanMedal::LifeSpanMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false)
{
}

LifeSpanMedal::~LifeSpanMedal()
{
}

double LifeSpanMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalLifeSpan()));
}
