#include "game/world/block/WinnerWithoutBlockDestructionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

double WinnerWithoutBlockDestructionMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, wasWinnerWithoutDestroyingBlocks);
}
