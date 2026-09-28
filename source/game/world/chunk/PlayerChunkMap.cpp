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
#include "game/server/ServerLevel.hpp"
#include "game/world/generation/PlayerList.hpp"
#include "game/network/INetworkPlayer.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/PlayerChunkContainer.hpp"

PlayerChunkMap::PlayerChunkMap(ServerLevel* level, int dimension, int viewDistance)
{
    mLevel = (Level*)level;
    mViewDistance = viewDistance;
    mPauseBlockUpdatePackets = false;
    setViewDistance(level->getServer()->getPlayers()->getViewDistance());
    mLastUpdate = 0;
    mTicksUntilNextUpdate = 0;
    mShouldDropChunks = false;
    mDimension = dimension;
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
    mChangedChunks.clear();
    mAllChunks.clear();
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
    return x - centerX >= -mViewDistance && x - centerX <= mViewDistance &&
           z - centerZ >= -mViewDistance && z - centerZ <= mViewDistance;
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
                for (int x = chunkX - mViewDistance; x <= chunkX + mViewDistance; ++x)
                    for (int z = chunkZ - mViewDistance; z <= chunkZ + mViewDistance; ++z)
                        if (!chunkInRange(x, z, chunkX, chunkZ))
                            getOrCreateChunk(x, z)->remove(player, true);
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
        ContainerVector::iterator inhabited = mAllChunks.begin();
        while (inhabited != mAllChunks.end())
        {
            (*inhabited)->updateInhabitedTime();
            ++inhabited;
        }
    }

    if (!mPauseBlockUpdatePackets)
    {
        bool sentRegionUpdate = false;
        for (unsigned int i = 0; i < mChangedChunks.size();)
        {
            sentRegionUpdate |= mChangedChunks[i]->broadcastChanges(!sentRegionUpdate);
            if (mChangedChunks[i]->mBlockChangesCount == 0)
            {
                mChangedChunks[i] = mChangedChunks.back();
                mChangedChunks.pop_back();
            }
            else
            {
                ++i;
            }
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
    mAllChunks.push_back(chunk);
    return chunk;
}

PlayerChunkContainer* PlayerChunkMap::getChunkAndAddPlayer(int x, int z,
                                                            boost::shared_ptr<ServerPlayer> player,
                                                            bool send)
{
    MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
    if (Minecraft::InMiniGame(eMiniGameId_Any, true) && gameMode->IsRoundPlaying()
        && !MasterGameMode::IsChunkWithinActiveBounds(gameMode, x, z))
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

void PlayerChunkMap::getChunkAndRemovePlayer(int x, int z,
                                                               boost::shared_ptr<ServerPlayer> player,
                                                               bool send)
{
    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->x == x && request->z == z && request->player.get() == player.get())
        {
            mAddRequests.erase(request);
            return;
        }
        ++request;
    }
    ContainerMap::iterator it = mChunks.find(getKeyForChunk(x, z));
    if (it != mChunks.end())
    {
        it->second->remove(player, send);
    }
}

void PlayerChunkMap::broadcastBlockUpdate(boost::shared_ptr<Packet> packet, int x, int y, int z)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x >> 4, z >> 4);
    if (chunk != NULL)
    {
        chunk->broadcast(packet);
    }
}

void PlayerChunkMap::blockChanged(const BlockPos& pos)
{
    int chunkX = pos.x >> 4;
    int chunkZ = pos.z >> 4;
    PlayerChunkContainer* chunk;
    if (static_cast<ServerChunkCache*>(static_cast<ServerLevel*>(mLevel)->getChunkSource())->isCreatingOnRequest())
    {
        chunk = getOrCreateChunk(chunkX, chunkZ);
    }
    else
    {
        chunk = getChunkIfLoaded(chunkX, chunkZ);
    }
    if (chunk != NULL)
    {
        chunk->blockChanged(pos.x & 15, pos.y, pos.z & 15);
    }
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
        chunk->prioritiseBlockChanges();
    }
}

