#include "game/core/WeaponDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

WeaponDamageStatTracker::WeaponDamageStatTracker(const StatsUID& statsUID,
                                                 ETrackingType trackingType)
    : UntypedStatTracker(statsUID)
{
    mWeapon = Scoreboard::Weapon(0, NULL);
    mTrackingType = trackingType;
    mDamageByWeapon[mWeapon.GetKey()] = 0.0f;
}

WeaponDamageStatTracker::~WeaponDamageStatTracker()
{
}

void WeaponDamageStatTracker::update(const DamageEvent* damageEvent)
{
    if (mTrackingType == TRACKING_DEALT)
    {
        if (isTracking(damageEvent->mVictim))
        {
            return;
        }
    }
    else if (!isTracking(damageEvent->mAttacker))
    {
        return;
    }

    Scoreboard::Weapon weapon = damageEvent->mWeapon;

    if (!weapon.HasItem())
    {
        return;
    }

    unsigned int key = weapon.GetKey();
    float damage = damageEvent->mDamage;

    if (mDamageByWeapon.find(key) == mDamageByWeapon.end())
    {
        mDamageByWeapon[key] = damage;
    }
    else
    {
        mDamageByWeapon[key] += damage;
    }

    if (mDamageByWeapon[key] >= mDamageByWeapon[mWeapon.GetKey()])
    {
        mWeapon = weapon;
    }
}
