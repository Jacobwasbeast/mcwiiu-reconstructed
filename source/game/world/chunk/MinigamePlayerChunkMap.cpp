#include "game/world/chunk/MinigamePlayerChunkMap.hpp"

#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/chunk/PlayerChunkContainer.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/minigame/EMiniGameId.hpp"

MinigamePlayerChunkMap::MinigamePlayerChunkMap(ServerLevel* level, int dimension, int viewDistance)
    : PlayerChunkMap(level, dimension, viewDistance)
{
}

MinigamePlayerChunkMap::~MinigamePlayerChunkMap()
{
}

void MinigamePlayerChunkMap::updatePlayerReferences(boost::shared_ptr<ServerPlayer> player)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        if (mPlayers[i]->mXuid == player->mXuid)
        {
            mPlayers[i] = player;
            break;
        }
    }

    ContainerVector chunks(mAllChunks.begin(), mAllChunks.end());
    for (ContainerVector::iterator it = chunks.begin(); it != chunks.end(); ++it)
        (*it)->updatePlayerReference(player);

    for (RequestVector::iterator it = mAddRequests.begin(); it != mAddRequests.end(); ++it)
    {
        if (it->player->mXuid == player->mXuid)
            it->player = player;
    }
}

void MinigamePlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    updatePlayerReferences(player);
}

void MinigamePlayerChunkMap::remove(boost::shared_ptr<ServerPlayer>)
{
}

void MinigamePlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
}

void MinigamePlayerChunkMap::removeOutsideAreaForAllPlayers(int minX, int minZ,
                                                              int maxX, int maxZ)
{
}

void MinigamePlayerChunkMap::loadPlayerInActiveArea(boost::shared_ptr<ServerPlayer> player)
{
    int minX;
    int minZ;
    int maxX;
    int maxZ;
    gMinecraftServer->mMasterGameMode->GetMaxActiveBounds(minX, minZ, maxX, maxZ);

    if (!Minecraft::InMiniGame(eMiniGameId_Glide, false))
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

    if (gMinecraftServer->getPlayers()->canReceiveAllPackets(player))
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            minX, maxX, minZ, maxZ, true, shouldActuallyDropChunks(), false));
        player->mConnection->send(packet);
    }
}

void MinigamePlayerChunkMap::reloadAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    ContainerVector chunks(mAllChunks);
    mShouldDropChunks = true;
    for (ContainerVector::iterator it = chunks.begin(); it != chunks.end(); ++it)
    {
        PlayerChunkContainer* chunk = *it;
        const int chunkX = chunk->getPos().x;
        const int chunkZ = chunk->getPos().z;
        if (chunkX < minX || chunkX > maxX || chunkZ < minZ || chunkZ > maxZ)
            PlayerChunkContainer::removeAllPlayers(chunk, false);
    }

    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        if (gMinecraftServer->getPlayers()->canReceiveAllPackets(player))
        {
            boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
                minX, maxX, minZ, maxZ, false, shouldActuallyDropChunks(), true));
            player->mConnection->send(packet);
        }
    }
    mShouldDropChunks = false;
    mAddRequests.clear();

    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = mPlayers[i];
        updatePlayerReferences(player);
        loadPlayerInActiveArea(player);
    }
}

void MinigamePlayerChunkMap::onPlayerJoin(boost::shared_ptr<ServerPlayer> player)
{
    mPlayers.push_back(player);
    loadPlayerInActiveArea(player);
}

void MinigamePlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        if (mPlayers[i]->mXuid == player->mXuid)
        {
            mPlayers.erase(mPlayers.begin() + i);
            break;
        }
    }

    ContainerVector chunks(mAllChunks.begin(), mAllChunks.end());
    for (ContainerVector::iterator it = chunks.begin(); it != chunks.end(); ++it)
    {
        (*it)->updatePlayerReference(player);
        (*it)->remove(player, false);
    }

    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player->mXuid == player->mXuid)
            request = mAddRequests.erase(request);
        else
            ++request;
    }
}

uint32_t MinigamePlayerChunkMap::containsPlayer(boost::shared_ptr<ServerPlayer> player)
{
    for (unsigned int i = 0; i < mPlayers.size(); ++i)
    {
        if (mPlayers[i]->mXuid == player->mXuid)
            return 1;
    }
    return 0;
}
