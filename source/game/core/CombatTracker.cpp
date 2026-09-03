#include "game/core/CombatTracker.hpp"

#include "game/core/CombatEntry.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"

CombatTracker::CombatTracker(LivingEntity* mob)
    : mMob(mob), mLastDamageTime(0), mCombatStartTime(0), mCombatEndTime(0),
      mTakingDamage(false), mInCombat(false), mPreparedStatus(0)
{
}

CombatTracker::~CombatTracker()
{
    std::vector<CombatEntry*>::iterator it;
    for (it = mEntries.begin(); it != mEntries.end(); ++it)
        delete *it;
    mEntries.clear();
}

void CombatTracker::resetPreparedStatus()
{
    mPreparedStatus = 0;
}

void CombatTracker::prepareForDamage()
{
    resetPreparedStatus();
    if (mMob == 0)
        return;
    if (mMob->onLadder())
        mPreparedStatus = eLOCATION_CLIMBING;
    else if (mMob->isInWater())
        mPreparedStatus = eLOCATION_WATER;
}

void CombatTracker::recheckStatus()
{
    if (!mInCombat)
        return;
    const int timeout = mTakingDamage ? 300 : 100;
    if (mMob == 0 || !mMob->isAlive() || mMob->mTickCount - mLastDamageTime > timeout)
    {
        if (mTakingDamage && mMob != 0)
            mMob->onLeaveCombat();
        mCombatEndTime = mMob != 0 ? mMob->mTickCount : mCombatEndTime;
        mTakingDamage = false;
        mInCombat = false;
        if (mMob != 0 && mMob->isAlive())
        {
            std::vector<CombatEntry*>::iterator it;
            for (it = mEntries.begin(); it != mEntries.end(); ++it)
                delete *it;
            mEntries.clear();
        }
    }
}

CombatEntry* CombatTracker::GetKnockoffEntry()
{
    if (mEntries.size() < 2)
        return 0;
    CombatEntry* latest = mEntries.back();
    if (latest == 0 || latest->getSource() == 0)
        return 0;
    const int latestTime = latest->getTime();
    int i;
    for (i = (int)mEntries.size() - 2; i >= 0; --i)
    {
        CombatEntry* entry = mEntries[i];
        if (entry != 0 && latestTime - entry->getTime() < 60 && entry->isCombatRelated())
            return entry;
    }
    return 0;
}

CombatTracker::eLOCATION CombatTracker::getFallLocation(CombatEntry* entry)
{
    return entry != 0 ? entry->getLocation() : eLOCATION_NONE;
}

boost::shared_ptr<ClientboundChatPacket> CombatTracker::getDeathMessagePacket()
{
    if (mEntries.empty() || mEntries.back() == 0 || mEntries.back()->getSource() == 0)
        return boost::shared_ptr<ClientboundChatPacket>();
    boost::shared_ptr<Entity> entity = mMob != 0 ? mMob->shared_from_this() : boost::shared_ptr<Entity>();
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    return mEntries.back()->getSource()->getDeathMessagePacket(living);
}

void CombatTracker::recordDamage(const DamageSource* source, float damage, float fallDistance)
{
    if (mMob == 0)
        return;
    const int time = mMob->mTickCount;
    if (!mInCombat)
    {
        mInCombat = true;
        mCombatStartTime = time;
    }
    mTakingDamage = true;
    mLastDamageTime = time;
    CombatEntry* entry = new CombatEntry(source, time, mMob->getHealth(), damage,
                                         static_cast<eLOCATION>(mPreparedStatus), fallDistance);
    mEntries.push_back(entry);
    resetPreparedStatus();
}

boost::shared_ptr<LivingEntity> CombatTracker::getKiller()
{
    int i;
    for (i = (int)mEntries.size() - 1; i >= 0; --i)
    {
        if (mEntries[i] != 0)
        {
            boost::shared_ptr<Entity> attacker = mEntries[i]->getAttacker();
            if (attacker)
                return boost::dynamic_pointer_cast<LivingEntity>(attacker);
        }
    }
    return boost::shared_ptr<LivingEntity>();
}

int CombatTracker::getCombatDuration()
{
    if (!mInCombat)
        return mCombatEndTime - mCombatStartTime;
    return mMob != 0 ? mMob->mTickCount - mCombatStartTime : 0;
}

LivingEntity* CombatTracker::getMob()
{
    return mMob;
}

bool CombatTracker::isTakingDamage()
{
    return mTakingDamage;
}

bool CombatTracker::isInCombat()
{
    return mInCombat;
}

CombatEntry* CombatTracker::getLastEntry()
{
    return mEntries.empty() ? 0 : mEntries.back();
}
