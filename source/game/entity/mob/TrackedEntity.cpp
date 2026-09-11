#include "game/entity/mob/TrackedEntity.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/core/ExperienceOrb.hpp"
#include "game/core/Minecart.hpp"
#include "game/core/Mth.hpp"
#include "game/core/Painting.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/LeashFenceKnotEntity.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/PathfinderMob.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/entity/mob/FireworksRocketEntity.hpp"
#include "game/entity/projectile/Arrow.hpp"
#include "game/entity/projectile/Fireball.hpp"
#include "game/entity/projectile/FishingHook.hpp"
#include "game/entity/projectile/SpectralArrow.hpp"
#include "game/item/AreaEffectCloud.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemFrame.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/item/ThrownPotion.hpp"
#include "game/network/ClientboundAddEntityPacket.hpp"
#include "game/network/ClientboundAddExperienceOrbPacket.hpp"
#include "game/network/ClientboundAddMobPacket.hpp"
#include "game/network/ClientboundAddPaintingPacket.hpp"
#include "game/network/ClientboundAddPlayerPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/FallingBlockEntity.hpp"
#include "game/core/Direction.hpp"

TrackedEntity::TrackedEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval, int maxRange, bool sendVelocityUpdates)
    : mEntity(entity), mRange(range), mUpdateInterval(updateInterval), mMaxRange(maxRange),
      mXLoc(0), mYLoc(0), mZLoc(0), mXRot(0.0), mYRot(0.0), mZRot(0.0),
      mSendVelocityUpdates(sendVelocityUpdates), mWasRiding(false), mUpdateCounter(0)
{
    mXLoc = EntityTracker::truncate(entity->mX);
    mYLoc = EntityTracker::truncate(entity->mY);
    mZLoc = EntityTracker::truncate(entity->mZ);
    mXRot = entity->mXRot;
    mYRot = entity->mYRot;
    mZRot = entity->mYRot;
}

bool TrackedEntity::canBySeenBy(boost::shared_ptr<ServerPlayer> player)
{
    return true;
}

bool TrackedEntity::isVisibleTo(EntityTracker* tracker, const boost::shared_ptr<ServerPlayer>& player, bool force, bool suppress)
{
    if (!player || !mEntity || mEntity->mRemoved)
    {
        return false;
    }
    if (!force && !canBySeenBy(player))
    {
        return false;
    }
    if (!suppress && mEntity->isInvisibleTo(boost::static_pointer_cast<Player>(player)))
    {
        return false;
    }
    return mEntity->broadcastToPlayer(player);
}

namespace
{
int encodeFireballPower(double power)
{
    double clamped = std::max(-4.0, std::min(4.0, power));
    return (int)((clamped + 4.0) * 65535.0 / 8.0 - 32768.0);
}

boost::shared_ptr<ClientboundAddEntityPacket> makeFireballPacket(
    boost::shared_ptr<Entity> entity, int type, int data, int yRot, int xRot, int x, int y, int z,
    boost::shared_ptr<Fireball> fireball)
{
    boost::shared_ptr<ClientboundAddEntityPacket> packet(
        new ClientboundAddEntityPacket(entity, type, data, yRot, xRot, x, y, z));
    packet->setXa(encodeFireballPower(fireball->mPowerX));
    packet->setYa(encodeFireballPower(fireball->mPowerY));
    packet->setZa(encodeFireballPower(fireball->mPowerZ));
    return packet;
}
}

