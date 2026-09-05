#ifndef GAME_MINIGAME_HOARDERMEDAL_HPP
#define GAME_MINIGAME_HOARDERMEDAL_HPP

#include "game/minigame/GameMedal.hpp"

class HoarderMedal : public GameMedal
{
public:
    virtual ~HoarderMedal();

    virtual double GetScore(PlayerStats* playerStats) const;
};

#endif
