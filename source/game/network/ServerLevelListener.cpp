#include "game/network/ServerLevelListener.hpp"
#include "game/core/EntityType.hpp"

#include <vector>

#include "game/core/MemSect.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/entity/mob/EntityTracker.hpp"
#include "game/network/ClientboundBlockDestructionPacket.hpp"
#include "game/network/ClientboundLevelEventPacket.hpp"
#include "game/network/ClientboundSoundPacket.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
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
    MemSect(10);
    mLevel->getTracker()->addEntity(entity);
    MemSect(0);

    const bool isServerPlayer = entity->GetType() == EntityType::SERVER_PLAYER;
    if (isServerPlayer)
        mLevel->mDimension->onPlayerJoin(boost::static_pointer_cast<ServerPlayer>(entity));
}

void ServerLevelListener::entityRemoved(boost::shared_ptr<Entity> entity)
{
    mLevel->getTracker()->removeEntity(entity);

    const bool isServerPlayer = entity->GetType() == EntityType::SERVER_PLAYER;
    if (isServerPlayer)
        mLevel->mDimension->onPlayerLeave(boost::static_pointer_cast<ServerPlayer>(entity));
}

void ServerLevelListener::playerRemoved(boost::shared_ptr<Entity> player)
{
    boost::shared_ptr<ServerPlayer> serverPlayer =
        boost::dynamic_pointer_cast<ServerPlayer>(player);
    serverPlayer->getLevel()->getTracker()->removePlayer(player);
}

void ServerLevelListener::playSoundToAllPlayers(const SoundEvent* sound,
                                                 SoundSource::ESoundSource, double, double,
                                                 double, float volume, float pitch, float)
{
    boost::shared_ptr<Packet> packet = boost::shared_ptr<ClientboundSoundPacket>(new ClientboundSoundPacket(sound, volume, pitch));

    std::vector<boost::shared_ptr<ServerPlayer> >* players = mServer->getPlayers()->getPlayers();
    std::vector<boost::shared_ptr<ServerPlayer> > sentPlayers;

    for (unsigned int i = 0; i < players->size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = (*players)[i];

        if (!sentPlayers.empty())
        {
            INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
            if (networkPlayer == NULL)
                continue;

            bool alreadySent = false;
            for (unsigned int j = 0; j < sentPlayers.size(); ++j)
            {
                boost::shared_ptr<ServerPlayer> sentPlayer = sentPlayers[j];
                INetworkPlayer* other = sentPlayer->mConnection->getNetworkPlayer();
                if (other != NULL && networkPlayer->IsSameSystem(other))
                    alreadySent = true;
            }

            if (alreadySent)
                continue;
        }

        player->mConnection->send(packet);
        sentPlayers.push_back(player);
    }
}

void ServerLevelListener::playSound(boost::shared_ptr<Player> player, const SoundEvent* sound,
                                    SoundSource::ESoundSource source, double x, double y,
                                    double z, float volume, float pitch, float)
{
    if (volume <= 0.02f)
        return;

    (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->broadcast(player, x, y, z, volume > 1.0f ? 16.0f * volume : 16.0,
                                              mLevel->mDimension->getType()->getId(),
                                              boost::shared_ptr<ClientboundSoundPacket>(new ClientboundSoundPacket(sound, source, x, y, z, volume, pitch)));
}

void ServerLevelListener::levelEvent(boost::shared_ptr<Player> player, int type,
                                     const BlockPos& pos, int data)
{
    (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->broadcast(player, pos.x, pos.y, pos.z, 64.0,
                                              mLevel->mDimension->getType()->getId(),
                                              boost::shared_ptr<ClientboundLevelEventPacket>(new ClientboundLevelEventPacket(type, pos, data, false)));
}

void ServerLevelListener::globalLevelEvent(int type, const BlockPos& pos, int data)
{
    (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->broadcastAll(
        boost::shared_ptr<ClientboundLevelEventPacket>(new ClientboundLevelEventPacket(type, pos, data, true)));
}

void ServerLevelListener::destroyBlockProgress(int breakerId, const BlockPos& pos, int progress)
{
    std::vector<boost::shared_ptr<ServerPlayer> >* players =
        (gMinecraftServer == NULL ? NULL : gMinecraftServer->mPlayers)->getPlayers();

    INetworkPlayer* breakerNetworkPlayer = NULL;
    for (unsigned int i = 0; i < players->size(); ++i)
    {
        if ((*players)[i]->getId() == breakerId)
        {
            breakerNetworkPlayer = (*players)[i]->mConnection->getNetworkPlayer();
            break;
        }
    }

    for (std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players->begin();
         it != players->end(); ++it)
    {
        boost::shared_ptr<ServerPlayer> player = *it;
        ServerPlayer* target = player.get();

        if (target == NULL || target->mLevel != mLevel || target->getId() == breakerId)
            continue;

        if (target->mConnection->getNetworkPlayer() == NULL)
            continue;

        if (breakerNetworkPlayer != NULL
            && breakerNetworkPlayer->IsSameSystem(target->mConnection->getNetworkPlayer()))
            continue;

        double dx = pos.x - target->mX;
        double dy = pos.y - target->mY;
        double dz = pos.z - target->mZ;

        if (dx * dx + dy * dy + dz * dz < 1024.0)
        {
            target->mConnection->send(boost::shared_ptr<ClientboundBlockDestructionPacket>(
                new ClientboundBlockDestructionPacket(breakerId, pos, progress)));
        }
    }
}
