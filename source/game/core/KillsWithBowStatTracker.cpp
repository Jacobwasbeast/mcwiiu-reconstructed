#include "game/core/KillsWithBowStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

KillsWithBowStatTracker::KillsWithBowStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mValue(0)
{
}

KillsWithBowStatTracker::~KillsWithBowStatTracker()
{
}

void KillsWithBowStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && damageEvent->mFatal
        && damageEvent->mDamageSource == Scoreboard::DAMAGE_SOURCE_ARROW)
    {
        ++mValue;
    }
}

int KillsWithBowStatTracker::getValue() const
{
    return mValue;
}
