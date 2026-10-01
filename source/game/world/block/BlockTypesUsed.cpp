#include "game/world/block/BlockTypesUsed.hpp"

#include <set>

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
        std::map<int, int>::const_iterator it = roundStats->mBlocksPresent.mByBlock.begin();
        for (; it != roundStats->mBlocksPresent.mByBlock.end(); ++it)
        {
            blockTypes.insert(it->first);
        }
    }

    return (float)blockTypes.size();
}
