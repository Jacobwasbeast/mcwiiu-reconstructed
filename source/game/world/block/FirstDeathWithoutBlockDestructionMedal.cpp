#include "game/world/block/FirstDeathWithoutBlockDestructionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

FirstDeathWithoutBlockDestructionMedal::~FirstDeathWithoutBlockDestructionMedal()
{
}

double FirstDeathWithoutBlockDestructionMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, lostFirstWithoutDestroyingBlocks);
}
