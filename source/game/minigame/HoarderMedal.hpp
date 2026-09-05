#ifndef GAME_MINIGAME_HOARDERMEDAL_HPP
#define GAME_MINIGAME_HOARDERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class HoarderMedal : public GameMedal
{
public:
    HoarderMedal(int name, float target, unsigned int weight, GameMedal::EOperator op, bool bestOnly, bool canMultiAward)
        : GameMedal(name, target, weight, op, bestOnly, canMultiAward)
    {
    }

    virtual ~HoarderMedal();

    virtual double GetScore(PlayerStats* playerStats) const;
};

#endif
