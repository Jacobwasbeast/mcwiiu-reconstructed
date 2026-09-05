#ifndef GAME_CORE_DAMAGETAKENBYSOURCESTATTRACKER_HPP
#define GAME_CORE_DAMAGETAKENBYSOURCESTATTRACKER_HPP

#include "game/core/DamageEvent.hpp"
#include "game/core/UntypedStatTracker.hpp"

class DamageEvent;

class DamageTakenBySourceStatTracker : public UntypedStatTracker
{
public:
    DamageTakenBySourceStatTracker(const StatsUID& statsUID, Scoreboard::EDamageSource source);
    virtual ~DamageTakenBySourceStatTracker();
    virtual void update(const DamageEvent* damageEvent);

private:
    float mValue;
    Scoreboard::EDamageSource mDamageSource;
};

#endif
