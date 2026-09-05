#include "game/core/DamageReceivedStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

DamageReceivedStatTracker::DamageReceivedStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mValue(0.0f)
{
}

DamageReceivedStatTracker::~DamageReceivedStatTracker()
{
}

void DamageReceivedStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(mStatsUID))
    {
        mValue += damageEvent->mDamage;
    }
}
