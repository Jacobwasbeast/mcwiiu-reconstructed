#include "game/world/chunk/PlayerChunkContainer.hpp"

#include <limits>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/entity/mob/EntityTracker.hpp"
#include "game/network/BlockRegionUpdatePacket.hpp"
#include "game/network/ClientboundBlockUpdatePacket.hpp"
#include "game/network/ClientboundChunkBlocksUpdatePacket.hpp"
#include "game/network/ChunkVisibilityPacket.hpp"
#include "game/network/Packet.hpp"
#include "game/network/ServerGamePacketListenerImpl.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/server/ServerPlayer.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"

#define private public
#include "game/world/chunk/PlayerChunkMap.hpp"
#undef private

PlayerChunkContainer::PlayerChunkContainer(PlayerChunkMap* map, int x, int z)
    : mMap(map), mPlayers(), mPos(x, z), mBlockChanges(new unsigned short[64]()),
      mBlockChangesCapacity(64), mChunk(NULL), mBlockChangesCount(0),
      mMinX(0), mMaxX(0), mMinY(0), mMaxY(0), mMinZ(0), mMaxZ(0),
      mUpdateCounter(0), mPrioritiseBlockChanges(false),
      mLastUpdate(0), mHasSentToPlayers(false)
{
    mChunk = mMap->getLevel()->getChunkSource()->getOrCreateChunk(x, z, false);
}

PlayerChunkContainer::~PlayerChunkContainer()
{
    delete[] mBlockChanges;
}

void PlayerChunkContainer::sendToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    ServerLevel* level = static_cast<ServerLevel*>(mMap->getLevel());
    level->getTracker()->playerLoadedChunk(player, mChunk);
}

void PlayerChunkContainer::add(boost::shared_ptr<ServerPlayer> player, bool send)
{
    if (!player || hasPlayer(player))
        return;

    player->mTrackedChunks.insert(mPos);

    if (send && player->mConnection != NULL)
        player->mConnection->send(boost::shared_ptr<Packet>(
            new ChunkVisibilityPacket(mPos.x, mPos.z, true, false)));

    if (mPlayers.empty())
        mLastUpdate = mMap->getLevel()->getGameTime();

    mPlayers.push_back(player);
    sendToPlayer(player);
    player->mLoadedChunks.push_back(mPos);

    ServerLevel* level = static_cast<ServerLevel*>(mMap->getLevel());
    static_cast<ServerChunkCache*>(level->getChunkSource())->dontDrop(mPos.x, mPos.z);
}

bool PlayerChunkContainer::hasPlayer(boost::shared_ptr<ServerPlayer> player)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (it->get() == player.get())
            return true;
        ++it;
    }
    return false;
}

void PlayerChunkContainer::updateInhabitedTime()
{
    long long gameTime = mMap->getLevel()->getGameTime();
    if (mChunk != NULL)
        mChunk->setInhabitedTime(mChunk->getInhabitedTime() + gameTime - mLastUpdate);
    mLastUpdate = gameTime;
}

void PlayerChunkContainer::remove(boost::shared_ptr<ServerPlayer> player, bool send)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end() && it->get() != player.get())
        ++it;
    if (it == mPlayers.end())
        return;

    mPlayers.erase(it);
    player->mTrackedChunks.erase(mPos);
    if (mPlayers.empty())
    {
        updateInhabitedTime();
        ServerLevel* level = static_cast<ServerLevel*>(mMap->getLevel());
        static_cast<ServerChunkCache*>(level->getChunkSource())->drop(mPos.x, mPos.z);
    }
    if (send && player && player->mConnection != NULL)
        player->mConnection->send(boost::shared_ptr<Packet>(new ChunkVisibilityPacket(mPos.x, mPos.z, false, false)));
}

void PlayerChunkContainer::broadcast(boost::shared_ptr<Packet> packet)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it && (*it)->mConnection != NULL)
            (*it)->mConnection->send(packet);
        ++it;
    }
}

void PlayerChunkContainer::broadcast(boost::shared_ptr<BlockEntity> blockEntity)
{
    if (blockEntity)
    {
        boost::shared_ptr<Packet> packet = blockEntity->getUpdatePacket();
        if (packet)
            broadcast(packet);
    }
}

