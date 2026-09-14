#include "game/core/CombatTracker.hpp"

#include "game/core/CombatEntry.hpp"
#include "game/core/DamageSource.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"

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
    if (mEntries.empty())
        return boost::shared_ptr<ClientboundChatPacket>(
            new ClientboundChatPacket(mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_Text, -1));
    if (mEntries.back() == 0 || mEntries.back()->getSource() == 0)
        return boost::shared_ptr<ClientboundChatPacket>();
    boost::shared_ptr<Entity> entity = mMob != 0 ? mMob->shared_from_this() : boost::shared_ptr<Entity>();
    boost::shared_ptr<LivingEntity> living = boost::dynamic_pointer_cast<LivingEntity>(entity);
    return mEntries.back()->getSource()->getDeathMessagePacket(living);
}

void CombatTracker::recordDamage(const DamageSource* source, float damage, float fallDistance)
{
    recheckStatus();
    prepareForDamage();

    CombatEntry* entry = new CombatEntry(source, mMob->mTickCount, mMob->getHealth(), damage,
                                         static_cast<eLOCATION>(mPreparedStatus), fallDistance);

    if (mEntries.size() > 3)
    {
        delete mEntries.front();
        mEntries.erase(mEntries.begin());
    }
    mEntries.push_back(entry);

    mTakingDamage = true;
    mLastDamageTime = mMob->mTickCount;

    if (entry->isCombatRelated() && !mInCombat && mMob->isAlive())
    {
        mCombatStartTime = mMob->mTickCount;
        mCombatEndTime = mMob->mTickCount;
        mInCombat = true;
    }
}

boost::shared_ptr<LivingEntity> CombatTracker::getKiller()
{
    boost::shared_ptr<Player> bestPlayerKiller;
    float bestPlayerDamage = 0.0f;
    boost::shared_ptr<LivingEntity> bestLivingKiller;
    float bestLivingDamage = 0.0f;

    std::vector<CombatEntry*>::iterator it;
    for (it = mEntries.begin(); it != mEntries.end(); ++it)
    {
        CombatEntry* entry = *it;
        const DamageSource* source = entry->getSource();
        if (source != 0 && source->getEntity())
        {
            if (boost::dynamic_pointer_cast<Player>(source->getEntity()) &&
                (!bestPlayerKiller || entry->getDamage() > bestPlayerDamage))
            {
                bestPlayerDamage = entry->getDamage();
                bestPlayerKiller = boost::dynamic_pointer_cast<Player>(source->getEntity());
            }
        }

        source = entry->getSource();
        if (source != 0 && source->getEntity())
        {
            if (boost::dynamic_pointer_cast<LivingEntity>(source->getEntity()) &&
                (!bestLivingKiller || entry->getDamage() > bestLivingDamage))
            {
                bestLivingDamage = entry->getDamage();
                bestLivingKiller = boost::dynamic_pointer_cast<LivingEntity>(source->getEntity());
            }
        }
    }

    if (!bestPlayerKiller || bestPlayerDamage < bestLivingDamage / 3.0f)
        return bestLivingKiller;
    return bestPlayerKiller;
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
