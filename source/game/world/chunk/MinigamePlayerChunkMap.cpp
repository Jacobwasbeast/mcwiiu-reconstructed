#include "game/world/chunk/MinigamePlayerChunkMap.hpp"

#include <algorithm>
#include <boost/shared_ptr.hpp>

#include "game/core/MasterGameMode.hpp"
#include "game/core/Minecraft.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/chunk/PlayerChunkContainer.hpp"
#include "game/world/generation/PlayerList.hpp"

extern MinecraftServer* gMinecraftServer;

MinigamePlayerChunkMap::MinigamePlayerChunkMap(ServerLevel* level, int viewDistance, int radius)
    : PlayerChunkMap(level, viewDistance, radius)
{
}

MinigamePlayerChunkMap::~MinigamePlayerChunkMap()
{
}

void MinigamePlayerChunkMap::updatePlayerReferences(boost::shared_ptr<ServerPlayer> player)
{
    PlayerUID uid = player->mPlayerUID;
    PlayerVector::iterator playerIt = mPlayers.begin();
    while (playerIt != mPlayers.end())
    {
        if (*playerIt && (*playerIt)->mPlayerUID == uid)
        {
            *playerIt = player;
            break;
        }
        ++playerIt;
    }

    ContainerVector::iterator chunkIt = mLoadedChunks.begin();
    while (chunkIt != mLoadedChunks.end())
    {
        if (*chunkIt != NULL)
            (*chunkIt)->updatePlayerReference(player);
        ++chunkIt;
    }

    RequestVector::iterator requestIt = mAddRequests.begin();
    while (requestIt != mAddRequests.end())
    {
        if (requestIt->player && requestIt->player->mPlayerUID == uid)
            requestIt->player = player;
        ++requestIt;
    }
}

void MinigamePlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    updatePlayerReferences(player);
}

void MinigamePlayerChunkMap::remove(boost::shared_ptr<ServerPlayer> player)
{
}

void MinigamePlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    player->mLastSectionPos = player->mSectionPos;
    player->mLastChunkPos = player->mChunkPos;
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

    if (!Minecraft::InMiniGame((EMiniGameId)3, false))
    {
        minX -= 5;
        minZ -= 5;
        maxX += 5;
        maxZ += 5;
    }

    int x;
    int z;
    for (x = minX; x <= maxX; ++x)
    {
        for (z = minZ; z <= maxZ; ++z)
            getChunkAndAddPlayer(x, z, player, false);
    }

    if (gMinecraftServer != NULL && gMinecraftServer->getPlayers()->canReceiveAllPackets(player))
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            minX, minZ, maxX, maxZ, true, shouldActuallyDropChunks(), false));
        if (player->connection != NULL)
            player->connection->send(packet);
    }
}

void MinigamePlayerChunkMap::reloadAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    ContainerVector chunks(mLoadedChunks);
    mPauseBlockUpdatePackets = true;

    ContainerVector::iterator chunkIt = chunks.begin();
    while (chunkIt != chunks.end())
    {
        if (*chunkIt != NULL)
        {
            long long pos = (*chunkIt)->getPos();
            int x = static_cast<int>(pos >> 32);
            int z = static_cast<int>(pos);
            if (x < minX || x > maxX || z < minZ || z > maxZ)
                PlayerChunkContainer::removeAllPlayers(*chunkIt, false);
        }
        ++chunkIt;
    }

    mPauseBlockUpdatePackets = false;
    mLoadedChunks.clear();

    PlayerVector::iterator playerIt = mPlayers.begin();
    while (playerIt != mPlayers.end())
    {
        if (*playerIt)
        {
            if (gMinecraftServer != NULL && gMinecraftServer->getPlayers()->canReceiveAllPackets(*playerIt))
            {
                boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
                    minX, minZ, maxX, maxZ, false, shouldActuallyDropChunks(), true));
                if ((*playerIt)->connection != NULL)
                    (*playerIt)->connection->send(packet);
            }
            loadPlayerInActiveArea(*playerIt);
        }
        ++playerIt;
    }
}

void MinigamePlayerChunkMap::onPlayerJoin(boost::shared_ptr<ServerPlayer> player)
{
    mPlayers.push_back(player);
    loadPlayerInActiveArea(player);
}

void MinigamePlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    PlayerVector::iterator playerIt = mPlayers.begin();
    while (playerIt != mPlayers.end())
    {
        if (*playerIt && (*playerIt)->mPlayerUID == player->mPlayerUID)
            playerIt = mPlayers.erase(playerIt);
        else
            ++playerIt;
    }

    ContainerVector::iterator chunkIt = mLoadedChunks.begin();
    while (chunkIt != mLoadedChunks.end())
    {
        if (*chunkIt != NULL)
            (*chunkIt)->remove(player, false);
        ++chunkIt;
    }
}

bool MinigamePlayerChunkMap::containsPlayer(boost::shared_ptr<ServerPlayer> player)
{
    PlayerUID uid = player->mPlayerUID;
    PlayerVector::const_iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it && (*it)->mPlayerUID == uid)
            return true;
        ++it;
    }
    return false;
}