void PlayerChunkContainer::broadcastChanges(bool force)
{
    if (mUpdateCounter > 0)
        --mUpdateCounter;

    if (mBlockChangesCount == 0)
    {
        mPrioritiseBlockChanges = false;
        return;
    }

    Level* level = mMap->getLevel();
    if (mBlockChangesCount == 1)
    {
        BlockPos pos(mPos.x * 16 + mMinX, mMinY, mPos.z * 16 + mMinZ);
        boost::shared_ptr<Packet> packet(new ClientboundBlockUpdatePacket(level, pos));
        broadcast(packet);
        broadcast(level->getBlockEntity(pos));
    }
    else if (mBlockChangesCount > mBlockChangesCapacity)
    {
        if (!mPrioritiseBlockChanges && (!force || mUpdateCounter > 0))
            return;

        int minY = (mMinY / 2) * 2;
        int maxY = (mMaxY / 2) * 2 + 2;
        int width = mMaxX - mMinX + 1;
        int height = maxY - minY + 2;
        int depth = mMaxZ - mMinZ + 1;
        if (height > 256)
            height = 256;

        std::vector<short> positions;
        boost::shared_ptr<Packet> packet(new BlockRegionUpdatePacket(
            mPos.x * 16 + mMinX, minY, mPos.z * 16 + mMinZ,
            width, height, depth, level, false, &positions));
        broadcast(packet);

        ServerLevel* serverLevel = static_cast<ServerLevel*>(level);
        std::vector<boost::shared_ptr<BlockEntity> >* blockEntities =
            serverLevel->getBlockEntitiesInRegion(mPos.x * 16 + mMinX, minY,
                                                  mPos.z * 16 + mMinZ,
                                                  mPos.x * 16 + mMinX + width,
                                                  minY + height,
                                                  mPos.z * 16 + mMinZ + depth);
        std::vector<boost::shared_ptr<BlockEntity> >::iterator entity = blockEntities->begin();
        while (entity != blockEntities->end())
        {
            broadcast(*entity);
            ++entity;
        }

        mBlockChangesCount = 0;
        mPrioritiseBlockChanges = false;
        mUpdateCounter = 1;
        return;
    }
    else
    {
        std::vector<short> positions;
        int i;
        for (i = 0; i < mBlockChangesCount; ++i)
            positions.push_back(static_cast<short>(mBlockChanges[i]));

        arrayWithLength<short> changes(positions);
        boost::shared_ptr<Packet> packet(new ClientboundChunkBlocksUpdatePacket(
            mPos.x, mPos.z, changes, static_cast<unsigned char>(mBlockChangesCount), level));
        broadcast(packet);

        for (i = 0; i < mBlockChangesCount; ++i)
        {
            unsigned short packed = mBlockChanges[i];
            BlockPos pos((mPos.x << 4) | ((packed >> 12) & 15), packed & 255,
                         (mPos.z << 4) | ((packed >> 8) & 15));
            broadcast(level->getBlockEntity(pos));
        }
    }

    mBlockChangesCount = 0;
    mPrioritiseBlockChanges = false;
}

void PlayerChunkContainer::blockChanged(int x, int y, int z)
{
    if (mBlockChangesCount == 0)
    {
        mMap->mBlockUpdates.push_back(this);
        mMinX = mMaxX = x;
        mMinY = mMaxY = y;
        mMinZ = mMaxZ = z;
    }
    else
    {
        if (x < mMinX) mMinX = x;
        if (x > mMaxX) mMaxX = x;
        if (y < mMinY) mMinY = y;
        if (y > mMaxY) mMaxY = y;
        if (z < mMinZ) mMinZ = z;
        if (z > mMaxZ) mMaxZ = z;
    }

    if (mBlockChangesCount < mBlockChangesCapacity)
    {
        unsigned short packed = static_cast<unsigned short>((x << 12) | (z << 8) | y);
        int i;
        for (i = 0; i < mBlockChangesCount; ++i)
            if (mBlockChanges[i] == packed)
                return;
        mBlockChanges[mBlockChangesCount] = packed;
    }
    ++mBlockChangesCount;
}

void PlayerChunkContainer::prioritiseBlockChanges()
{
    mPrioritiseBlockChanges = true;
}

ChunkPos PlayerChunkContainer::getPos()
{
    return mPos;
}

double PlayerChunkContainer::getDistanceSqrToClosestPlayer()
{
    double result = std::numeric_limits<double>::max();
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it)
        {
            double distance = mPos.distanceToSqr(boost::static_pointer_cast<Entity>(*it));
            if (distance < result)
                result = distance;
        }
        ++it;
    }
    return result;
}

bool PlayerChunkContainer::updatePlayerReference(boost::shared_ptr<ServerPlayer> player)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it && player && (*it)->getSUID() == player->getSUID())
        {
            *it = player;
            return true;
        }
        ++it;
    }
    return false;
}

void PlayerChunkContainer::removeAllPlayers(PlayerChunkContainer* container, bool send)
{
    std::vector<boost::shared_ptr<ServerPlayer> > players(container->mPlayers);
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = players.begin();
    while (it != players.end())
    {
        container->remove(*it, send);
        ++it;
    }
}
