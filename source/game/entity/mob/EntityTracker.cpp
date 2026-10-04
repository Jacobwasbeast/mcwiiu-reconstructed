#include "game/entity/mob/EntityTracker.hpp"
#include "game/core/Boat.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/core/Minecraft.hpp"
#include "game/entity/mob/Creature.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/entity/mob/TrackedEntity.hpp"
#include "game/network/ClientboundSetEntityLinkPacket.hpp"
#include "game/network/ClientboundSetPassengersPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/chunk/LevelChunk.hpp"

EntityTracker::EntityTracker(ServerLevel* level)
    : mTrackedEntities(), mEntityMap()
{
    mLevel = level;
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

void EntityTracker::addEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval, bool trackDelta)
{
    boost::shared_ptr<TrackedEntity> tracker(new TrackedEntity(entity, range, mMaxRange, updateInterval, trackDelta));
    mTrackedEntities.insert(tracker);
    mEntityMap[entity->getId()] = tracker;
    tracker->updatePlayers(this, &mLevel->mPlayers);
}

void EntityTracker::addEntity(boost::shared_ptr<Entity> entity, int range, int updateInterval)
{
    addEntity(entity, range, updateInterval, false);
}

void EntityTracker::addEntity(boost::shared_ptr<Entity> entity)
{
    if (entity->GetType() == EntityType::SERVER_PLAYER)
    {
        addEntity(entity, 512, 2);
        boost::shared_ptr<ServerPlayer> player = boost::static_pointer_cast<ServerPlayer>(entity);
        for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
        {
            if ((*it)->getEntity() != player)
                (*it)->updatePlayer(this, player);
        }
    }
    else if (entity->GetType() == EntityType::FISHING_HOOK)
        addEntity(entity, 64, 5, true);
    else if (entity->GetType() == EntityType::SMALL_FIREBALL)
        addEntity(entity, 64, 10, false);
    else if (entity->GetType() == EntityType::DRAGON_FIREBALL)
        addEntity(entity, 64, 10, false);
    else if ((entity->GetType() & EntityType::ARROW) == EntityType::ARROW)
        addEntity(entity, 64, 20, false);
    else if ((entity->GetType() & EntityType::FIREBALL) == EntityType::FIREBALL)
        addEntity(entity, 64, 10, true);
    else if (entity->GetType() == EntityType::SNOWBALL)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
            addEntity(entity, 64, 40, true);
        else
            addEntity(entity, 64, 10, true);
    }
    else if (entity->GetType() == EntityType::LLAMA_SPIT)
        addEntity(entity, 64, 10, false);
    else if (entity->GetType() == EntityType::THROWN_ENDERPEARL)
        addEntity(entity, 64, 10, true);
    else if (entity->GetType() == EntityType::EYE_OF_ENDER_SIGNAL)
        addEntity(entity, 64, 4, true);
    else if (entity->GetType() == EntityType::THROWN_EGG)
        addEntity(entity, 64, 10, true);
    else if (entity->GetType() == EntityType::THROWN_POTION)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
            addEntity(entity, 64, 40, false);
        else
            addEntity(entity, 64, 10, true);
    }
    else if (entity->GetType() == EntityType::THROWN_EXP_BOTTLE)
        addEntity(entity, 64, 10, true);
    else if (entity->GetType() == EntityType::FIREWORKS_ROCKET_ENTITY)
    {
        if (Minecraft::InMiniGame(eMiniGameId_Tumble, true))
            addEntity(entity, 64, 40, false);
        else
            addEntity(entity, 64, 10, true);
    }
    else if (entity->GetType() == EntityType::THROWN_TRIDENT)
        addEntity(entity, 64, 10, true);
    else if (entity->GetType() == EntityType::ITEM_ENTITY)
        addEntity(entity, 64, 20, true);
    else if (eTYPE_DERIVED_FROM(EntityType::MINECART, entity->GetType()))
        addEntity(entity, 80, 3, true);
    else if (eTYPE_DERIVED_FROM(EntityType::BOAT, entity->GetType()))
        addEntity(entity, 80, 3, true);
    else if (eTYPE_DERIVED_FROM(EntityType::SQUID, entity->GetType()))
        addEntity(entity, 64, 3, true);
    else if (eTYPE_DERIVED_FROM(EntityType::WITHER_BOSS, entity->GetType()))
        addEntity(entity, 80, 3, true);
    else if (eTYPE_DERIVED_FROM(EntityType::SHULKER_BULLET, entity->GetType()))
        addEntity(entity, 80, 3, true);
    else if (eTYPE_DERIVED_FROM(EntityType::BAT, entity->GetType()))
        addEntity(entity, 80, 3, false);
    else if (eTYPE_DERIVED_FROM(EntityType::ENDER_DRAGON, entity->GetType()))
        addEntity(entity, 160, 3, true);
    else if (eTYPE_DERIVED_FROM(EntityType::PRIMED_TNT, entity->GetType()))
        addEntity(entity, 160, 10, true);
    else if (eTYPE_DERIVED_FROM(EntityType::FALLING_BLOCK_ENTITY, entity->GetType()))
        addEntity(entity, 160, 20, true);
    else if (eTYPE_DERIVED_FROM(EntityType::ITEM_FRAME, entity->GetType()))
        addEntity(entity, 160, 0x7fffffff, false);
    else if (eTYPE_DERIVED_FROM(EntityType::HANGING_ENTITY, entity->GetType()))
        addEntity(entity, 160, 0x7fffffff, false);
    else if (entity->GetType() == EntityType::ARMOR_STAND)
        addEntity(entity, 160, 3, true);
    else if (entity->GetType() == EntityType::EXPERIENCE_ORB)
        addEntity(entity, 160, 20, true);
    else if (entity->GetType() == EntityType::AREA_EFFECT_CLOUD)
        addEntity(entity, 160, 0x7fffffff, true);
    else if (entity->GetType() == EntityType::ENDER_CRYSTAL)
        addEntity(entity, 256, 0x7fffffff, false);
    else if (entity->GetType() == EntityType::EVOCATION_FANGS)
        addEntity(entity, 160, 2, false);
    else if (entity->GetType() == EntityType::PHANTOM)
        addEntity(entity, 160, 1, true);
    else if (boost::dynamic_pointer_cast<Creature>(entity) != NULL || entity->GetType() == EntityType::SHULKER)
        addEntity(entity, 80, 3, true);
}

