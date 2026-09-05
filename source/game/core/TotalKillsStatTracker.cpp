#include "game/core/TotalKillsStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

TotalKillsStatTracker::TotalKillsStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mKills(0)
{
}

TotalKillsStatTracker::~TotalKillsStatTracker()
{
}

void TotalKillsStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mFatal)
    {
        ++mKills;
    }
}
