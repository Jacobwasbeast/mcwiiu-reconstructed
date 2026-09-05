#ifndef GAME_CORE_FALLKILLSTATTRACKER_HPP
#define GAME_CORE_FALLKILLSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class FallKillStatTracker : public UntypedStatTracker
{
public:
    FallKillStatTracker(const StatsUID& statsUID);
    virtual ~FallKillStatTracker();
    virtual void update(const DamageEvent* damageEvent);

private:
    int mValue;
};

#endif
