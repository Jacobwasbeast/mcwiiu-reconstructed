#ifndef GAME_CORE_ROUNDDEATHSCONDITION_HPP
#define GAME_CORE_ROUNDDEATHSCONDITION_HPP

#include "game/minigame/GameMedal.hpp"

class RoundDeathsCondition : public GameMedal::Condition
{
public:
    RoundDeathsCondition(float target, GameMedal::EOperator op, bool bestOnly);

    virtual ~RoundDeathsCondition();
    virtual double getScore(const GameMedal* medal, PlayerStats* playerStats) const;
};

#endif
