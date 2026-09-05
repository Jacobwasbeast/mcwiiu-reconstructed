#include "game/core/KillsWithBowStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

KillsWithBowStatTracker::KillsWithBowStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mKills(0)
{
}

KillsWithBowStatTracker::~KillsWithBowStatTracker()
{
}

void KillsWithBowStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mFallCount != 0
        && damageEvent->mTime == 11)
    {
        ++mKills;
    }
}