void PlayerChunkMap::add(boost::shared_ptr<ServerPlayer> player)
{
    static const int DIRECTIONS[4][2] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};

    const int chunkX = static_cast<int>(player->mX) >> 4;
    const int chunkZ = static_cast<int>(player->mZ) >> 4;
    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
    getOrCreateChunk(chunkX, chunkZ)->add(player, false);

    const int viewDistance = mViewDistance;
    int offsetX = 0;
    int offsetZ = 0;
    int direction = 0;
    int minX = chunkX;
    int maxX = chunkX;
    int minZ = chunkZ;
    int maxZ = chunkZ;
    for (int length = 1; length <= viewDistance * 2; ++length)
    {
        for (int side = 0; side < 2; ++side)
        {
            const int* step = DIRECTIONS[direction++ % 4];
            for (int i = 0; i < length; ++i)
            {
                offsetX += step[0];
                offsetZ += step[1];
                const int x = chunkX + offsetX;
                const int z = chunkZ + offsetZ;
                if (x > maxX)
                    maxX = x;
                if (x < minX)
                    minX = x;
                if (z > maxZ)
                    maxZ = z;
                if (z < minZ)
                    minZ = z;
                if (length < 14 || (length < 15 && (side == 0 || i < 13)))
                    getOrCreateChunk(x, z)->add(player, false);
                else
                    getChunkAndAddPlayer(x, z, player, false);
            }
        }
    }

    direction %= 4;
    for (int i = 0; i < viewDistance * 2; ++i)
    {
        offsetX += DIRECTIONS[direction][0];
        offsetZ += DIRECTIONS[direction][1];
        const int x = chunkX + offsetX;
        const int z = chunkZ + offsetZ;
        if (x > maxX)
            maxX = x;
        if (x < minX)
            minX = x;
        if (z > maxZ)
            maxZ = z;
        if (z < minZ)
            minZ = z;
        if (viewDistance * 2 < 15)
            getOrCreateChunk(x, z)->add(player, false);
        else
            getChunkAndAddPlayer(x, z, player, false);
    }

    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL && !networkPlayer->IsLocal())
    {
        boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
            minX, maxX, minZ, maxZ, true, shouldActuallyDropChunks(), false));
        player->mConnection->send(packet);
    }

    static_cast<ServerChunkCache*>(static_cast<ServerLevel*>(getLevel())->getChunkSource())->dontDrop(chunkX, chunkZ);
    mPlayers.push_back(player);
}

void PlayerChunkMap::remove(boost::shared_ptr<ServerPlayer> player)
{
    const int chunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    for (int x = chunkX - mViewDistance; x <= chunkX + mViewDistance; ++x)
    {
        for (int z = chunkZ - mViewDistance; z <= chunkZ + mViewDistance; ++z)
        {
            PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
            if (chunk != NULL)
                chunk->remove(player, true);
        }
    }

    PlayerVector::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (it != mPlayers.end())
        mPlayers.erase(it);

    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get())
            request = mAddRequests.erase(request);
        else
            ++request;
    }
}

void PlayerChunkMap::removeOutsideArea(boost::shared_ptr<ServerPlayer> player,
                                        int minX, int minZ, int maxX, int maxZ)
{
    const int chunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int chunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    for (int x = chunkX - mViewDistance; x <= chunkX + mViewDistance; ++x)
    {
        for (int z = chunkZ - mViewDistance; z <= chunkZ + mViewDistance; ++z)
        {
            if (x < minX || x > maxX || z < minZ || z > maxZ)
            {
                PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
                if (chunk != NULL)
                    chunk->remove(player, false);
            }
        }
    }

    boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
        minX, maxX, minZ, maxZ, false, shouldActuallyDropChunks(), true));
    player->mConnection->send(packet);

    PlayerVector::iterator it = std::find(mPlayers.begin(), mPlayers.end(), player);
    if (it != mPlayers.end())
        mPlayers.erase(it);

    RequestVector::iterator request = mAddRequests.begin();
    while (request != mAddRequests.end())
    {
        if (request->player.get() == player.get())
            request = mAddRequests.erase(request);
        else
            ++request;
    }
}

