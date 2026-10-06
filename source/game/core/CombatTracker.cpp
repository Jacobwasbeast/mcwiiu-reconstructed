#include "game/core/CombatTracker.hpp"

#include "game/core/CombatEntry.hpp"
#include "game/core/DamageSource.hpp"
#include "game/network/ClientboundChatPacket.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/core/EntityType.hpp"
#include "game/core/GameMode.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

CombatTracker::CombatTracker(LivingEntity* mob)
    : mMob(mob), mLastDamageTime(0), mCombatStartTime(0), mCombatEndTime(0),
      mInCombat(false), mTakingDamage(false), mPreparedStatus(0)
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
    if (mMob->onLadder())
    {
        Block* block = mMob->mLevel->getBlockState(BlockPos(mMob->mX, mMob->mBoundingBox->mMinY, mMob->mZ))->getBlock();
        if (block == Blocks::ladder)
            mPreparedStatus = eLOCATION_LADDER;
        else if (block == Blocks::vine)
            mPreparedStatus = eLOCATION_VINES;
    }
    else if (mMob->isInWater())
    {
        mPreparedStatus = eLOCATION_WATER;
    }
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
    int count = (int)mEntries.size();
    if (count < 2)
        return NULL;

    const DamageSource* lastSource = mEntries[count - 1]->getSource();
    if (!lastSource->equals(DamageSource::OUT_OF_WORLD) && !lastSource->equals(DamageSource::FALL)
        && !lastSource->equals(DamageSource::CREATIVE_PLAYER))
        return NULL;

    for (int i = count - 2; i >= 0; --i)
    {
        CombatEntry* entry = mEntries[i];
        const DamageSource* source = entry->getSource();
        if (source->equals(DamageSource::OUT_OF_WORLD) || source->equals(DamageSource::CREATIVE_PLAYER))
            continue;
        if (lastSource->equals(DamageSource::FALL) && source->equals(DamageSource::FALL))
            return NULL;
        if (entry != NULL && mEntries[i + 1]->getTime() - entry->getTime() < KNOCKOFF_TICKS)
            return entry;
        return NULL;
    }
    return NULL;
}

CombatTracker::eLOCATION CombatTracker::getFallLocation(CombatEntry* entry)
{
    return static_cast<eLOCATION>(entry->getLocation());
}

