#include "game/world/block/WinnerWithoutBlockDestructionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

WinnerWithoutBlockDestructionMedal::~WinnerWithoutBlockDestructionMedal()
{
}

double WinnerWithoutBlockDestructionMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, wasWinnerWithoutDestroyingBlocks);
}
