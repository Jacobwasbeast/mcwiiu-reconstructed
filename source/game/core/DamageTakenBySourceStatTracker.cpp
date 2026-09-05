#include "game/core/DamageTakenBySourceStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

DamageTakenBySourceStatTracker::DamageTakenBySourceStatTracker(
    const StatsUID& statsUID, Scoreboard::EDamageSource source)
    : UntypedStatTracker(statsUID)
    , mValue(0.0f)
    , mDamageSource(source)
{
}

DamageTakenBySourceStatTracker::~DamageTakenBySourceStatTracker()
{
}

void DamageTakenBySourceStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(mStatsUID) && damageEvent->mDamageSource == mDamageSource)
    {
        mValue += damageEvent->mDamage;
    }
}
