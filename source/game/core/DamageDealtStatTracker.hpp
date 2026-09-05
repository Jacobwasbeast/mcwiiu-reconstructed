#ifndef GAME_CORE_DAMAGEDEALTSTATTRACKER_HPP
#define GAME_CORE_DAMAGEDEALTSTATTRACKER_HPP

#include "game/core/UntypedStatTracker.hpp"

class DamageDealtStatTracker : public UntypedStatTracker
{
public:
    DamageDealtStatTracker(const StatsUID& statsUID);
    virtual ~DamageDealtStatTracker();

    virtual void update(const DamageEvent* damageEvent);

private:
    float mDamage;
};

#endif
