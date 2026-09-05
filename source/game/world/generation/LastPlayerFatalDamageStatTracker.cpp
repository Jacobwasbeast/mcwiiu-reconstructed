#include "game/world/generation/LastPlayerFatalDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

LastPlayerFatalDamageStatTracker::LastPlayerFatalDamageStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mLastFatalDamage(0)
{
}

LastPlayerFatalDamageStatTracker::~LastPlayerFatalDamageStatTracker()
{
    delete mLastFatalDamage;
}

void LastPlayerFatalDamageStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim) && damageEvent->mFatal)
    {
        delete mLastFatalDamage;
        mLastFatalDamage = new DamageEvent(*damageEvent);
    }
}
