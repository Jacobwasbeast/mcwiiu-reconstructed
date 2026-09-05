#include "game/core/TimeSpentCondition.hpp"

TimeSpentCondition::TimeSpentCondition(float target, eTimeSpentCategory category,
    GameMedal::EOperator op, bool bestOnly)
    : MultiConditionMedal::MultiCondition(target, op, bestOnly)
    , mCategory(category)
{
}

TimeSpentCondition::~TimeSpentCondition()
{
}

double TimeSpentCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    bool bestOnly = mBestOnly;
    unsigned int score = bestOnly ? 0xffffffffU : 0U;

    unsigned int round = 0;
    unsigned int totalRounds = static_cast<unsigned int>(playerStats->getTotalRounds());
    while (round < totalRounds)
    {
        PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
        unsigned int value;

        if (mCategory == 2)
        {
            int previous = 0;
            unsigned int layer = static_cast<unsigned int>(roundStats->mLayer);
            if (roundStats->mLayer != -1)
            {
                unsigned int thermal = 0;
                while (thermal <= layer)
                {
                    boost::unordered_map<int, int>::iterator found =
                        roundStats->mThermalsEntered.find(static_cast<int>(thermal));
                    if (found != roundStats->mThermalsEntered.end())
                    {
                        value = static_cast<unsigned int>(roundStats->mThermalsEntered.at(
                            static_cast<int>(thermal)) - previous);
                        previous = roundStats->mThermalsEntered.at(static_cast<int>(thermal));
                        if (bestOnly)
                        {
                            if (value < score)
                                score = value;
                        }
                        else if (score < value)
                        {
                            score = value;
                        }
                    }
                    ++thermal;
                }
            }
        }
        else
        {
            value = static_cast<unsigned int>(roundStats->mTimeSpent[mCategory]);
            if (bestOnly)
            {
                if (value < score)
                    score = value;
            }
            else if (score < value)
            {
                score = value;
            }
        }

        ++round;
        totalRounds = static_cast<unsigned int>(playerStats->getTotalRounds());
    }

    return static_cast<double>(static_cast<float>(score));
}
