#include "game/core/WonByAmountCondition.hpp"

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#define eTimeSpentCategory eGameStatsTimeSpentCategory
#include "game/core/GameStats.hpp"
#undef eTimeSpentCategory
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"
#include "game/minigame/MiniGameDef.hpp"

WonByAmountCondition::WonByAmountCondition(float target, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, false)
{
}

WonByAmountCondition::~WonByAmountCondition()
{
}

bool WonByAmountCondition::canAward(const GameMedal* medal, const StatsUID& uid, GameStats* gameStats) const
{
    PlayerStats* playerStats = gameStats->GetPlayerStats(uid);
    if (playerStats == NULL)
    {
        return false;
    }

    typedef boost::unordered_map<StatsUID, PlayerStats*, boost::hash<StatsUID> > PlayerStatsMap;
    PlayerStatsMap* allPlayerStats = gameStats->GetPlayerStats();
    unsigned int bestOpponentScore = static_cast<unsigned int>(-1);
    unsigned int playerScore = static_cast<unsigned int>(-1);
    unsigned int playerRound = 0;
    unsigned int round = 0;
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    unsigned int roundCount = miniGame->GetRoundCount();

    while (round <= roundCount)
    {
        playerRound = 0;
        while (playerRound < static_cast<unsigned int>(playerStats->getTotalRounds()))
        {
            playerScore = static_cast<unsigned int>(playerStats->getRoundStats(playerRound)->mPoints);
            if (allPlayerStats->empty())
            {
                break;
            }

            for (PlayerStatsMap::const_iterator it = allPlayerStats->begin();
                 it != allPlayerStats->end(); ++it)
            {
                if (it->first != uid && it->second->hasRoundStats(round))
                {
                    PlayerStats::RoundStats* opponentStats = it->second->getRoundStats(round);
                    if (opponentStats->mFinished)
                    {
                        unsigned int score = static_cast<unsigned int>(opponentStats->mPoints);
                        if (score < bestOpponentScore)
                        {
                            bestOpponentScore = score;
                        }
                    }
                }
            }

            ++playerRound;
        }

        if (playerScore < bestOpponentScore)
        {
            unsigned int amount = bestOpponentScore - playerScore;
            if (mOperator == GameMedal::OPERATOR_AT_MOST && static_cast<float>(amount) <= mTarget)
            {
                return true;
            }

            if (mOperator == GameMedal::OPERATOR_AT_LEAST && mTarget <= static_cast<float>(amount))
            {
                return true;
            }
        }

        ++round;
        roundCount = Minecraft::GetInstance()->GetMiniGame()->GetRoundCount();
    }

    return false;
}