void EntityTracker::removeEntity(boost::shared_ptr<Entity> entity)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity>, IntKeyHash2, IntKeyEq>::iterator it = mEntityMap.find(entity->getId());
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
    if (entity->GetType() == EntityType::SERVER_PLAYER)
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
    std::vector<boost::shared_ptr<ServerPlayer> > movedPlayers;
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        boost::shared_ptr<TrackedEntity> tracker = *it;
        tracker->tick(this, &mLevel->mPlayers);
        if (tracker->mPlayerEntitiesUpdated && tracker->mEntity->GetType() == EntityType::SERVER_PLAYER)
            movedPlayers.push_back(boost::static_pointer_cast<ServerPlayer>(tracker->mEntity));
    }

    for (unsigned int i = 0; i < gMinecraftServer->getPlayers()->mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> serverPlayer = gMinecraftServer->getPlayers()->mPlayers[i];
        if (serverPlayer->mDimension != mLevel->mDimension->getType()->getId() || serverPlayer->mConnection == NULL)
            continue;
        INetworkPlayer* networkPlayer = serverPlayer->mConnection->getNetworkPlayer();
        if (networkPlayer == NULL)
            continue;
        for (unsigned int j = 0; j < movedPlayers.size(); ++j)
        {
            boost::shared_ptr<ServerPlayer> movedPlayer = movedPlayers[j];
            if (movedPlayer == serverPlayer)
                break;
            if (movedPlayer->mConnection == NULL)
                continue;
            INetworkPlayer* movedNetworkPlayer = movedPlayer->mConnection->getNetworkPlayer();
            if (movedNetworkPlayer != NULL && networkPlayer->IsSameSystem(movedNetworkPlayer))
            {
                movedPlayers.push_back(serverPlayer);
                break;
            }
        }
    }

    for (unsigned int i = 0; i < movedPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> movedPlayer = movedPlayers[i];
        if (movedPlayer->mConnection == NULL)
            continue;
        for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
        {
            boost::shared_ptr<TrackedEntity> tracker = *it;
            if (tracker->mEntity != movedPlayer)
                tracker->updatePlayer(this, movedPlayer);
        }
    }

    for (std::vector<boost::shared_ptr<Player> >::iterator it = mLevel->mPlayers.begin(); it != mLevel->mPlayers.end();
         ++it)
    {
        boost::shared_ptr<ServerPlayer> levelPlayer = boost::dynamic_pointer_cast<ServerPlayer>(*it);
        if (!levelPlayer->isAlive())
            levelPlayer->flushEntitiesToRemove();
    }
}

