#ifndef GAME_CORE_DAMAGERECEIVEDSTATTRACKER_HPP
#define GAME_CORE_DAMAGERECEIVEDSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class DamageReceivedStatTracker : public UntypedStatTracker
{
public:
    DamageReceivedStatTracker(const StatsUID& statsUID);
    virtual ~DamageReceivedStatTracker();

    virtual void update(const DamageEvent* damageEvent);

protected:
    float mValue;
};

#endif
