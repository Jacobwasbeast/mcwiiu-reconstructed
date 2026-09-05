#include "game/core/ReachEndTimeCondition.hpp"

#include <float.h>

#include "game/world/generation/PlayerStats.hpp"

ReachEndTimeCondition::ReachEndTimeCondition(float target, bool reachedEndOnly, GameMedal::EOperator op)
    : MultiConditionMedal::MultiCondition(target, op, true),
      mReachedEndOnly(reachedEndOnly)
{
}

ReachEndTimeCondition::~ReachEndTimeCondition()
{
}

double ReachEndTimeCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    if (playerStats == NULL)
    {
        return FLT_MAX;
    }

    if (mReachedEndOnly)
    {
        return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->getBestLifeSpan(!mBestOnly, false))));
    }

    return static_cast<double>(static_cast<float>(static_cast<unsigned int>(playerStats->getTotalLifeSpan())));
}
