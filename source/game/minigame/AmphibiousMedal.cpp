#include "game/minigame/AmphibiousMedal.hpp"

#include "game/core/StatTracker.hpp"
#include "game/world/generation/PlayerStats.hpp"

AmphibiousMedal::~AmphibiousMedal()
{
}

float AmphibiousMedal::GetScore(PlayerStats* playerStats) const
{
    return static_cast<float>(playerStats->mAmphibiousKills->getValue());
}
