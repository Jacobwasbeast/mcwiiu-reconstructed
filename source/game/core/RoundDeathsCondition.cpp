#include "game/core/RoundDeathsCondition.hpp"

#include "game/world/generation/PlayerStats.hpp"

RoundDeathsCondition::~RoundDeathsCondition()
{
}

double RoundDeathsCondition::getScore(const GameMedal* medal, PlayerStats* playerStats) const
{
    switch (mOperator)
    {
    case GameMedal::OPERATOR_AT_LEAST:
        return static_cast<double>(static_cast<float>(playerStats->getDeathsInAnyRound(0x7fffffff)));

    case GameMedal::OPERATOR_AT_MOST:
        return static_cast<double>(static_cast<float>(playerStats->getDeathsInAnyRound(0)));

    case GameMedal::OPERATOR_EQUAL:
        return static_cast<double>(static_cast<float>(playerStats->getDeathsInAnyRound(static_cast<int>(mTarget))));
    }

    return -1.0f;
}
