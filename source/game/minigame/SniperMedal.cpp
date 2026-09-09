#include "game/minigame/SniperMedal.hpp"

#include "game/core/SniperStatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

SniperMedal::SniperMedal(int name, float target, unsigned int weight)
    : GameMedal(name, target, weight, OPERATOR_AT_LEAST, false, false)
{
}

SniperMedal::~SniperMedal()
{
}

double SniperMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->mSniper->mValue));
}
