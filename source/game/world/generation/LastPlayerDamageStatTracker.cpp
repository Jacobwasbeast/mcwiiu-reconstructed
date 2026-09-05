#include "game/world/generation/LastPlayerDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

LastPlayerDamageStatTracker::LastPlayerDamageStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mLastDamage(0)
{
}

LastPlayerDamageStatTracker::~LastPlayerDamageStatTracker()
{
    delete mLastDamage;
}

void LastPlayerDamageStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim))
    {
        delete mLastDamage;
        mLastDamage = new DamageEvent(*damageEvent);
    }
}
