#ifndef GAME_WORLD_GENERATION_LASTPLAYERDAMAGESTATTRACKER_HPP
#define GAME_WORLD_GENERATION_LASTPLAYERDAMAGESTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class LastPlayerDamageStatTracker : public UntypedStatTracker
{
public:
    LastPlayerDamageStatTracker(const StatsUID& statsUID);
    virtual ~LastPlayerDamageStatTracker();

    virtual void update(const DamageEvent* damageEvent);

private:
    DamageEvent* mLastDamage;
};

#endif
