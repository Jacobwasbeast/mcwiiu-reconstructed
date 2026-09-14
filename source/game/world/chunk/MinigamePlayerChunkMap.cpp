#include "game/world/chunk/MinigamePlayerChunkMap.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/generation/PlayerList.hpp"

MinigamePlayerChunkMap::MinigamePlayerChunkMap(ServerLevel* level, int viewDistance, int radius)
    : PlayerChunkMap(level, viewDistance, radius)
{
}

MinigamePlayerChunkMap::~MinigamePlayerChunkMap()
{
}

void MinigamePlayerChunkMap::updatePlayerReferences(boost::shared_ptr<ServerPlayer> player)
{
    PlayerChunkMap::add(player);
}

void MinigamePlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    updatePlayerReferences(player);
}

void MinigamePlayerChunkMap::remove(boost::shared_ptr<ServerPlayer> player)
{
    (void)player;
}

void MinigamePlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    if (player)
    {
        player->mLastSentX = player->mX;
        player->mLastSentZ = player->mZ;
    }
}

void MinigamePlayerChunkMap::removeOutsideAreaForAllPlayers(int minX, int minZ,
                                                              int maxX, int maxZ)
{
    PlayerChunkMap::removeOutsideAreaForAllPlayers(minX, minZ, maxX, maxZ);
}

void MinigamePlayerChunkMap::loadPlayerInActiveArea(boost::shared_ptr<ServerPlayer> player)
{
    if (!player || gMinecraftServer == NULL || gMinecraftServer->mMasterGameMode == NULL)
        return;

    int minX;
    int minZ;
    int maxX;
    int maxZ;
    gMinecraftServer->mMasterGameMode->GetMaxActiveBounds(minX, minZ, maxX, maxZ);

    if (!Minecraft::InMiniGame((EMiniGameId)3, false))
    {
        minX -= 5;
        minZ -= 5;
        maxX += 5;
        maxZ += 5;
    }

    for (int x = minX; x <= maxX; ++x)
    {
        for (int z = minZ; z <= maxZ; ++z)
            getChunkAndAddPlayer(x, z, player, false);
    }

    PlayerList* players = gMinecraftServer->getPlayers();
    if (players != NULL && players->canReceiveAllPackets(player) && player->mConnection != NULL)
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            minX, maxX, minZ, maxZ, true, shouldActuallyDropChunks(), false));
        player->mConnection->send(packet);
    }
}

void MinigamePlayerChunkMap::reloadAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    PlayerChunkMap::reloadAllPlayers(minX, minZ, maxX, maxZ);
}

void MinigamePlayerChunkMap::onPlayerJoin(boost::shared_ptr<ServerPlayer> player)
{
    PlayerChunkMap::onPlayerJoin(player);
    loadPlayerInActiveArea(player);
}

void MinigamePlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    PlayerChunkMap::onPlayerLeave(player);
}

uint32_t MinigamePlayerChunkMap::containsPlayer(boost::shared_ptr<ServerPlayer> player)
{
    return player && PlayerChunkMap::isPlayerIn(player, player->mChunkX, player->mChunkZ);
}
