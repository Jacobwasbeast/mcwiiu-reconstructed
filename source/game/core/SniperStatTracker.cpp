#include "game/core/SniperStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"

SniperStatTracker::SniperStatTracker(const StatsUID& statsUID)
    : UntypedStatTracker(statsUID)
    , mValue(0.0)
{
}

SniperStatTracker::~SniperStatTracker()
{
}

void SniperStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker)
        && Item::byId(damageEvent->mWeapon.mItemId) != NULL)
    {
        if (mValue < static_cast<double>(damageEvent->mDistance))
        {
            mValue = damageEvent->mDistance;
        }
    }
}
