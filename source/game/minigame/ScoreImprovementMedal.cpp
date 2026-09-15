#include "game/minigame/ScoreImprovementMedal.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/generation/PlayerStats.hpp"

ScoreImprovementMedal::~ScoreImprovementMedal() = default;

double ScoreImprovementMedal::GetScore(PlayerStats* playerStats) const
{
    Minecraft* minecraft = Minecraft::GetInstance();
    int finalRound = minecraft->GetMiniGame()->GetFinalRoundIndex();

    if (minecraft->GetClientMasterGameMode() != NULL)
    {
        const MiniGameDef* miniGame = CommonMasterGameMode::GetMiniGame();
        if (miniGame->ArePointsActive())
        {
            return static_cast<double>(static_cast<float>(
                (playerStats->getRoundStats(finalRound)->mPoints
                 + playerStats->getRoundStats(finalRound)->mBonusPoints)
                - (playerStats->getRoundStats(0)->mPoints
                   + playerStats->getRoundStats(0)->mBonusPoints)));
        }

        miniGame = CommonMasterGameMode::GetMiniGame();
        if (miniGame->ArePowerupsActive())
        {
            int firstScore = 0;
            unsigned int round = 0;
            while (round < static_cast<unsigned int>(playerStats->getTotalRounds()))
            {
                PlayerStats::RoundStats* stats = playerStats->getRoundStats(round);
                if (stats->mReachedEnd)
                {
                    firstScore = stats->mLifeSpan * 2;
                    break;
                }
                ++round;
            }

            if (playerStats->getRoundStats(finalRound)->mReachedEnd)
            {
                return static_cast<double>(static_cast<float>(
                    firstScore - playerStats->getRoundStats(finalRound)->mLifeSpan));
            }
        }
    }

    return 0.0;
}