void PlayerChunkMap::move(boost::shared_ptr<ServerPlayer> player)
{
    const int chunkX = static_cast<int>(player->mX) >> 4;
    const int chunkZ = static_cast<int>(player->mZ) >> 4;
    const double deltaXPos = player->mLastSentX - player->mX;
    const double deltaZPos = player->mLastSentZ - player->mZ;
    if (deltaXPos * deltaXPos + deltaZPos * deltaZPos < 64.0)
        return;

    const int lastChunkX = static_cast<int>(player->mLastSentX) >> 4;
    const int lastChunkZ = static_cast<int>(player->mLastSentZ) >> 4;
    const int deltaX = chunkX - lastChunkX;
    const int deltaZ = chunkZ - lastChunkZ;
    if (deltaX == 0 && deltaZ == 0)
        return;

    const bool inMiniGame = Minecraft::InMiniGame(eMiniGameId_Any, true);
    std::vector<std::pair<int, int> > addedChunks;
    for (int x = chunkX - mViewDistance; x <= chunkX + mViewDistance; ++x)
    {
        for (int z = chunkZ - mViewDistance; z <= chunkZ + mViewDistance; ++z)
        {
            if (!chunkInRange(x, z, lastChunkX, lastChunkZ))
            {
                MasterGameMode* gameMode = gMinecraftServer->mMasterGameMode;
                if (!Minecraft::InMiniGame(eMiniGameId_Any, true) || !gameMode->IsRoundPlaying()
                    || MasterGameMode::IsChunkWithinActiveBounds(gameMode, x, z))
                {
                    getChunkAndAddPlayer(x, z, player, false);
                    addedChunks.push_back(std::make_pair(x, z));
                }
            }

            if (!chunkInRange(x - deltaX, z - deltaZ, chunkX, chunkZ) && !inMiniGame)
                getChunkAndRemovePlayer(x - deltaX, z - deltaZ, player, true);
        }
    }

    INetworkPlayer* networkPlayer = player->mConnection->getNetworkPlayer();
    if (networkPlayer != NULL && !networkPlayer->IsLocal())
    {
        while (!addedChunks.empty())
        {
            const int firstX = addedChunks[0].first;
            const int firstZ = addedChunks[0].second;
            int sameRow = 0;
            int sameColumn = 0;
            for (unsigned int i = 0; i < addedChunks.size(); ++i)
            {
                if (addedChunks[i].second == firstZ)
                    ++sameRow;
                if (addedChunks[i].first == firstX)
                    ++sameColumn;
            }

            if (sameRow > sameColumn)
            {
                std::vector<std::pair<int, int> >::iterator it = addedChunks.begin();
                while (it != addedChunks.end())
                {
                    if (it->second == firstZ)
                        it = addedChunks.erase(it);
                    else
                        ++it;
                }
                boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
                    firstX, firstX + sameRow, firstZ, firstZ, true, shouldActuallyDropChunks(), false));
                player->mConnection->send(packet);
            }
            else
            {
                std::vector<std::pair<int, int> >::iterator it = addedChunks.begin();
                while (it != addedChunks.end())
                {
                    if (it->first == firstX)
                        it = addedChunks.erase(it);
                    else
                        ++it;
                }
                boost::shared_ptr<Packet> packet(new ChunkVisibilityAreaPacket(
                    firstX, firstX, firstZ, firstZ + sameColumn, true, shouldActuallyDropChunks(), false));
                player->mConnection->send(packet);
            }
        }
    }

    player->mLastSentX = player->mX;
    player->mLastSentZ = player->mZ;
}

bool PlayerChunkMap::isPlayerIn(boost::shared_ptr<ServerPlayer> player, int x, int z)
{
    PlayerChunkContainer* chunk = getChunkIfLoaded(x, z);
    if (chunk == NULL)
        return false;

    std::list<ChunkPos>::iterator pending =
        std::find(player->mLoadedChunks.begin(), player->mLoadedChunks.end(), chunk->getPos());
    return chunk->hasPlayer(player) && pending == player->mLoadedChunks.end();
}

int PlayerChunkMap::convertChunkRangeToBlock(int range)
{
    return (range << 4) - 0x10;
}

void PlayerChunkMap::setRadius(int radius)
{
    if (radius <= mViewDistance)
        return;

    PlayerList* players = static_cast<ServerLevel*>(mLevel)->getServer()->getPlayers();
    for (unsigned int i = 0; i < players->mPlayers.size(); ++i)
    {
        boost::shared_ptr<ServerPlayer> player = players->mPlayers[i];
        if (player->mLevel != mLevel)
            continue;

        const int chunkX = static_cast<int>(player->mX) >> 4;
        const int chunkZ = static_cast<int>(player->mZ) >> 4;
        const int oldRadius = mViewDistance;
        for (int x = chunkX - radius; x < chunkX - oldRadius; ++x)
        {
            for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
                getChunkAndAddPlayer(x, z, player, true);
        }
        for (int x = chunkX + oldRadius; x <= chunkX + radius; ++x)
        {
            for (int z = chunkZ - radius; z <= chunkZ + radius; ++z)
                getChunkAndAddPlayer(x, z, player, true);
        }
        for (int z = chunkZ - radius; z < chunkZ - oldRadius; ++z)
        {
            for (int x = chunkX - oldRadius; x < chunkX + oldRadius; ++x)
                getChunkAndAddPlayer(x, z, player, true);
        }
        for (int z = chunkZ + oldRadius; z <= chunkZ + radius; ++z)
        {
            for (int x = chunkX - oldRadius; x < chunkX + oldRadius; ++x)
                getChunkAndAddPlayer(x, z, player, true);
        }
    }
    mViewDistance = radius;
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

    int oldRadius = mViewDistance;
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
