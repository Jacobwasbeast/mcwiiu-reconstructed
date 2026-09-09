#ifndef GAME_CORE_KILLSWITHBOWSTATTRACKER_HPP
#define GAME_CORE_KILLSWITHBOWSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class KillsWithBowStatTracker : public UntypedStatTracker
{
public:
    KillsWithBowStatTracker(const StatsUID& statsUID);
    virtual ~KillsWithBowStatTracker();
    virtual void update(const DamageEvent* damageEvent);
    int getValue() const;

private:
    int mValue;
};

#endif
