#include "game/core/DamageEvent.hpp"

#include "game/core/Scoreboard.hpp"

#include "game/core/StatsUID.hpp"

DamageEvent::DamageEvent(const DamageEvent& other)
    : mVictim(other.mVictim)
    , mAttacker(other.mAttacker)
    , mAttackerEntity(other.mAttackerEntity)
    , mDistance(other.mDistance)
    , mFatal(other.mFatal)
    , mDamageSource(other.mDamageSource)
    , mDamage(other.mDamage)
    , mWeapon(other.mWeapon)
    , mInWater(other.mInWater)
    , mFallDistance(other.mFallDistance)
    , mFallCount(other.mFallCount)
    , mTime(other.mTime)
{
}
