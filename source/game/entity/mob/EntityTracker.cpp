#include "game/entity/mob/EntityTracker.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/TrackedEntity.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/chunk/LevelChunk.hpp"

EntityTracker::EntityTracker(ServerLevel* level)
    : mLevel(level), mMaxRange(0)
{
    mMaxRange = level->getServer()->getPlayers()->getMaxRange();
}

unsigned int EntityTracker::truncate(double value)
{
    int result = (int)(value * 32.0);
    if (value * 32.0 < (double)result)
    {
        --result;
    }
    return (unsigned int)result;
}

void EntityTracker::addEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval, bool sendVelocityUpdates)
{
    boost::shared_ptr<TrackedEntity> tracker(new TrackedEntity(entity, range, mMaxRange, updateInterval, sendVelocityUpdates));
    mTrackedEntities.insert(tracker);
    mEntityMap[entity->getId()] = tracker;
    tracker->updatePlayers(this, mLevel->getPlayers(0, NULL));
}

void EntityTracker::addEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval)
{
    addEntity(entity, range, updateInterval, false);
}

void EntityTracker::addEntity(boost::shared_ptr<Entity> entity)
{
    unsigned long long type = entity->GetType();
    if (type == ENTITY_TYPE_SERVER_PLAYER)
    {
        addEntity(entity, 512, 2);
    }
    else if (type == ENTITY_TYPE_BOAT || type == ENTITY_TYPE_MINECART || type == ENTITY_TYPE_MINECART_RIDEABLE)
    {
        addEntity(entity, 80, 3, true);
    }
    else if (type == ENTITY_TYPE_PRIMED_TNT)
    {
        addEntity(entity, 160, 20, true);
    }
    else if (type == ENTITY_TYPE_ENDER_DRAGON)
    {
        addEntity(entity, 160, 4, true);
    }
    else
    {
        addEntity(entity, 64, 3, false);
    }
}

void EntityTracker::removeEntity(boost::shared_ptr<Entity> entity)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity> >::iterator it = mEntityMap.find(entity->getId());
    if (it != mEntityMap.end())
    {
        boost::shared_ptr<TrackedEntity> tracker = it->second;
        mEntityMap.erase(it);
        mTrackedEntities.erase(tracker);
        tracker->broadcastRemoved();
    }
}

void EntityTracker::removePlayer(boost::shared_ptr<Entity> entity)
{
    if (entity->GetType() == ENTITY_TYPE_SERVER_PLAYER)
    {
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(entity);
        for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
        {
            (*it)->removePlayer(this, player);
        }
        player->flushEntitiesToRemove();
    }
}

void EntityTracker::tick(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        (*it)->tick(this, mLevel->getPlayers(0, NULL));
    }
}

void EntityTracker::updateVisibility(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        if ((*it)->getEntity() == player)
        {
            (*it)->updatePlayers(this, mLevel->getPlayers(0, NULL));
        }
        else
        {
            (*it)->updatePlayer(this, player);
        }
    }
}

void EntityTracker::broadcast(boost::shared_ptr<Entity> entity, boost::shared_ptr<Packet> packet)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity> >::iterator it = mEntityMap.find(entity->getId());
    if (it != mEntityMap.end())
    {
        it->second->broadcast(packet);
    }
}

void EntityTracker::broadcastAndSend(boost::shared_ptr<Entity> entity, boost::shared_ptr<Packet> packet)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity> >::iterator it = mEntityMap.find(entity->getId());
    if (it != mEntityMap.end())
    {
        it->second->broadcastAndSend(packet);
    }
}

void EntityTracker::clear(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        (*it)->clear(player);
    }
}

void EntityTracker::playerLoadedChunk(boost::shared_ptr<ServerPlayer> player, LevelChunk* chunk)
{
    std::vector<boost::shared_ptr<Entity> > entities;
    chunk->getEntities(boost::shared_ptr<Entity>(), NULL, entities, NULL);
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = entities.begin(); it != entities.end(); ++it)
    {
        boost::shared_ptr<TrackedEntity> tracker = getTracker(*it);
        if (tracker)
        {
            tracker->updatePlayer(this, player);
        }
    }
}

void EntityTracker::setViewDistance(int distance)
{
    mMaxRange = distance * 16 - 16;
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        (*it)->setMaxRange(mMaxRange);
    }
}

void EntityTracker::updateMaxRange()
{
    mMaxRange = mLevel->getServer()->getPlayers()->getMaxRange();
}

boost::shared_ptr<TrackedEntity> EntityTracker::getTracker(boost::shared_ptr<Entity> entity)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity> >::iterator it = mEntityMap.find(entity->getId());
    if (it == mEntityMap.end())
    {
        return boost::shared_ptr<TrackedEntity>();
    }
    return it->second;
}
