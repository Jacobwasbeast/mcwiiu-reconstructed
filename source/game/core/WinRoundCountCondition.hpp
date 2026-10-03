#ifndef GAME_CORE_WINROUNDCOUNTCONDITION_HPP
#define GAME_CORE_WINROUNDCOUNTCONDITION_HPP

#include "game/minigame/MultiConditionMedal.hpp"

class WinRoundCountCondition : public MultiConditionMedal::MultiCondition
{
public:
    WinRoundCountCondition(float target, GameMedal::EOperator op, bool bestOnly);
    virtual ~WinRoundCountCondition();

    virtual double getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
