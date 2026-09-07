#include "game/world/block/BlockPresentMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

int getBlocksPresent(const PlayerStats::RoundStats& roundStats);

BlockPresentMedal::BlockPresentMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op)
{
}

BlockPresentMedal::~BlockPresentMedal()
{
}

double BlockPresentMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, getBlocksPresent);
}
