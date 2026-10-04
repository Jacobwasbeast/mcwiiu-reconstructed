#include "game/core/WeaponDamageStatTracker.hpp"

#include "game/core/DamageEvent.hpp"

WeaponDamageStatTracker::WeaponDamageStatTracker(const StatsUID& statsUID,
                                                 ETrackingType trackingType)
    : StatTracker<Scoreboard::Weapon>(statsUID, Scoreboard::Weapon(0, NULL))
    , mDamageByWeapon()
    , mTrackingType(trackingType)
{
    mDamageByWeapon[mValue.GetKey()] = 0.0f;
}

WeaponDamageStatTracker::~WeaponDamageStatTracker()
{
}

void WeaponDamageStatTracker::update(const DamageEvent* damageEvent)
{
    const bool trackingDamage =
        (mTrackingType == TRACKING_DEALT && isTracking(damageEvent->mVictim))
        || (mTrackingType == TRACKING_RECEIVED && isTracking(damageEvent->mAttacker));
    if (!trackingDamage)
    {
        return;
    }

    const Scoreboard::Weapon weapon = damageEvent->mWeapon;
    const unsigned int weaponKey = weapon.GetKey();
    if (!weapon.HasItem())
    {
        return;
    }

    const float damage = damageEvent->mDamage;
    std::map<unsigned int, float>::iterator existing = mDamageByWeapon.find(weaponKey);
    if (existing != mDamageByWeapon.end())
    {
        mDamageByWeapon[weaponKey] += damage;
    }
    else
    {
        mDamageByWeapon[weaponKey] = damage;
    }

    const unsigned int trackedKey = mValue.GetKey();
    float& trackedDamage = mDamageByWeapon[trackedKey];
    float& weaponDamage = mDamageByWeapon[weaponKey];
    if (weaponDamage >= trackedDamage)
    {
        mValue = weapon;
    }
}
