#include "game/entity/mob/TrackedEntity.hpp"

#include <cmath>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/EntityTracker.hpp"
#include "game/entity/mob/SynchedEntityData.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerPlayer.hpp"

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

boost::shared_ptr<Packet> TrackedEntity::getAddEntityPacket()
{
    return boost::shared_ptr<Packet>();
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
