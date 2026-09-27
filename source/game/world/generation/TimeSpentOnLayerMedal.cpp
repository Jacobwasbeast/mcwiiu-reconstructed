#include "game/world/generation/TimeSpentOnLayerMedal.hpp"

#include "game/core/CommonMasterGameMode.hpp"
#include "game/world/generation/PlayerStats.hpp"

TimeSpentOnLayerMedal::TimeSpentOnLayerMedal(ELayerMedal layer, int name, float target,
    unsigned int weight, GameMedal::EOperator op)
    : GameMedal(name, target, weight, op, true, false)
    , mLayer(layer)
{
}

TimeSpentOnLayerMedal::~TimeSpentOnLayerMedal()
{
}

double TimeSpentOnLayerMedal::GetScore(PlayerStats* playerStats) const
{
    unsigned int total = 0;
    unsigned int layer = 0;

    if (mLayer == LAYER_MEDAL_BOTTOM)
    {
        layer = static_cast<unsigned int>(CommonMasterGameMode::GetMasterOrClient()->GetLayerCount() - 1);
    }

    unsigned int round = 0;
    unsigned int totalRounds = static_cast<unsigned int>(playerStats->getTotalRounds());
    if (totalRounds != 0)
    {
        do
        {
            PlayerStats::RoundStats* roundStats = playerStats->getRoundStats(round);
            if (roundStats->mDeathLayer != PlayerStats::RoundStats::LAYERS_UNUSED && !roundStats->mTimeOnLayer.empty()
                && layer < roundStats->mTimeOnLayer.size())
            {
                total += roundStats->mTimeOnLayer.at(layer);
            }

            ++round;
            totalRounds = static_cast<unsigned int>(playerStats->getTotalRounds());
        }
        while (round < totalRounds);
    }

    return static_cast<float>(static_cast<int>(total));
}
