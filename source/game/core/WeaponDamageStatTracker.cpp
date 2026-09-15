#include "game/core/WeaponDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

WeaponDamageStatTracker::WeaponDamageStatTracker(const StatsUID& statsUID,
                                                 ETrackingType trackingType)
    : UntypedStatTracker(statsUID)
    , mWeapon(0, NULL)
    , mDamageByWeapon()
    , mTrackingType(trackingType)
{
    mDamageByWeapon[mWeapon.GetKey()] = 0.0f;
}

WeaponDamageStatTracker::~WeaponDamageStatTracker()
{
}

void WeaponDamageStatTracker::update(const DamageEvent* damageEvent)
{
    const bool trackingDamage = mTrackingType == TRACKING_DEALT
        ? isTracking(damageEvent->mAttacker)
        : isTracking(damageEvent->mVictim);
    if (!trackingDamage || !damageEvent->mWeapon.HasItem())
    {
        return;
    }

    const unsigned int weaponKey = damageEvent->mWeapon.GetKey();
    std::map<unsigned int, float>::iterator existing = mDamageByWeapon.find(weaponKey);
    if (existing != mDamageByWeapon.end())
    {
        existing->second += damageEvent->mDamage;
    }
    else
    {
        mDamageByWeapon[weaponKey] = damageEvent->mDamage;
    }

    const unsigned int trackedKey = mWeapon.GetKey();
    if (mDamageByWeapon[weaponKey] >= mDamageByWeapon[trackedKey])
    {
        mWeapon = damageEvent->mWeapon;
    }
}
