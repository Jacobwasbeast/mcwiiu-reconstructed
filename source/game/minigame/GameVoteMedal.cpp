#include "game/minigame/GameVoteMedal.hpp"

#include <math.h>

GameVoteMedal::~GameVoteMedal()
{
}

double GameVoteMedal::GetScore(PlayerStats* playerStats) const
{
    float roundCount = 0.0f;
    float score = 0.0f;
    unsigned int round = 0;
    unsigned int totalRounds = playerStats->getTotalRounds();

    while (round < totalRounds)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        float votes[4];
        votes[0] = static_cast<float>(roundStats->mScoresReceived[0]);
        votes[1] = static_cast<float>(roundStats->mScoresReceived[1]);
        votes[2] = static_cast<float>(roundStats->mScoresReceived[2]);
        votes[3] = static_cast<float>(roundStats->mScoresReceived[3]);

        float totalVotes = votes[0] + votes[1] + votes[2] + votes[3];
        roundCount += 1.0f;

        float rangeScore = (float)mRanges[0].getScore(votes[0] / totalVotes);
        if (rangeScore < 0.0f)
        {
            score += 0.0f;
        }
        else
        {
            float secondScore = (float)mRanges[1].getScore(votes[1] / totalVotes);
            if (secondScore < 0.0f)
            {
                score += 0.0f;
            }
            else
            {
                float thirdScore = (float)mRanges[2].getScore(votes[2] / totalVotes);
                if (thirdScore < 0.0f)
                {
                    score += 0.0f;
                }
                else
                {
                    float fourthScore = (float)mRanges[3].getScore(votes[3] / totalVotes);
                    if (fourthScore < 0.0f)
                    {
                        score += 0.0f;
                    }
                    else
                    {
                        score += rangeScore + secondScore + thirdScore + fourthScore;
                    }
                }
            }
        }

        totalRounds = playerStats->getTotalRounds();
        ++round;
    }

    return (double)((100.0f * score) / (roundCount + 4.0f));
}

double GameVoteMedal::Range::getScore(float score) const
{
    float difference = fabsf(score - mTarget);
    if (difference < mRange)
    {
        return (double)powf((mRange - difference) / mRange, 1.5f);
    }

    return -1.0;
}
