#ifndef GAME_CORE_TOTALKILLSSTATTRACKER_HPP
#define GAME_CORE_TOTALKILLSSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class TotalKillsStatTracker : public UntypedStatTracker
{
public:
    TotalKillsStatTracker(const StatsUID& statsUID);
    virtual ~TotalKillsStatTracker();
    virtual void update(const DamageEvent* damageEvent);

private:
    int mKills;
};

#endif
