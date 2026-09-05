#include "game/core/WonByAmountCondition.hpp"

#include <boost/functional/hash.hpp>
#include <boost/unordered_map.hpp>

#include "game/core/GameStats.hpp"
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
    int round = 0;
    const MiniGameDef* miniGame = Minecraft::GetInstance()->GetMiniGame();
    int roundCount = miniGame->GetRoundCount();

    while (round <= roundCount)
    {
        playerRound = 0;
        while (playerRound < static_cast<unsigned int>(playerStats->getTotalRounds()))
        {
            playerScore = static_cast<unsigned int>(playerStats->getRoundStats(playerRound)->mPoints);
            for (PlayerStatsMap::iterator it = allPlayerStats->begin();
                 it != allPlayerStats->end(); ++it)
            {
                PlayerStats* opponent = it->second;
                if (*(StatsUID*)opponent != *(StatsUID*)playerStats && opponent->hasRoundStats(round))
                {
                    PlayerStats::RoundStats* opponentStats = opponent->getRoundStats(round);
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
