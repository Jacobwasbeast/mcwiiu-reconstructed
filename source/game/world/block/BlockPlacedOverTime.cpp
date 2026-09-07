#include "game/world/block/BlockPlacedOverTime.hpp"

#include <map>

#include "game/world/generation/PlayerStats.hpp"

class GameStats
{
public:
    int GetRoundTime();
};

double countEmpty(const std::map<int, int>& blocksPlaced, int firstRound, int lastRound)
{
    double score = 176.0;
    int direction = lastRound < firstRound ? -1 : 1;

    if (firstRound != lastRound)
    {
        do
        {
            std::map<int, int>::const_iterator found = blocksPlaced.find(firstRound);
            if (found != blocksPlaced.end() && found->second != 0)
            {
                break;
            }

            firstRound += direction;
            score = (double)(float)(score + 10.0);
        }
        while (firstRound != lastRound);
    }

    return score;
}

BlockPlacedOverTime::BlockPlacedOverTime(
    int name,
    float target,
    unsigned int weight,
    ScoreFunction& scoreFunction,
    GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false)
    , mScoreFunction(&scoreFunction)
{
}

BlockPlacedOverTime::~BlockPlacedOverTime()
{
}

double BlockPlacedOverTime::START_DELAY(int roundTime, const std::map<int, int>& blocksPlaced)
{
    return countEmpty(blocksPlaced, 0, roundTime);
}

double BlockPlacedOverTime::TIME_LEFTOVER(int roundTime, const std::map<int, int>& blocksPlaced)
{
    return countEmpty(blocksPlaced, roundTime, 0);
}

double BlockPlacedOverTime::IN_FIRST_10SECS(int roundTime, const std::map<int, int>& blocksPlaced)
{
    std::map<int, int>::const_iterator found = blocksPlaced.find(roundTime);
    if (found == blocksPlaced.end())
    {
        return 176.0;
    }

    return (double)(found->second - 176);
}

double BlockPlacedOverTime::IN_LAST_10SECS(int roundTime, const std::map<int, int>& blocksPlaced)
{
    std::map<int, int>::const_iterator found = blocksPlaced.find(roundTime - 1);
    if (found == blocksPlaced.end())
    {
        return 176.0;
    }

    return (double)(found->second - 176);
}

double BlockPlacedOverTime::GetScore(PlayerStats* playerStats) const
{
    double score = 0.0;
    unsigned int round = 0;
    unsigned int totalRounds = playerStats->getTotalRounds();

    if (totalRounds != 0)
    {
        do
        {
            if (playerStats->hasRoundStats(round))
            {
                PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
                GameStats* gameStats = getGameStats();
                int roundTime = gameStats->GetRoundTime();
                score = (double)(float)(score + (*mScoreFunction)(roundTime / 10 - 1, roundStats->mBlocksPlacedInfo.mRounds));
            }

            ++round;
            totalRounds = playerStats->getTotalRounds();
        }
        while (round < totalRounds);
    }

    return score;
}
