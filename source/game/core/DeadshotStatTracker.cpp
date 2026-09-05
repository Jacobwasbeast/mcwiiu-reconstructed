#include "game/core/DeadshotStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/StatsUID.hpp"

DeadshotStatTracker::DeadshotStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mHit(false)
    , mFallCount(-1)
{
}

DeadshotStatTracker::~DeadshotStatTracker()
{
}

void DeadshotStatTracker::update(const DamageEvent* damageEvent)
{
    if (!Minecraft::InMiniGame(static_cast<EMiniGameId>(2), true))
    {
        return;
    }

    if (isTracking(damageEvent->mAttacker) && damageEvent->mWeapon.mItemId == 332
        && damageEvent->mAttacker != StatsUID::EMPTY_SUID()
        && damageEvent->mFallDistance > 0.0f)
    {
        mFallCount = damageEvent->mFallCount;
    }

    if (!isTracking(damageEvent->mVictim))
    {
        return;
    }

    if (damageEvent->mFatal && damageEvent->mDamageSource == 3 && mFallCount > 0)
    {
        mHit = true;
        return;
    }

    if (damageEvent->mFallCount == mFallCount && damageEvent->mFallDistance != 0.0f)
    {
        return;
    }

    mFallCount = -1;
}
