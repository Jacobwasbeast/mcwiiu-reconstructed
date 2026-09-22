#include "game/world/chunk/PlayerChunkMap.hpp"

#include <algorithm>

#include "game/core/Mth.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"
#include "game/network/ChunkVisibilityAreaPacket.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/PlayerChunkContainer.hpp"

PlayerChunkMap::PlayerChunkMap(ServerLevel* level, int viewDistance, int radius)
    : mTicksUntilNextUpdate(0), mLevel(NULL), mViewDistance(viewDistance), mRadius(radius),
      mLastUpdate(0), mShouldDropChunks(false), mPauseBlockUpdatePackets(false)
{
    mLevel = (Level*)level;
    setViewDistance(viewDistance);
    setRadius(radius);
}

PlayerChunkMap::~PlayerChunkMap()
{
    ContainerMap::iterator it = mChunks.begin();
    while (it != mChunks.end())
    {
        delete it->second;
        ++it;
    }
    mChunks.clear();
    mLoadedChunks.clear();
    mBlockUpdates.clear();
}

void PlayerChunkMap::flagEntitiesToBeRemoved(unsigned int* count, bool* removed)
{
    PlayerVector::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        (*it)->flagEntitiesToBeRemoved(count, removed);
        ++it;
    }
}

Level* PlayerChunkMap::getLevel()
{
    return mLevel;
}

long long PlayerChunkMap::tickAddRequests(boost::shared_ptr<ServerPlayer> player, bool force)
{
    if (mAddRequests.empty())
        return 0;

    int32_t playerX = static_cast<int32_t>(player->mX);
    int32_t playerZ = static_cast<int32_t>(player->mZ);

    if (!force)
    {
        RequestVector::iterator request = mAddRequests.begin();
        while (request != mAddRequests.end())
        {
            if (request->player.get() == player.get())
                return static_cast<long long>(System::processTimeInMilliSecs() - player->mChunkRequestTime);
            ++request;
        }
        return 0;
    }

    RequestVector::iterator best = mAddRequests.end();
    bool haveBest = false;
    int bestDistance = 0;
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get())
        {
            int dx = (request->x * 16 + 8) - playerX;
            int dz = (request->z * 16 + 8) - playerZ;
            int distance = dx * dx + dz * dz;
            if (!haveBest || distance < bestDistance)
            {
                bestDistance = distance;
                best = request;
                haveBest = true;
            }
        }
        ++request;
    }

    if (haveBest)
    {
        player->mChunkRequestTime = System::processTimeInMilliSecs();
        PlayerChunkContainer* chunk = this->getOrCreateChunk(best->x, best->z);
        chunk->add(player, best->add);
        mAddRequests.erase(best);
    }
    return 0;
}

bool PlayerChunkMap::chunkInRange(int x, int z, int centerX, int centerZ)
{
    return x - centerX >= -mRadius && x - centerX <= mRadius &&
           z - centerZ >= -mRadius && z - centerZ <= mRadius;
}

bool PlayerChunkMap::shouldActuallyDropChunks()
{
    return mShouldDropChunks;
}

void PlayerChunkMap::setViewDistance(int distance)
{
    const int newDistance = Mth::clamp(distance, 3, 15);
    const int oldDistance = mViewDistance;
    if (newDistance != oldDistance)
    {
        PlayerVector players(mPlayers);
        for (PlayerVector::iterator it = players.begin(); it != players.end(); ++it)
        {
            boost::shared_ptr<ServerPlayer> player = *it;
            const int chunkX = static_cast<int32_t>(player->mX) >> 4;
            const int chunkZ = static_cast<int32_t>(player->mZ) >> 4;
            if (newDistance > oldDistance)
            {
                for (int x = chunkX - newDistance; x <= chunkX + newDistance; ++x)
                {
                    for (int z = chunkZ - newDistance; z <= chunkZ + newDistance; ++z)
                    {
                        PlayerChunkContainer* chunk = getOrCreateChunk(x, z);
                        if (!chunk->hasPlayer(player)) chunk->add(player, true);
                    }
                }
            }
            else
            {
                for (int x = chunkX - oldDistance; x <= chunkX + oldDistance; ++x)
                    for (int z = chunkZ - oldDistance; z <= chunkZ + oldDistance; ++z)
                        if (x < chunkX - newDistance || x > chunkX + newDistance ||
                            z < chunkZ - newDistance || z > chunkZ + newDistance)
                        {
                            PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
                            if (chunk != NULL) chunk->remove(player, true);
                        }
            }
        }
        mViewDistance = newDistance;
    }
}

