#include "game/core/SniperStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

SniperStatTracker::SniperStatTracker(const StatsUID& statsUID)
    : StatTracker<double>(statsUID, 0.0)
{
}

SniperStatTracker::~SniperStatTracker()
{
}

void SniperStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker)
        && damageEvent->mWeapon.GetItem() == Items::bow)
    {
        mValue = mValue < damageEvent->mDistance ? damageEvent->mDistance : mValue;
    }
}
