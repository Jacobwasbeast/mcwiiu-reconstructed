#include "game/core/FallKillStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

FallKillStatTracker::FallKillStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mValue(0)
{
}

FallKillStatTracker::~FallKillStatTracker()
{
}

void FallKillStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mVictim) && damageEvent->mFatal && damageEvent->mFallDistance > 1.5f)
    {
        mValue += static_cast<int>(damageEvent->mFallDistance);
    }
}