void PlayerChunkMap::tick()
{
    long long gameTime = mLevel->getGameTime();
    long long elapsedSinceUpdate = gameTime - mLastUpdate;
    if (elapsedSinceUpdate < 0 || elapsedSinceUpdate > 8000)
    {
        mLastUpdate = gameTime;
        ContainerVector::iterator inhabited = mBlockUpdates.begin();
        while (inhabited != mBlockUpdates.end())
        {
            (*inhabited)->updateInhabitedTime();
            ++inhabited;
        }
    }

    if (!mPauseBlockUpdatePackets)
    {
        ContainerVector::iterator it = mLoadedChunks.begin();
        while (it != mLoadedChunks.end())
        {
            (*it)->broadcastChanges(true);
            ++it;
        }
    }

    mTicksUntilNextUpdate += 25;
    if (mTicksUntilNextUpdate > 25)
        mTicksUntilNextUpdate = 25;

    PIXBeginNamedEvent(0.0f, "PlayerChunkMap tickAddRequests");
    long long budget = mTicksUntilNextUpdate;
    while (budget > 0)
    {
        uint64_t start = System::processTimeInMilliSecs();

        boost::shared_ptr<ServerPlayer> bestPlayer;
        bool haveBest = false;
        long long bestWait = 0;
        RequestVector::const_iterator request = mAddRequests.begin();
        while (request != mAddRequests.end())
        {
            long long wait = tickAddRequests(request->player, false);
            if (!haveBest || bestWait < wait)
            {
                bestWait = wait;
                bestPlayer = request->player;
                haveBest = true;
            }
            ++request;
        }
        if (!haveBest)
            break;

        tickAddRequests(bestPlayer, true);

        uint64_t end = System::processTimeInMilliSecs();
        budget -= static_cast<long long>(end - start);
        mTicksUntilNextUpdate = static_cast<int32_t>(budget);
    }
    PIXEndNamedEvent();
}

long long PlayerChunkMap::getKeyForChunk(int x, int z)
{
    return (static_cast<long long>(static_cast<unsigned int>(x + 0x7fffffff)) << 32) |
           static_cast<unsigned int>(z + 0x7fffffff);
}

PlayerChunkContainer* PlayerChunkMap::getChunkIfLoaded(int x, int z)
{
    ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
    return it == mChunks.end() ? NULL : it->second;
}

PlayerChunkContainer* PlayerChunkMap::getOrCreateChunk(int x, int z)
{
    long long key = getKeyForChunk(x, z);
    ContainerMap::iterator it = mChunks.find(key);
    if (it != mChunks.end())
        return it->second;
    PlayerChunkContainer* chunk = new PlayerChunkContainer(this, x, z);
    mChunks[key] = chunk;
    mBlockUpdates.push_back(chunk);
    return chunk;
}

PlayerChunkContainer* PlayerChunkMap::getChunkAndAddPlayer(int x, int z,
                                                            boost::shared_ptr<ServerPlayer> player,
                                                            bool send)
{
    if (player->mServer != NULL && player->mServer->mMasterGameMode != NULL &&
        Minecraft::InMiniGame(eMiniGameId_Any, true) &&
        !MasterGameMode::IsChunkWithinActiveBounds(player->mServer->mMasterGameMode, x, z))
        return NULL;
    ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
    if (it == mChunks.end())
    {
            mAddRequests.push_back(PlayerChunkAddRequest(x, z, player, send));
        return NULL;
    }
    it->second->add(player, send);
    return it->second;
}

PlayerChunkContainer* PlayerChunkMap::getChunkAndRemovePlayer(int x, int z,
                                                               boost::shared_ptr<ServerPlayer> player,
                                                               bool send)
{
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->x == x && request->z == z && request->player.get() == player.get())
        {
            mAddRequests.erase(request);
            return NULL;
        }
        ++request;
    }
    ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
    if (it == mChunks.end())
        return NULL;
    it->second->remove(player, send);
    return it->second;
}

void PlayerChunkMap::broadcastBlockUpdate(boost::shared_ptr<Packet> packet, int x, int y, int z)
{
    if (getChunkIfLoaded(x >> 4, z >> 4) != NULL)
        mBlockUpdates.push_back(getChunkIfLoaded(x >> 4, z >> 4));
}

void PlayerChunkMap::blockChanged(const BlockPos& pos)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(pos.x >> 4, pos.z >> 4);
    if (chunk != NULL)
        mBlockUpdates.push_back(chunk);
}

