#include "game/core/DamageDealtStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

DamageDealtStatTracker::DamageDealtStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mDamage(0.0f)
{
}

DamageDealtStatTracker::~DamageDealtStatTracker()
{
}

void DamageDealtStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && !isTracking(damageEvent->mVictim))
    {
        mDamage += damageEvent->mDamage;
    }
}
