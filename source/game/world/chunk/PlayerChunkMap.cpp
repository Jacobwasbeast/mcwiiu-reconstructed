#include "game/world/chunk/PlayerChunkMap.hpp"

#include <algorithm>

#include "game/core/Mth.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/System.hpp"
#include "game/server/ServerChunkCache.hpp"
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
        if (it->get() != NULL)
            ++*count;
        ++it;
    }
    if (removed != NULL)
        *removed = !mPlayers.empty();
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
                return 0;
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
        PlayerChunkContainer* chunk = getOrCreateChunk(best->x, best->z);
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
    int newRadius = Mth::clamp(distance, 3, 15);
    if (newRadius != mRadius)
    {
        PlayerVector players(mPlayers);
        if (!players.empty())
        {
            PlayerVector::iterator it = players.begin();
            if (newRadius - mRadius > 0)
            {
                while (it != players.end())
                {
                    boost::shared_ptr<ServerPlayer> player = *it;
                    int chunkX = static_cast<int32_t>(player->mX) >> 4;
                    int chunkZ = static_cast<int32_t>(player->mZ) >> 4;
                    for (int x = chunkX - newRadius; x <= chunkX + newRadius; ++x)
                    {
                        for (int z = chunkZ - newRadius; z <= chunkZ + newRadius; ++z)
                        {
                            PlayerChunkContainer* chunk = getOrCreateChunk(x, z);
                            if (!chunk->hasPlayer(player))
                                chunk->add(player, true);
                        }
                    }
                    ++it;
                }
                mRadius = newRadius;
                return;
            }
            while (it != players.end())
            {
                boost::shared_ptr<ServerPlayer> player = *it;
                int chunkX = static_cast<int32_t>(player->mX) >> 4;
                int chunkZ = static_cast<int32_t>(player->mZ) >> 4;
                for (int x = chunkX - mRadius; x <= chunkX + mRadius; ++x)
                {
                    for (int z = chunkZ - mRadius; z <= chunkZ + mRadius; ++z)
                    {
                        if (!chunkInRange(x, z, chunkX, chunkZ))
                        {
                            PlayerChunkContainer* chunk = getOrCreateChunk(x, z);
                            chunk->remove(player, true);
                        }
                    }
                }
                ++it;
            }
        }
        mRadius = newRadius;
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

        RequestVector::iterator best = mAddRequests.end();
        bool haveBest = false;
        long long bestWait = 0;
        RequestVector::iterator request = mAddRequests.begin();
        while (request != mAddRequests.end())
        {
            long long wait = tickAddRequests(request->player, false);
            if (!haveBest || bestWait < wait)
            {
                bestWait = wait;
                best = request;
                haveBest = true;
            }
            ++request;
        }
        if (!haveBest)
            break;

        tickAddRequests(best->player, true);

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
    static_cast<ServerChunkCache*>(getLevel()->getChunkSource())->dontDrop(chunkX, chunkZ);
    mPlayers.push_back(player);
}

void PlayerChunkMap::remove(boost::shared_ptr<ServerPlayer> player)
{
    PlayerVector::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (it != mPlayers.end())
        mPlayers.erase(it);
}

void PlayerChunkMap::removeOutsideArea(boost::shared_ptr<ServerPlayer> player,
                                        int minX, int minZ, int maxX, int maxZ)
{
    RequestVector::iterator it = mAddRequests.begin();
    while (it != mAddRequests.end())
    {
        if (it->player == player && (it->x < minX || it->x > maxX || it->z < minZ || it->z > maxZ))
            it = mAddRequests.erase(it);
        else
            ++it;
    }
}

void PlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    if (std::find(mPlayers.begin(), mPlayers.end(), player) == mPlayers.end())
        add(player);
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
    if (mRadius < radius)
        mRadius = radius;
}

void PlayerChunkMap::removeOutsideAreaForAllPlayers(int minX, int minZ, int maxX, int maxZ)
{
    PlayerVector::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        removeOutsideArea(*it, minX, minZ, maxX, maxZ);
        ++it;
    }
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
    add(player);
}

void PlayerChunkMap::onPlayerLeave(boost::shared_ptr<ServerPlayer> player)
{
    remove(player);
}
