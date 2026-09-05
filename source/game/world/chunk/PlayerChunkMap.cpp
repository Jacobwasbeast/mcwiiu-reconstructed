#include "game/world/chunk/PlayerChunkMap.hpp"

#include <algorithm>

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
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (force || request->player == player)
        {
            request->player = player;
            request = mAddRequests.erase(request);
        }
        else
            ++request;
    }
    return static_cast<long long>(mAddRequests.size());
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
    mViewDistance = distance;
    mRadius = distance;
}

void PlayerChunkMap::tick()
{
    ++mTicksUntilNextUpdate;
    if (mPauseBlockUpdatePackets)
        return;
    ContainerVector::iterator it = mLoadedChunks.begin();
    while (it != mLoadedChunks.end())
    {
        if (*it == NULL)
            it = mLoadedChunks.erase(it);
        else
            ++it;
    }
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
    return getChunkIfLoaded(x, z);
}

PlayerChunkContainer* PlayerChunkMap::getChunkAndAddPlayer(int x, int z,
                                                            boost::shared_ptr<ServerPlayer> player,
                                                            bool send)
{
    PlayerChunkContainer* chunk = getOrCreateChunk(x, z);
    if (chunk != NULL)
        mAddRequests.push_back(PlayerChunkAddRequest(x, z, player, send));
    return chunk;
}

PlayerChunkContainer* PlayerChunkMap::getChunkAndRemovePlayer(int x, int z,
                                                               boost::shared_ptr<ServerPlayer> player,
                                                               bool send)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
    if (chunk != NULL)
        mAddRequests.push_back(PlayerChunkAddRequest(x, z, player, !send));
    return chunk;
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
    if (std::find(mPlayers.begin(), mPlayers.end(), player) == mPlayers.end())
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
    return range << 4;
}

void PlayerChunkMap::setRadius(int radius)
{
    mRadius = radius;
    mShouldDropChunks = radius < mViewDistance;
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
    removeOutsideAreaForAllPlayers(minX, minZ, maxX, maxZ);
    mShouldDropChunks = false;
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
