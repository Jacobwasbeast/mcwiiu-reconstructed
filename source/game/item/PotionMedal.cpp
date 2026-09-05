#include "game/item/PotionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

PotionMedal::~PotionMedal()
{
}

double PotionMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalPotionsUsed()));
}