bool PlayerChunkMap::isTrackingBlock(int x, int y, int z)
{
    return getChunkIfLoaded(x >> 4, z >> 4) != NULL;
}

void PlayerChunkMap::prioritiseBlockChanges(int x, int y, int z)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x >> 4, z >> 4);
    if (chunk != NULL)
    {
        ContainerVector::iterator it = std::find(mBlockUpdates.begin(), mBlockUpdates.end(), chunk);
        if (it != mBlockUpdates.end())
            mBlockUpdates.erase(it);
        mBlockUpdates.insert(mBlockUpdates.begin(), chunk);
    }
}

void PlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    int32_t chunkX = static_cast<int32_t>(player->mX) >> 4;
    int32_t chunkZ = static_cast<int32_t>(player->mZ) >> 4;
    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
    const int radius = mRadius;
    player->mChunkX = chunkX;
    player->mChunkZ = chunkZ;
    player->mChunkRadius = radius;
    getChunkAndAddPlayer(chunkX, chunkZ, player, false);
    for (int ring = 1; ring <= radius; ++ring)
    {
        for (int x = chunkX - ring; x <= chunkX + ring; ++x)
        {
            getChunkAndAddPlayer(x, chunkZ - ring, player, false);
            getChunkAndAddPlayer(x, chunkZ + ring, player, false);
        }
        for (int z = chunkZ - ring + 1; z < chunkZ + ring; ++z)
        {
            getChunkAndAddPlayer(chunkX - ring, z, player, false);
            getChunkAndAddPlayer(chunkX + ring, z, player, false);
        }
    }
    if (player->mConnection != NULL)
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            chunkX - radius, chunkZ - radius, chunkX + radius, chunkZ + radius,
            true, shouldActuallyDropChunks(), false));
        player->mConnection->send(packet);
    }
    static_cast<ServerChunkCache*>(getLevel()->getChunkSource())->dontDrop(chunkX, chunkZ);
    if (std::find(mPlayers.begin(), mPlayers.end(), player) == mPlayers.end())
        mPlayers.push_back(player);
}

void PlayerChunkMap::remove(boost::shared_ptr<ServerPlayer> player)
{
    const int chunkX = static_cast<int32_t>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int32_t>(player->mLastSentZ) >> 4;
    for (int ring = 0; ring <= mRadius; ++ring)
    {
        for (int x = chunkX - ring; x <= chunkX + ring; ++x)
        {
            PlayerChunkContainer* near = getChunkIfLoaded(x, chunkZ - ring);
            if (near != NULL) near->remove(player, true);
            if (ring != 0)
            {
                near = getChunkIfLoaded(x, chunkZ + ring);
                if (near != NULL) near->remove(player, true);
            }
        }
        for (int z = chunkZ - ring + 1; z < chunkZ + ring; ++z)
        {
            PlayerChunkContainer* near = getChunkIfLoaded(chunkX - ring, z);
            if (near != NULL) near->remove(player, true);
            if (ring != 0)
            {
                near = getChunkIfLoaded(chunkX + ring, z);
                if (near != NULL) near->remove(player, true);
            }
        }
    }
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get())
            request = mAddRequests.erase(request);
        else
            ++request;
    }
    PlayerVector::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (it != mPlayers.end()) mPlayers.erase(it);
}

void PlayerChunkMap::removeOutsideArea(boost::shared_ptr<ServerPlayer> player,
                                        int minX, int minZ, int maxX, int maxZ)
{
    const int chunkX = static_cast<int32_t>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int32_t>(player->mLastSentZ) >> 4;
    for (int x = chunkX - mRadius; x <= chunkX + mRadius; ++x)
        for (int z = chunkZ - mRadius; z <= chunkZ + mRadius; ++z)
            if (x < minX || x > maxX || z < minZ || z > maxZ)
            {
                PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
                if (chunk != NULL)
                    chunk->remove(player, true);
            }
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get() &&
            (request->x < minX || request->x > maxX || request->z < minZ || request->z > maxZ))
            request = mAddRequests.erase(request);
        else
            ++request;
    }
    if (player->mConnection != NULL)
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            minX, minZ, maxX, maxZ, false, shouldActuallyDropChunks(), true));
        player->mConnection->send(packet);
    }
}

void PlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    if (std::find(mPlayers.begin(), mPlayers.end(), player) == mPlayers.end())
    {
        add(player);
        return;
    }
    const double deltaX = player->mLastSentX - player->mX;
    const double deltaZ = player->mLastSentZ - player->mZ;
    if (deltaX * deltaX + deltaZ * deltaZ < 64.0)
        return;
    const int oldX = static_cast<int32_t>(player->mLastSentX) >> 4;
    const int oldZ = static_cast<int32_t>(player->mLastSentZ) >> 4;
    const int newX = static_cast<int32_t>(player->mX) >> 4;
    const int newZ = static_cast<int32_t>(player->mZ) >> 4;
    if (oldX == newX && oldZ == newZ) return;
    for (int x = oldX - mRadius; x <= oldX + mRadius; ++x)
        for (int z = oldZ - mRadius; z <= oldZ + mRadius; ++z)
            if (!chunkInRange(x, z, newX, newZ)) getChunkAndRemovePlayer(x, z, player, true);
    for (int x = newX - mRadius; x <= newX + mRadius; ++x)
        for (int z = newZ - mRadius; z <= newZ + mRadius; ++z)
            if (!chunkInRange(x, z, oldX, oldZ)) getChunkAndAddPlayer(x, z, player, false);
    player->mChunkX = newX; player->mChunkZ = newZ;
    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
    if (player->mConnection != NULL)
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            newX - mRadius, newZ - mRadius, newX + mRadius, newZ + mRadius,
            true, shouldActuallyDropChunks(), false));
        player->mConnection->send(packet);
    }
    tickAddRequests(player, true);
}

bool PlayerChunkMap::isPlayerIn(boost::shared_ptr<ServerPlayer> player, int x, int z)
{
    RequestVector::const_iterator it = mAddRequests.begin();
    while (it != mAddRequests.end())
    {
        if (it->player == player && it->x == x && it->z == z)
            return true;
        ++it;
    }
    return false;
}

int PlayerChunkMap::convertChunkRangeToBlock(int range)
{
    return (range << 4) - 0x10;
}

void PlayerChunkMap::setRadius(int radius)
{
    if (radius == mRadius) return;
    const int oldRadius = mRadius;
    PlayerVector players(mPlayers);
    for (PlayerVector::iterator it = players.begin(); it != players.end(); ++it)
    {
        const int cx = static_cast<int32_t>((*it)->mLastSentX) >> 4;
        const int cz = static_cast<int32_t>((*it)->mLastSentZ) >> 4;
        if (radius > oldRadius)
        {
            for (int ring = oldRadius + 1; ring <= radius; ++ring)
            {
                for (int x = cx - ring; x <= cx + ring; ++x)
                {
                    getChunkAndAddPlayer(x, cz - ring, *it, true);
                    getChunkAndAddPlayer(x, cz + ring, *it, true);
                }
                for (int z = cz - ring + 1; z < cz + ring; ++z)
                {
                    getChunkAndAddPlayer(cx - ring, z, *it, true);
                    getChunkAndAddPlayer(cx + ring, z, *it, true);
                }
            }
        }
        else
        {
            for (int x = cx - oldRadius; x <= cx + oldRadius; ++x)
                for (int z = cz - oldRadius; z <= cz + oldRadius; ++z)
                    if (x < cx - radius || x > cx + radius ||
                        z < cz - radius || z > cz + radius)
                    {
                        PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
                        if (chunk != NULL)
                            chunk->remove(*it, true);
                    }
            RequestVector::iterator request = mAddRequests.begin();
            while (request != mAddRequests.end())
            {
                if (request->player.get() == it->get() &&
                    (request->x < cx - radius || request->x > cx + radius ||
                     request->z < cz - radius || request->z > cz + radius))
                    request = mAddRequests.erase(request);
                else
                    ++request;
            }
        }
        (*it)->mChunkRadius = radius;
    }
    mRadius = radius;
}

void PlayerChunkMap::removeOutsideAreaForAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    (void)minX; (void)minZ; (void)maxX; (void)maxZ;
}

void PlayerChunkMap::reloadAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    PlayerVector players(mPlayers);

    mShouldDropChunks = true;
    PlayerVector::iterator it = players.begin();
    while (it != players.end())
    {
        removeOutsideArea(*it, minX, minZ, maxX, maxZ);
        ++it;
    }
    mShouldDropChunks = false;

    int oldRadius = mRadius;
    setRadius(3);
    it = players.begin();
    while (it != players.end())
    {
        add(*it);
        ++it;
    }
    setRadius(oldRadius);
}

void PlayerChunkMap::PauseBlockUpdatePackets(bool pause)
{
    mPauseBlockUpdatePackets = pause;
}

void PlayerChunkMap::onPlayerJoin(boost::shared_ptr<ServerPlayer> player)
{
    (void)player;
}

void PlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    (void)player;
}
