#include "game/world/block/FirstDeathWithoutBlockDestructionMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

int lostFirstWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats);

int lostFirstWithoutDestroyingBlocks(const PlayerStats::RoundStats& roundStats)
{
    return roundStats.mReachedEnd && roundStats.mBlocksDestroyed.mCount == 0;
}

FirstDeathWithoutBlockDestructionMedal::~FirstDeathWithoutBlockDestructionMedal()
{
}

double FirstDeathWithoutBlockDestructionMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, lostFirstWithoutDestroyingBlocks);
}
