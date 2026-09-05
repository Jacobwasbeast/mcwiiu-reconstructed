#ifndef GAME_CORE_SNIPERSTATTRACKER_HPP
#define GAME_CORE_SNIPERSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class SniperStatTracker : public UntypedStatTracker
{
public:
    SniperStatTracker(const StatsUID& statsUID);
    virtual ~SniperStatTracker();

    virtual void update(const DamageEvent* damageEvent);

private:
    double mValue;
};

#endif
