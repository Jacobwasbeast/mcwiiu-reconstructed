#include "game/world/chunk/PlayerChunkContainer.hpp"

#include <limits>
#include <vector>

#include <boost/shared_ptr.hpp>

#include "game/network/ChunkVisibilityPacket.hpp"
#include "game/server/ServerChunkCache.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/chunk/PlayerChunkMap.hpp"

class ClientboundBlockUpdatePacket;
class ClientboundChunkBlocksUpdatePacket;
class BlockRegionUpdatePacket;

PlayerChunkContainer::PlayerChunkContainer(PlayerChunkMap* map, int x, int z)
    : mMap(map), mPlayers(), mUnknown0(0), mPos(x, z), mBlockChanges(new unsigned short[64]()),
      mBlockChangesCapacity(64), mChunk(NULL), mBlockChangesCount(0),
      mMinX(0), mMaxX(0), mMinY(0), mMaxY(0), mMinZ(0), mMaxZ(0),
      mUpdateCounter(0), mPrioritiseBlockChanges(false),
      mHasSentToPlayers(0), mGoingToSendBRUP(0), mReserved(0), mReserved2(0),
      mReserved3(0), mReserved4(0), mLastUpdate(0), mUnknown1(0), mUnknown2(0)
{
    mChunk = mMap->getLevel()->getChunkSource()->getChunk(x, z);
}

PlayerChunkContainer::~PlayerChunkContainer()
{
    delete[] mBlockChanges;
}

void PlayerChunkContainer::sendToPlayer(boost::shared_ptr<ServerPlayer> player)
{
    mMap->getLevel()->getTracker()->playerLoadedChunk(player, mChunk);
}

void PlayerChunkContainer::add(boost::shared_ptr<ServerPlayer> player, bool send)
{
    if (!player || hasPlayer(player))
        return;

    mPlayers.push_back(player);
    if (mPlayers.size() == 1)
        mLastUpdate = mMap->getLevel()->getGameTime();

    if (send)
        sendToPlayer(player);
    mMap->getLevel()->getChunkSource()->dontDrop(mPos.x, mPos.z);
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
    if (mPlayers.empty())
    {
        updateInhabitedTime();
        mMap->getLevel()->getChunkSource()->drop(mPos.x, mPos.z);
    }
    if (send && player && player->connection != NULL)
        player->connection->send(boost::shared_ptr<Packet>(new ChunkVisibilityPacket(mPos.x, mPos.z, false, false)));
}

void PlayerChunkContainer::broadcast(boost::shared_ptr<Packet> packet)
{
    std::vector<boost::shared_ptr<ServerPlayer> >::iterator it = mPlayers.begin();
    while (it != mPlayers.end())
    {
        if (*it && (*it)->connection != NULL)
            (*it)->connection->send(packet);
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
        mGoingToSendBRUP = 0;
        return;
    }

    Level* level = mMap->getLevel();
    if (mBlockChangesCount == 1)
    {
        BlockPos pos(mPos.x * 16 + mMinX, mMinY, mPos.z * 16 + mMinZ);
        boost::shared_ptr<Packet> packet(new ClientboundBlockUpdatePacket(level, pos));
        broadcast(packet);
    }
    else
    {
        std::vector<short> positions;
        int i;
        for (i = 0; i < mBlockChangesCount; ++i)
            positions.push_back(static_cast<short>(mBlockChanges[i]));
        boost::shared_ptr<Packet> packet(new BlockRegionUpdatePacket(
            mPos.x * 16 + mMinX, mMinY, mPos.z * 16 + mMinZ,
            mMaxX - mMinX, mMaxY - mMinY, mMaxZ - mMinZ, level, force, &positions));
        broadcast(packet);
    }

    mBlockChangesCount = 0;
    mGoingToSendBRUP = 0;
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
        mBlockChanges[mBlockChangesCount++] = packed;
    }
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
        if (*it && player && (*it)->mPlayerUID == player->mPlayerUID)
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
