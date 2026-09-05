#ifndef GAME_CORE_WINROUNDCOUNTCONDITION_HPP
#define GAME_CORE_WINROUNDCOUNTCONDITION_HPP

#include "game/minigame/GameMedal.hpp"

class WinRoundCountCondition : public GameMedal::Condition
{
public:
    WinRoundCountCondition(float target, GameMedal::EOperator op, bool bestOnly);
    virtual ~WinRoundCountCondition();

    virtual double getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
