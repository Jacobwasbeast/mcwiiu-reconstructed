#ifndef GAME_CORE_TOTALTHERMALSENTEREDCONDITION_HPP
#define GAME_CORE_TOTALTHERMALSENTEREDCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class TotalThermalsEnteredCondition : public MultiConditionMedal::MultiCondition
{
public:
    TotalThermalsEnteredCondition(float target, GameMedal::EOperator op, bool bestOnly);
    virtual ~TotalThermalsEnteredCondition();

    virtual double getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
