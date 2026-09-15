#include "game/network/BlockChangeListener.hpp"
#include "game/core/MasterGameMode.hpp"
#include "game/server/MinecraftServer.hpp"
#include "game/server/ServerLevel.hpp"
#include <utility>
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/LevelChunk.hpp"

BlockChangeListener::BlockChangeListener(ServerLevel* level)
    : mLevel(level), mChunks(new boost::unordered_map<long long, Chunk>())
{
}

BlockChangeListener::~BlockChangeListener()
{
    delete mChunks;
}

long long BlockChangeListener::GetChunkKey(int x, int z)
{
    return (static_cast<long long>(x) + 0x7fffffffLL << 32) |
           (static_cast<unsigned int>(z) + 0x7fffffffU);
}

bool BlockChangeListener::HasChunk(long long key) const
{
    return mChunks->find(key) != mChunks->end();
}

const BlockChangeListener::Chunk& BlockChangeListener::GetChunk(long long key) const
{
    return mChunks->at(key);
}

int BlockChangeListener::GetChangesForChunk(int x, int z) const
{
    long long key = GetChunkKey(x, z);
    if (!HasChunk(key))
        return 0;
    return static_cast<int>(mChunks->at(key).GetChanges().size());
}

bool BlockChangeListener::ShouldLoadOnClient(int x, int z) const
{
    long long key = GetChunkKey(x, z);
    if (!HasChunk(key))
        return true;
    return mChunks->at(key).GetChangeCount() <= 20;
}

BlockChangeListener::Chunk& BlockChangeListener::GetChunk(int x, int z, bool create)
{
    long long key = GetChunkKey(x, z);
    boost::unordered_map<long long, Chunk>::iterator it = mChunks->find(key);
    if (create && it == mChunks->end())
        it = mChunks->insert(std::make_pair(key, Chunk(x, z))).first;
    return it->second;
}

int BlockChangeListener::GetBlockKey(const BlockPos& pos)
{
    return LevelChunk::PackBlockPos(pos);
}

void BlockChangeListener::blockChanged(Level*, const BlockPos& pos, const BlockState*, const BlockState* newState, int, bool)
{
    GetChunk(pos.x >> 4, pos.z >> 4, true).AddBlockChange(pos);
    if (gMinecraftServer != NULL && newState != NULL && newState->getBlock() == Blocks::AIR)
        gMinecraftServer->mMasterGameMode->BlockDestroyedAt(pos);
}

void BlockChangeListener::invalidateChunksOutsideArea(int minChunkX, int minChunkZ, int maxChunkX, int maxChunkZ)
{
    boost::unordered_map<long long, Chunk>::iterator it = mChunks->begin();
    while (it != mChunks->end())
    {
        const Chunk& chunk = it->second;
        if (chunk.GetX() < minChunkX || chunk.GetX() > maxChunkX || chunk.GetZ() < minChunkZ || chunk.GetZ() > maxChunkZ)
            it = mChunks->erase(it);
        else
            ++it;
    }
}

void BlockChangeListener::allChanged() {}
void BlockChangeListener::blockLightChanged(const BlockPos&) {}
void BlockChangeListener::setBlocksDirty(int, int, int, int, int, int, Level*) {}
void BlockChangeListener::playStreamingMusic(const SoundEvent*, const BlockPos&, bool) {}
void BlockChangeListener::playSound(boost::shared_ptr<Player>, const SoundEvent*, SoundSource::ESoundSource, double, double, double, float, float, float) {}
void BlockChangeListener::addParticle(Level*, ePARTICLE_TYPE, bool, double, double, double, double, double, double, arrayWithLength<int>) {}
void BlockChangeListener::addParticle(Level*, ePARTICLE_TYPE, bool, bool, double, double, double, double, double, double, arrayWithLength<int>) {}
void BlockChangeListener::entityAdded(boost::shared_ptr<Entity>) {}
void BlockChangeListener::entityRemoved(boost::shared_ptr<Entity>) {}
void BlockChangeListener::playerRemoved(boost::shared_ptr<Entity>) {}
void BlockChangeListener::globalLevelEvent(int, const BlockPos&, int) {}
void BlockChangeListener::levelEvent(boost::shared_ptr<Player>, int, const BlockPos&, int) {}
void BlockChangeListener::destroyBlockProgress(int, const BlockPos&, int) {}

BlockChangeListener::Chunk::Chunk(int x, int z)
    : mChangeCount(0), mChanges(), mX(x), mZ(z)
{
    mChanges.reserve(20);
}

std::vector<short>& BlockChangeListener::Chunk::GetChanges()
{
    return mChanges;
}

int BlockChangeListener::Chunk::GetChangeCount()
{
    return mChangeCount;
}

bool BlockChangeListener::Chunk::HasBlockChange(short blockKey)
{
    for (std::vector<short>::const_iterator it = mChanges.begin(); it != mChanges.end(); ++it)
        if (*it == blockKey)
            return true;
    return false;
}

void BlockChangeListener::Chunk::AddBlockChange(const BlockPos& pos)
{
    if (mChangeCount < 21)
    {
        if (mChangeCount < 20)
        {
            short blockKey = static_cast<short>(BlockChangeListener::GetBlockKey(pos));
            if (HasBlockChange(blockKey))
                return;
            ++mChangeCount;
            mChanges.push_back(blockKey);
            return;
        }
        mChanges.clear();
    }
    ++mChangeCount;
}

int BlockChangeListener::Chunk::GetX() const
{
    return mX;
}

int BlockChangeListener::Chunk::GetZ() const
{
    return mZ;
}
