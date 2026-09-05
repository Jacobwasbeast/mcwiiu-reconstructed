#include "game/world/block/BlocksPresentWithIdMedal.hpp"

#include "game/world/generation/PlayerStats.hpp"

BlocksPresentWithIdMedal::~BlocksPresentWithIdMedal()
{
}

double BlocksPresentWithIdMedal::GetScore(PlayerStats* playerStats) const
{
    double score = 1.0;
    unsigned int round = 0;
    unsigned int totalRounds = playerStats->getTotalRounds();

    if (totalRounds != 0)
    {
        do
        {
            if (playerStats->hasRoundStats(round))
            {
                PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
                int blockCount = 0;

                for (std::set<int>::const_iterator it = mBlockIds.begin(); it != mBlockIds.end(); ++it)
                    blockCount += roundStats->mBlocksPlacedInfo.mRounds[*it];

                score = static_cast<double>((*mScoreFunction)(blockCount, roundStats->mBlocksPlacedInfo.mCount));
            }

            ++round;
            totalRounds = playerStats->getTotalRounds();
        }
        while (round < totalRounds);
    }

    return score;
}
