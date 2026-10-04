#include "game/world/generation/MaxSnowballHitsWithSnowballOnPlayerStatTracker.hpp"

#include "game/core/DamageEvent.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"

MaxSnowballHitsWithSnowballOnPlayerStatTracker::MaxSnowballHitsWithSnowballOnPlayerStatTracker(
    const StatsUID& statsUID)
    : StatTracker<unsigned int>(statsUID, 0)
{
    mSnowballHits[mPlayerUID] = 0;
}

MaxSnowballHitsWithSnowballOnPlayerStatTracker::~MaxSnowballHitsWithSnowballOnPlayerStatTracker()
{
}

void MaxSnowballHitsWithSnowballOnPlayerStatTracker::update(const DamageEvent* damageEvent)
{
    if (isTracking(damageEvent->mAttacker) && Item::getId(Items::snowball) == damageEvent->mWeapon.mItemId)
    {
        StatsUID emptyPlayerUID;
        if (damageEvent->mVictim != emptyPlayerUID)
        {
            boost::unordered_map<StatsUID, unsigned int, boost::hash<StatsUID>, std::equal_to<StatsUID> >::iterator found = mSnowballHits.find(damageEvent->mVictim);
            if (found == mSnowballHits.end())
            {
                mSnowballHits[damageEvent->mVictim] = 1;
                unsigned int& best = mSnowballHits[mPlayerUID];
                unsigned int& hits = mSnowballHits[damageEvent->mVictim];
                if (hits < best)
                    return;
            }
            else
            {
                ++mSnowballHits[damageEvent->mVictim];
                unsigned int& best = mSnowballHits[mPlayerUID];
                unsigned int& hits = mSnowballHits[damageEvent->mVictim];
                if (hits < best)
                    return;
            }

            mValue = mSnowballHits[damageEvent->mVictim];
            mPlayerUID = damageEvent->mVictim;
        }
    }
}
