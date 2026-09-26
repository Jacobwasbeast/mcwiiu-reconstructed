#include "game/world/block/BlocksPresentWithIdMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlocksPresentWithIdMedal::~BlocksPresentWithIdMedal()
{
}

double BlocksPresentWithIdMedal::GetScore(PlayerStats* playerStats) const
{
    float score = 0.0f;

    for (unsigned int round = 0; round < (unsigned int)playerStats->getTotalRounds(); round++)
    {
        if (!playerStats->hasRoundStats(round))
        {
            continue;
        }

        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        int blockCount = 0;

        std::set<int>::const_iterator it = mBlockIds.begin();
        for (; it != mBlockIds.end(); ++it)
        {
            blockCount += roundStats->mBlocksPresent.mByBlock[*it];
        }

        score = (*mScoreFunction)(blockCount, roundStats->mBlocksPresent.mCount);
    }

    return score;
}
