#include "game/core/FastestTimeAndHightestScoreInARoundCondition.hpp"

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/GameStats.hpp"
#include "game/core/StatsUID.hpp"
#include "game/world/generation/PlayerStats.hpp"

FastestTimeAndHightestScoreInARoundCondition::FastestTimeAndHightestScoreInARoundCondition()
    : MultiConditionMedal::MultiCondition(-1.0f, GameMedal::OPERATOR_EQUAL, false)
{
}

FastestTimeAndHightestScoreInARoundCondition::~FastestTimeAndHightestScoreInARoundCondition()
{
}

bool FastestTimeAndHightestScoreInARoundCondition::canAward(const GameMedal* medal,
                                                             const StatsUID& uid,
                                                             GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    typedef boost::unordered_map<StatsUID, PlayerStats*, boost::hash<StatsUID> > PlayerStatsMap;
    PlayerStatsMap* allPlayerStats = gameStats->GetPlayerStats();
    const unsigned int roundCount = static_cast<unsigned int>(playerStats->getTotalRounds());

    for (unsigned int round = 0; round < roundCount; ++round)
    {
        if (playerStats->getReachedEnd(round) == 0)
        {
            continue;
        }

        if (allPlayerStats->empty())
        {
            const unsigned int fastestTime = static_cast<unsigned int>(playerStats->getLifeSpan(round));
            playerStats->getPoints(round);
            if (fastestTime < 0x80000000U)
            {
                return true;
            }
        }
        else
        {
            unsigned int fastestTime = 0x7fffffffU;
            unsigned int highestScore = 0;
            PlayerStatsMap::iterator it = allPlayerStats->begin();
            for (; it != allPlayerStats->end(); ++it)
            {
                PlayerStats* otherStats = it->second;
                if (otherStats->getReachedEnd(round) != 0)
                {
                    const unsigned int time = static_cast<unsigned int>(otherStats->getLifeSpan(round));
                    if (time < fastestTime)
                    {
                        fastestTime = time;
                    }
                }

                const unsigned int score = static_cast<unsigned int>(otherStats->getPoints(round));
                if (score > highestScore)
                {
                    highestScore = score;
                }
            }

            const unsigned int ownTime = static_cast<unsigned int>(playerStats->getLifeSpan(round));
            const unsigned int ownScore = static_cast<unsigned int>(playerStats->getPoints(round));
            if (ownTime <= fastestTime && ownScore >= highestScore)
            {
                return true;
            }
        }
    }

    return false;
}
