#include "game/item/SplashPotionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

SplashPotionMedal::~SplashPotionMedal()
{
}

double SplashPotionMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<double>(static_cast<float>(playerStats->getTotalSplashPotionsUsed()));
}
