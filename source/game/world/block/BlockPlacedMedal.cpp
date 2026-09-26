#include "game/world/block/BlockPlacedMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlockPlacedMedal::BlockPlacedMedal(int name, float target, unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false)
{
}

BlockPlacedMedal::~BlockPlacedMedal()
{
}

double BlockPlacedMedal::GetScore(PlayerStats* playerStats) const
{
    return GameMedal::totalOfAllRounds(playerStats, getBlocksPlaced);
}