boost::shared_ptr<Packet> TrackedEntity::getAddEntityPacket()
{
    if (!mEntity)
    {
        return boost::shared_ptr<Packet>();
    }

    const int yRot = (int)mXRot;
    const int xRot = (int)mYRot;
    const int headRot = (int)mZRot;
    unsigned long long entityType = mEntity->GetType();
    unsigned int entityTypeHigh = (unsigned int)(entityType >> 32);
    unsigned int entityTypeLow = (unsigned int)entityType;

    if (entityType == ENTITY_TYPE_SERVER_PLAYER)
    {
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(mEntity);
        return boost::shared_ptr<Packet>(
            new ClientboundAddPlayerPacket(player, player->mXuid, player->m_0x798, mXLoc, mYLoc, mZLoc,
                                           yRot, xRot, headRot));
    }

    boost::shared_ptr<PathfinderMob> creature = boost::dynamic_pointer_cast<PathfinderMob>(mEntity);
    if (creature || entityType == ENTITY_TYPE_SHULKER)
    {
        int yHeadRot = Mth::floor(mEntity->getYHeadRot() * 256.0f / 360.0f);
        return boost::shared_ptr<Packet>(
            new ClientboundAddMobPacket(boost::static_pointer_cast<LivingEntity>(mEntity), yRot, xRot, mXLoc,
                                        mYLoc, mZLoc, yHeadRot, mEntity->getAdditionContructionTag()));
    }

    if (entityType == 0x000000000A080001ULL)
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddPaintingPacket(boost::static_pointer_cast<Painting>(mEntity)));
    }

    if (entityType == 0x000000000800000AULL)
    {
        return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(mEntity, 2, 1));
    }

    if ((entityType & ENTITY_TYPE_MINECART) == ENTITY_TYPE_MINECART)
    {
        boost::shared_ptr<Minecart> minecart = boost::static_pointer_cast<Minecart>(mEntity);
        int displayData = Block::getIdWithData(minecart->getDisplayBlockState());
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 10, displayData, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (entityType == ENTITY_TYPE_BOAT)
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 1, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x8000003, entityTypeHigh, entityTypeLow))
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddExperienceOrbPacket(boost::static_pointer_cast<ExperienceOrb>(mEntity), yRot, xRot,
                                                  mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0xd000005, entityTypeHigh, entityTypeLow))
    {
        int yHeadRot = Mth::floor(mEntity->getYHeadRot() * 256.0f / 360.0f);
        return boost::shared_ptr<Packet>(
            new ClientboundAddMobPacket(boost::static_pointer_cast<LivingEntity>(mEntity), yRot, xRot, mXLoc,
                                        mYLoc, mZLoc, yHeadRot));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x88000006, entityTypeHigh, entityTypeLow))
    {
        boost::shared_ptr<FishingHook> hook = boost::static_pointer_cast<FishingHook>(mEntity);
        boost::shared_ptr<Player> owner = hook->getOwner();
        int data = owner ? owner->getId() : mEntity->getId();
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x5a, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x8a020001, entityTypeHigh, entityTypeLow))
    {
        boost::shared_ptr<SpectralArrow> arrow = boost::static_pointer_cast<SpectralArrow>(mEntity);
        int data = arrow->mOwner ? arrow->mOwner->getId() + 1 : mEntity->getId() + 1;
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x5b, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x8a020002, entityTypeHigh, entityTypeLow))
    {
        boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(mEntity);
        int data = arrow->mOwner ? arrow->mOwner->getId() + 1 : mEntity->getId() + 1;
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x3c, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x8a200006, entityTypeHigh, entityTypeLow))
    {
        boost::shared_ptr<Arrow> arrow = boost::static_pointer_cast<Arrow>(mEntity);
        int data = arrow->mOwner ? arrow->mOwner->getId() + 1 : mEntity->getId() + 1;
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x5e, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x8a200001, entityTypeHigh, entityTypeLow))
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x3d, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x88000012, entityTypeHigh, entityTypeLow))
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x44, 0, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (eTYPE_DERIVED_FROM__FULT1(0, 0x8a200004, entityTypeHigh, entityTypeLow))
    {
        boost::shared_ptr<ThrownPotion> potion = boost::static_pointer_cast<ThrownPotion>(mEntity);
        not_null_ptr<ItemInstance> item = potion->getItem();
        int data = Item::getId(item->getItem());
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x49, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (entityType == 0x000000008A200005ULL)
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x4b, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (entityType == 0x000000008A200003ULL)
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x41, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (entityType == 0x0000000088000004ULL)
    {
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x48, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if (entityType == 0x0000000088000005ULL)
    {
        boost::shared_ptr<FireworksRocketEntity> rocket =
            boost::static_pointer_cast<FireworksRocketEntity>(mEntity);
        boost::shared_ptr<LivingEntity> owner = rocket->getOwner();
        int data = owner ? owner->getId() : 0;
        return boost::shared_ptr<Packet>(
            new ClientboundAddEntityPacket(mEntity, 0x4c, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
    }

    if ((entityType & 0x000000008A400000ULL) != 0x000000008A400000ULL)
    {
        if (entityType == 0x0000000008000010ULL)
        {
            return boost::shared_ptr<Packet>(new ClientboundAddEntityPacket(
                mEntity, 0x43, 0, yRot, xRot, (int)mEntity->mX, (int)mEntity->mY, (int)mEntity->mZ));
        }

        if (entityType == 0x000000008A200002ULL)
        {
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x3e, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == 0x0000000008000011ULL)
        {
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4f, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == ENTITY_TYPE_PRIMED_TNT)
        {
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x32, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == 0x000000000800000EULL)
        {
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x33, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == ENTITY_TYPE_LEASH_FENCE_KNOT)
        {
            boost::shared_ptr<FallingBlockEntity> fallingBlock =
                boost::static_pointer_cast<FallingBlockEntity>(mEntity);
            int data = Block::getIdWithData(fallingBlock->getBlock());
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x46, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == ENTITY_TYPE_ARMOR_STAND)
        {
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4e, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == 0x000000000A080002ULL)
        {
            boost::shared_ptr<ItemFrame> frame = boost::static_pointer_cast<ItemFrame>(mEntity);
            int data = frame->getDirection()->get2DDataValue();
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x47, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == 0x000000000A080003ULL)
        {
            boost::shared_ptr<LeashFenceKnotEntity> knot =
                boost::static_pointer_cast<LeashFenceKnotEntity>(mEntity);
            BlockPos pos = knot->getPos();
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 0x4d, 0, pos, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        if (entityType == 0x000000000800000FULL)
        {
            boost::shared_ptr<AreaEffectCloud> cloud = boost::static_pointer_cast<AreaEffectCloud>(mEntity);
            boost::shared_ptr<LivingEntity> owner = cloud->getOwner();
            int data = owner ? owner->getId() + 1 : mEntity->getId() + 1;
            return boost::shared_ptr<Packet>(
                new ClientboundAddEntityPacket(mEntity, 3, data, yRot, xRot, mXLoc, mYLoc, mZLoc));
        }

        return boost::shared_ptr<Packet>();
    }

    boost::shared_ptr<Fireball> fireball = boost::static_pointer_cast<Fireball>(mEntity);
    int packetType = 0x3f;
    if (entityType == 0x000000008A400004ULL)
    {
        packetType = 0x40;
    }
    else if (entityType == 0x000000008A400001ULL)
    {
        packetType = 0x5d;
    }
    else if (entityType == 0x000000008A400002ULL)
    {
        packetType = 0x42;
    }

    int data = 0;
    if (fireball->mOwner)
    {
        data = fireball->mOwner->getId();
    }

    return makeFireballPacket(mEntity, packetType, data, yRot, xRot, mXLoc, mYLoc, mZLoc, fireball);
}

bool TrackedEntity::ShouldSendEntityDataOnContruction(boost::shared_ptr<Entity> entity)
{
    return entity && entity->GetType() != ENTITY_TYPE_PLAYER;
}

void TrackedEntity::updatePlayer(EntityTracker* tracker, boost::shared_ptr<ServerPlayer> player)
{
    if (!player || !mEntity)
    {
        return;
    }
    if (isVisibleTo(tracker, player, false, false))
    {
        mTrackedPlayers.insert(player);
        mEntity->startSeenByPlayer(player);
    }
    else
    {
        removePlayer(tracker, player);
    }
}

void TrackedEntity::updatePlayers(EntityTracker* tracker, std::vector<boost::shared_ptr<Player> >* players)
{
    if (!players)
    {
        return;
    }
    for (std::vector<boost::shared_ptr<Player> >::iterator it = players->begin(); it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = boost::dynamic_pointer_cast<ServerPlayer>(*it);
        if (player)
        {
            updatePlayer(tracker, player);
        }
    }
}

void TrackedEntity::broadcast(boost::shared_ptr<Packet> packet)
{
    if (!packet || !Packet::canSendToAnyClient(packet))
    {
        return;
    }
    for (boost::unordered_set<boost::shared_ptr<ServerPlayer>, boost::hash<boost::shared_ptr<ServerPlayer> > >::iterator it = mTrackedPlayers.begin(); it != mTrackedPlayers.end(); ++it)
    {
        if (*it)
        {
            (*it)->mConnection->send(packet);
        }
    }
}

void TrackedEntity::broadcastAndSend(boost::shared_ptr<Packet> packet)
{
    broadcast(packet);
}

void TrackedEntity::sendDirtyEntityData()
{
    if (!mEntity)
    {
        return;
    }
    boost::shared_ptr<SynchedEntityData> data = mEntity->getEntityData();
    if (data && data->isDirty())
    {
        data->clearDirty();
    }
}

void TrackedEntity::resetPlayerVisibility()
{
    mTrackedPlayers.clear();
}

void TrackedEntity::tick(EntityTracker* tracker, std::vector<boost::shared_ptr<Player> >* players)
{
    if (!mEntity)
    {
        return;
    }
    ++mUpdateCounter;
    if (mUpdateCounter % mUpdateInterval == 0)
    {
        updatePlayers(tracker, players);
    }
    sendDirtyEntityData();
}

void TrackedEntity::broadcastRemoved()
{
    if (!mEntity)
    {
        return;
    }
    for (boost::unordered_set<boost::shared_ptr<ServerPlayer>, boost::hash<boost::shared_ptr<ServerPlayer> > >::iterator it = mTrackedPlayers.begin(); it != mTrackedPlayers.end(); ++it)
    {
        if (*it)
        {
            boost::shared_ptr<Entity> entity = mEntity;
            (*it)->sendRemoveEntity(&entity);
        }
    }
    mTrackedPlayers.clear();
}

void TrackedEntity::removePlayer(EntityTracker* tracker, boost::shared_ptr<ServerPlayer> player)
{
    boost::unordered_set<boost::shared_ptr<ServerPlayer>, boost::hash<boost::shared_ptr<ServerPlayer> > >::iterator it = mTrackedPlayers.find(player);
    if (it != mTrackedPlayers.end())
    {
        mTrackedPlayers.erase(it);
        if (player)
        {
            boost::shared_ptr<Entity> entity = mEntity;
            player->sendRemoveEntity(&entity);
        }
    }
}

void TrackedEntity::clear(boost::shared_ptr<ServerPlayer> player)
{
    removePlayer(NULL, player);
}

boost::shared_ptr<Entity> TrackedEntity::getEntity()
{
    return mEntity;
}

void TrackedEntity::setMaxRange(int range)
{
    mMaxRange = range;
}
