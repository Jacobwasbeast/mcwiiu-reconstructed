#ifndef GAME_WORLD_GENERATION_LASTPLAYERFATALDAMAGESTATTRACKER_HPP
#define GAME_WORLD_GENERATION_LASTPLAYERFATALDAMAGESTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class LastPlayerFatalDamageStatTracker : public UntypedStatTracker
{
public:
    LastPlayerFatalDamageStatTracker(const StatsUID& statsUID);
    virtual ~LastPlayerFatalDamageStatTracker();

    virtual void update(const DamageEvent* damageEvent);

private:
    DamageEvent* mLastFatalDamage;
};

#endif
