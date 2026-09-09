#ifndef GAME_CORE_TOTALKILLSSTATTRACKER_HPP
#define GAME_CORE_TOTALKILLSSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;
class MostKillsMedal;
class LeastKillsMedal;

class TotalKillsStatTracker : public UntypedStatTracker
{
public:
    TotalKillsStatTracker(const StatsUID& statsUID);
    virtual ~TotalKillsStatTracker();
    virtual void update(const DamageEvent* damageEvent);

private:
    friend class MostKillsMedal;
    friend class LeastKillsMedal;
    int mKills;
};

#endif
