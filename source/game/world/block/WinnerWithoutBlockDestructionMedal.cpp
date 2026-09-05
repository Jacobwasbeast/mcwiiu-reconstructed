#include "game/world/block/WinnerWithoutBlockDestructionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

int wasWinnerWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats);

WinnerWithoutBlockDestructionMedal::~WinnerWithoutBlockDestructionMedal()
{
}

double WinnerWithoutBlockDestructionMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, wasWinnerWithoutDestroyingBlocks);
}
