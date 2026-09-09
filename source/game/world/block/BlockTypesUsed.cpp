#include "game/world/block/BlockTypesUsed.hpp"

#include <set>

BlockTypesUsed::~BlockTypesUsed()
{
}

double BlockTypesUsed::GetScore(PlayerStats* playerStats) const
{
    std::set<int> blockTypes;

    for (unsigned int round = 0; round < (unsigned int)playerStats->getTotalRounds(); round++)
    {
        if (!playerStats->hasRoundStats(round))
        {
            continue;
        }

        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        std::map<int, int>::const_iterator it = roundStats->mBlocksUsed.mItems.begin();
        for (; it != roundStats->mBlocksUsed.mItems.end(); ++it)
        {
            blockTypes.insert(it->first);
        }
    }

    return (float)blockTypes.size();
}
