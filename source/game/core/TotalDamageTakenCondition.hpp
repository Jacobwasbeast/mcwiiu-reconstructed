#ifndef GAME_CORE_TOTALDAMAGETAKENCONDITION_HPP
#define GAME_CORE_TOTALDAMAGETAKENCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class TotalDamageTakenCondition : public MultiConditionMedal::MultiCondition
{
public:
    TotalDamageTakenCondition(float target, GameMedal::EOperator op, bool bestOnly);
    virtual ~TotalDamageTakenCondition();

    virtual double getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