boost::shared_ptr<ClientboundChatPacket> CombatTracker::getDeathMessagePacket()
{
    if (mEntries.empty())
        return boost::shared_ptr<ClientboundChatPacket>(
            new ClientboundChatPacket(mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_Text, -1));

    CombatEntry* knockoff = GetKnockoffEntry();
    CombatEntry* last = mEntries.back();
    boost::shared_ptr<ClientboundChatPacket> result;
    boost::shared_ptr<Entity> killer = last->getSource()->getEntity();
    boost::shared_ptr<Entity> attacker;
    const DamageSource* source = last->getSource();

    if (knockoff == NULL)
    {
        if (source->equals(DamageSource::CREATIVE_PLAYER))
        {
            ClientboundChatPacket* packet =
                new ClientboundChatPacket(ClientboundChatPacket::eChatPacketMessage_DeathCustom, -1);
            packet->strings.push_back(mMob->getNetworkName());
            packet->strings.push_back(mMob->mDeathMessage);
            packet->ints.push_back(mMob->mDeathMessageId);
            result = boost::shared_ptr<ClientboundChatPacket>(packet);
            return result;
        }
    }
    else if (source->equals(DamageSource::FALL) || source->equals(DamageSource::OUT_OF_WORLD)
             || source->equals(DamageSource::CREATIVE_PLAYER))
    {
        attacker = knockoff->getSource()->getEntity();
        const DamageSource* knockoffSource = knockoff->getSource();
        if (knockoffSource->equals(DamageSource::FALL) || knockoffSource->equals(DamageSource::OUT_OF_WORLD)
            || knockoffSource->equals(DamageSource::CREATIVE_PLAYER))
        {
            ClientboundChatPacket::EChatPacketMessage message = ClientboundChatPacket::eChatPacketMessage_DeathFellHigh;
            switch (getFallLocation(knockoff))
            {
            case eLOCATION_LADDER:
                message = ClientboundChatPacket::eChatPacketMessage_DeathFellLadder;
                break;
            case eLOCATION_VINES:
                message = ClientboundChatPacket::eChatPacketMessage_DeathFellVines;
                break;
            case eLOCATION_WATER:
                message = ClientboundChatPacket::eChatPacketMessage_DeathFellWater;
                break;
            default:
                break;
            }
            result = boost::shared_ptr<ClientboundChatPacket>(
                new ClientboundChatPacket(mMob->getNetworkName(), message, -1));
        }
        else if (attacker != NULL && (killer == NULL || attacker != killer))
        {
            not_null_ptr<ItemInstance> item = (attacker->GetType() & EntityType::LIVING_ENTITY) == EntityType::LIVING_ENTITY
                ? boost::static_pointer_cast<LivingEntity>(attacker)->getMainHandItem()
                : ItemInstance::EMPTY;
            if (!item->isEmpty() && item->hasCustomHoverName())
            {
                result = boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                    mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_DeathFellDoomedByItem, attacker,
                    attacker->getNetworkName(), item->getHoverNameUnfiltered()));
            }
            else
            {
                result = boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                    mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_DeathFellDoomedBy, attacker,
                    attacker->getNetworkName()));
            }
        }
        else if (killer != NULL)
        {
            not_null_ptr<ItemInstance> item = eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, killer->GetType())
                ? boost::static_pointer_cast<LivingEntity>(killer)->getMainHandItem()
                : ItemInstance::EMPTY;
            if (!item->isEmpty() && item->hasCustomHoverName())
            {
                result = boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                    mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_DeathFellFinishedItem, killer,
                    killer->getNetworkName(), item->getHoverNameUnfiltered()));
            }
            else
            {
                result = boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                    mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_DeathFellFinished, killer,
                    killer->getNetworkName()));
            }
        }
        else
        {
            result = boost::shared_ptr<ClientboundChatPacket>(new ClientboundChatPacket(
                mMob->getNetworkName(), ClientboundChatPacket::eChatPacketMessage_DeathFellDoomed, -1));
        }
        return result;
    }

    result = source->getDeathMessagePacket(boost::dynamic_pointer_cast<LivingEntity>(mMob->shared_from_this()));
    return result;
}

void CombatTracker::recordDamage(const DamageSource* source, float healthBeforeDamage, float damage)
{
    recheckStatus();
    prepareForDamage();

    CombatEntry* entry = new CombatEntry(source, mMob->mTickCount, healthBeforeDamage, damage,
                                         static_cast<eLOCATION>(mPreparedStatus), mMob->mFallDistance);

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
        mMob->onEnterCombat();
    }

    if ((mMob->GetType() & EntityType::PLAYER) == EntityType::PLAYER)
    {
        Player* player = static_cast<Player*>(mMob);
        bool killed = healthBeforeDamage - damage <= 0.0f;
        boost::shared_ptr<ClientboundChatPacket> deathMessage;
        if (killed)
        {
            deathMessage = getDeathMessagePacket();
        }
        GameMode* gameMode = player->GetGameMode();
        if (gameMode != NULL)
        {
            gameMode->RecordDamage(mMob->shared_from_this(), entry, deathMessage);
        }
        if (killed)
        {
            gameMode->RecordDeath(mMob->shared_from_this());
        }
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
            if (eTYPE_DERIVED_FROM(EntityType::PLAYER, entry->getSource()->getEntity()->GetType()) &&
                (!bestPlayerKiller || entry->getDamage() > bestPlayerDamage))
            {
                bestPlayerDamage = entry->getDamage();
                bestPlayerKiller = boost::dynamic_pointer_cast<Player>(entry->getSource()->getEntity());
            }
        }

        source = entry->getSource();
        if (source != 0 && source->getEntity())
        {
            if (eTYPE_DERIVED_FROM(EntityType::LIVING_ENTITY, entry->getSource()->getEntity()->GetType()) &&
                (!bestLivingKiller || entry->getDamage() > bestLivingDamage))
            {
                bestLivingDamage = entry->getDamage();
                bestLivingKiller = boost::dynamic_pointer_cast<LivingEntity>(entry->getSource()->getEntity());
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
    return mMob->mTickCount - mCombatStartTime;
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
