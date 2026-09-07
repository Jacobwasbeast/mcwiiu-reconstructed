#include "game/network/ServerLevelListener.hpp"

#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/network/ClientboundBlockDestructionPacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/world/Dimension.hpp"
#include "game/world/DimensionType.hpp"
#include "game/world/Level.hpp"

ServerLevelListener::ServerLevelListener(MinecraftServer* server, ServerLevel* level)
    : mServer(server), mLevel(level)
{
}

ServerLevelListener::~ServerLevelListener()
{
}

void ServerLevelListener::addParticle(Level*, ePARTICLE_TYPE, bool, double, double, double,
                                       double, double, double, arrayWithLength<int>)
{
}

void ServerLevelListener::addParticle(Level*, ePARTICLE_TYPE, bool, bool, double, double, double,
                                       double, double, double, arrayWithLength<int>)
{
}

void ServerLevelListener::allChanged()
{
}

void ServerLevelListener::setBlocksDirty(int, int, int, int, int, int, Level*)
{
}

void ServerLevelListener::blockLightChanged(const BlockPos&)
{
}

void ServerLevelListener::playStreamingMusic(const SoundEvent*, const BlockPos&, bool)
{
}

void ServerLevelListener::blockChanged(Level*, const BlockPos& pos, const BlockState*,
                                       const BlockState*, int, bool immediate)
{
    if (immediate)
        mLevel->getChunkMap()->blockChanged(pos);
}

void ServerLevelListener::entityAdded(boost::shared_ptr<Entity> entity)
{
    mLevel->getTracker()->addEntity(entity);
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        mLevel->mDimension->onPlayerJoin(boost::static_pointer_cast<ServerPlayer>(entity));
}

void ServerLevelListener::entityRemoved(boost::shared_ptr<Entity> entity)
{
    mLevel->getTracker()->removeEntity(entity);
    if ((entity->GetType() & ENTITY_TYPE_PLAYER) == ENTITY_TYPE_PLAYER)
        mLevel->mDimension->onPlayerLeave(boost::static_pointer_cast<ServerPlayer>(entity));
}

void ServerLevelListener::playerRemoved(boost::shared_ptr<Entity> player)
{
    boost::shared_ptr<ServerPlayer> serverPlayer =
        boost::dynamic_pointer_cast<ServerPlayer>(player);
    if (!serverPlayer)
        return;
    serverPlayer->getLevel()->getTracker()->removePlayer(player);
}

void ServerLevelListener::playSoundToAllPlayers(const SoundEvent* sound,
                                                 SoundSource::ESoundSource source, double x,
                                                 double y, double z, float volume, float pitch,
                                                 float range)
{
    boost::shared_ptr<Packet> packet(new ClientboundSoundPacket(sound, source, x, y, z,
                                                                 volume, pitch));
    mServer->getPlayers()->broadcast(x, y, z, range, mLevel->mDimension->getType()->getId(),
                                     packet);
}

void ServerLevelListener::playSound(boost::shared_ptr<Player> player, const SoundEvent* sound,
                                    SoundSource::ESoundSource source, double x, double y,
                                    double z, float volume, float pitch, float range)
{
    if (volume <= 0.02f)
        return;
    double radius = range;
    if (radius <= 1.0)
        radius = static_cast<double>(volume) * 16.0;
    boost::shared_ptr<Packet> packet(new ClientboundSoundPacket(sound, source, x, y, z,
                                                                 volume, pitch));
    mServer->getPlayers()->broadcast(player, x, y, z, radius,
                                     mLevel->mDimension->getType()->getId(), packet);
}

void ServerLevelListener::levelEvent(boost::shared_ptr<Player> player, int type,
                                     const BlockPos& pos, int data)
{
    boost::shared_ptr<Packet> packet(new ClientboundLevelEventPacket(type, pos, data, false));
    mServer->getPlayers()->broadcast(player, pos.x, pos.y, pos.z, 64.0,
                                     mLevel->mDimension->getType()->getId(), packet);
}

void ServerLevelListener::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    boost::shared_ptr<Packet> packet(new ClientboundLevelEventPacket(type, pos, data, false));
    mServer->getPlayers()->broadcast(pos.x, pos.y, pos.z, 64.0,
                                     mLevel->mDimension->getType()->getId(), packet);
}

void ServerLevelListener::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    boost::shared_ptr<Packet> packet(new ClientboundBlockDestructionPacket(breakerId, pos,
                                                                             progress));
    mServer->getPlayers()->broadcast(pos.x, pos.y, pos.z, 32.0,
                                     mLevel->mDimension->getType()->getId(), packet);
}