void EntityTracker::updateVisibility(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        boost::shared_ptr<TrackedEntity> tracker = *it;
        if (tracker->getEntity() == player)
            tracker->updatePlayers(this, &mLevel->mPlayers);
        else
            tracker->updatePlayer(this, player);
    }
}

void EntityTracker::broadcast(boost::shared_ptr<Entity> entity, boost::shared_ptr<Packet> packet)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity>, IntKeyHash2, IntKeyEq>::iterator it = mEntityMap.find(entity->getId());
    if (it != mEntityMap.end())
    {
        boost::shared_ptr<TrackedEntity> tracker = it->second;
        tracker->broadcast(packet);
    }
}

void EntityTracker::broadcastAndSend(boost::shared_ptr<Entity> entity, boost::shared_ptr<Packet> packet)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity>, IntKeyHash2, IntKeyEq>::iterator it = mEntityMap.find(entity->getId());
    if (it != mEntityMap.end())
    {
        boost::shared_ptr<TrackedEntity> tracker = it->second;
        tracker->broadcastAndSend(packet);
    }
}

void EntityTracker::clear(boost::shared_ptr<ServerPlayer> player)
{
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        boost::shared_ptr<TrackedEntity> tracker = *it;
        tracker->clear(player);
    }
}

void EntityTracker::playerLoadedChunk(boost::shared_ptr<ServerPlayer> player, LevelChunk* chunk)
{
    std::vector<boost::shared_ptr<Entity> > leashedEntities;
    std::vector<boost::shared_ptr<Entity> > vehicles;
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin();
         it != mTrackedEntities.end(); ++it)
    {
        boost::shared_ptr<TrackedEntity> tracker = *it;
        boost::shared_ptr<Entity> entity = tracker->getEntity();
        if (entity != player && entity->mChunkX == chunk->x && entity->mChunkZ == chunk->z)
        {
            tracker->updatePlayer(this, player);
            if (((entity->GetType() & EntityType::MOB) == EntityType::MOB &&
                 boost::static_pointer_cast<Mob>(entity)->getLeashHolder() != NULL) ||
                (entity->GetType() == EntityType::BOAT &&
                 boost::static_pointer_cast<Boat>(entity)->getLeashHolder() != NULL))
                leashedEntities.push_back(entity);
            if (!entity->getPassengers()->empty())
                vehicles.push_back(entity);
        }
    }
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = leashedEntities.begin(); it != leashedEntities.end(); ++it)
    {
        boost::shared_ptr<Entity> leashed = *it;
        if ((leashed->GetType() & EntityType::MOB) == EntityType::MOB)
            player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundSetEntityLinkPacket(leashed, boost::static_pointer_cast<Mob>(leashed)->getLeashHolder())));
        else if (leashed->GetType() == EntityType::BOAT)
            player->mConnection->send(boost::shared_ptr<Packet>(new ClientboundSetEntityLinkPacket(leashed, boost::static_pointer_cast<Boat>(leashed)->getLeashHolder())));
    }
    for (std::vector<boost::shared_ptr<Entity> >::iterator it = vehicles.begin(); it != vehicles.end(); ++it)
    {
        boost::shared_ptr<Entity> vehicle = *it;
        boost::shared_ptr<ClientboundSetPassengersPacket> packet(new ClientboundSetPassengersPacket(vehicle));
        player->mConnection->send(boost::shared_ptr<Packet>(packet));
    }
}

void EntityTracker::setViewDistance(int distance)
{
    mMaxRange = distance * 16 - 16;
    for (boost::unordered_set<boost::shared_ptr<TrackedEntity>, boost::hash<boost::shared_ptr<TrackedEntity> > >::iterator it = mTrackedEntities.begin(); it != mTrackedEntities.end(); ++it)
    {
        boost::shared_ptr<TrackedEntity> tracker = *it;
        tracker->setMaxRange(mMaxRange);
    }
}

void EntityTracker::updateMaxRange()
{
    mMaxRange = mLevel->getServer()->getPlayers()->getMaxRange();
}

boost::shared_ptr<TrackedEntity> EntityTracker::getTracker(boost::shared_ptr<Entity> entity)
{
    boost::unordered_map<int, boost::shared_ptr<TrackedEntity>, IntKeyHash2, IntKeyEq>::iterator it = mEntityMap.find(entity->getId());
    if (it != mEntityMap.end())
    {
        return it->second;
    }
    return boost::shared_ptr<TrackedEntity>();
}
