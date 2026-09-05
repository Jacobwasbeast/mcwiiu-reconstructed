#include "game/entity/mob/NaturalMobSpawner.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/unordered_set.hpp>
#include <string>
#include <utility>
#include <vector>

#include "game/core/Mth.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/entity/mob/MobGroupData.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/server/ServerLevel.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/WorldBorder.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/Material.hpp"

void MemSect(int section);
void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();
void PIXAddNamedCounter(const char* name);
namespace System
{
    long long processTimeInMilliSecs();
}

NaturalMobSpawner::NaturalMobSpawner(int chunkRadius, ServerLevel* level)
    : mLastSpawnTime(0), mMinChunk(0), mMaxChunk(0), mLevel(level)
{
    int radius = (chunkRadius >> 1) + ((chunkRadius < 0 && (chunkRadius & 1) != 0) ? 1 : 0);
    mMinChunk = -radius;
    mMaxChunk = radius - 1;
}

void NaturalMobSpawner::addChunkPos(int chunkX, int chunkZ)
{
    if (chunkX < mMinChunk || chunkX > mMaxChunk || chunkZ < mMinChunk || chunkZ > mMaxChunk)
        return;
    if (!mLevel->hasChunkAt(BlockPos(chunkX << 4, 0, chunkZ << 4)))
        return;
    insert(std::make_pair(ChunkPos(chunkX, chunkZ), false));
}

void NaturalMobSpawner::addEdgeChunkPos(int chunkX, int chunkZ)
{
    if (chunkX < mMinChunk || chunkX > mMaxChunk || chunkZ < mMinChunk || chunkZ > mMaxChunk)
        return;
    if (!mLevel->hasChunkAt(BlockPos(chunkX << 4, 0, chunkZ << 4)))
        return;
    ChunkPos pos(chunkX, chunkZ);
    if (find(pos) == end())
        insert(std::make_pair(pos, true));
}

BlockPos NaturalMobSpawner::getRandomPosWithin(Level* level, int chunkX, int chunkZ)
{
    LevelChunk* chunk = level->getChunk(chunkX, chunkZ);
    int x = (chunkX << 4) + level->mRandom->nextInt(16);
    int z = (chunkZ << 4) + level->mRandom->nextInt(16);
    int height = Mth::roundUp(chunk->getHeightmap(BlockPos(x, 0, z)) + 1, 16);
    if (height < 1)
        height = 256;
    return BlockPos(x, level->mRandom->nextInt(height), z);
}

bool NaturalMobSpawner::isValidEmptyUnderWaterSpawnBlock(const BlockState* state)
{
    return !state->isSolid() && !state->getMaterial()->isSolid() && state->getMaterial()->isLiquid()
        && !BaseRailBlock::isRail(state);
}

bool NaturalMobSpawner::isValidEmptySpawnBlock(const BlockState* state)
{
    return !state->isSolid() && !state->getMaterial()->isSolid() && !state->getMaterial()->isLiquid()
        && !BaseRailBlock::isRail(state);
}

bool NaturalMobSpawner::isSpawnPositionOk(int placementType, Level* level, const BlockPos& pos,
                                          int xSize, int zSize)
{
    if (!level->hasChunkAt(pos) || !level->getWorldBorder()->isWithinBounds(pos))
        return false;

    const BlockState* state = level->getBlockState(pos);
    if (placementType == 2)
    {
        return state->getMaterial()->isLiquid()
            && level->getBlockState(pos.below())->getMaterial()->isLiquid()
            && !level->getBlockState(pos.above())->isSolid();
    }

    if (placementType == 3)
    {
        MutableBlockPos mutablePos(pos.x, pos.y, pos.z);
        int solidBelow = 0;
        int i;
        for (i = 0; i < 5 && pos.y - i >= 0; ++i)
        {
            mutablePos.set(pos.x, pos.y - i, pos.z);
            if (level->getBlockState(mutablePos)->getMaterial()->isSolid())
                ++solidBelow;
        }
        bool sidesSolid = solidBelow == 5
            && level->getBlockState(pos.offset(5, 0, 0))->getMaterial()->isSolid()
            && level->getBlockState(pos.offset(-5, 0, 0))->getMaterial()->isSolid()
            && level->getBlockState(pos.offset(0, 0, 5))->getMaterial()->isSolid()
            && level->getBlockState(pos.offset(0, 0, -5))->getMaterial()->isSolid();
        return state->getMaterial()->isSolid() && sidesSolid
            && !level->getBlockState(pos.above())->isSolid();
    }

    const BlockState* below = level->getBlockState(pos.below());
    if (placementType == 4)
    {
        if (!below->isSolid() && state->getMaterial()->isSolid())
            return false;
        return below->getMaterial() != Material::AIR
            && isValidEmptyUnderWaterSpawnBlock(below)
            && isValidEmptyUnderWaterSpawnBlock(level->getBlockState(pos.above()));
    }

    return below->isSolid() && isValidEmptySpawnBlock(below)
        && isValidEmptySpawnBlock(level->getBlockState(pos.above()));
}

int NaturalMobSpawner::tick(ServerLevel* level, bool spawnFriendlies, bool spawnEnemies,
                            bool spawnPersistent)
{
    if (!spawnFriendlies && !spawnEnemies && !spawnPersistent)
        return 0;

    long long now = System::processTimeInMilliSecs();
    if (now - mLastSpawnTime >= 0 && now - mLastSpawnTime > 1000)
    {
        mLastSpawnTime = now;
        clear();
        PIXBeginNamedEvent(0.0f, "Setup chunks to poll");

        std::vector<boost::shared_ptr<Player> >* players = level->getPlayers(0, NULL);
        if (players != NULL)
        {
            std::vector<boost::shared_ptr<Player> >::iterator player = players->begin();
            for (; player != players->end(); ++player)
            {
                if (!*player)
                    continue;
                BlockPos pos(boost::static_pointer_cast<Entity>(*player));
                int chunkX = pos.x >> 4;
                int chunkZ = pos.z >> 4;
                addChunkPos(chunkX, chunkZ);
            }
        }

        PIXEndNamedEvent();
    }

    BlockPos sharedSpawn = level->getSharedSpawnPos();
    unsigned int spawned = 0;
    for (unsigned int categoryIndex = 0; categoryIndex < MobCategory::CATEGORIES.length;
         ++categoryIndex)
    {
        MobCategory* category = MobCategory::CATEGORIES.data[categoryIndex];
        if (category == NULL)
            continue;
        if ((category->isFriendly() && !spawnFriendlies)
            || (!category->isFriendly() && !spawnEnemies)
            || (category->isPersistent() && !spawnPersistent))
            continue;

        unsigned int instanceCount = 0;
        unsigned long long type = category->getEnumBaseClass();
        if (level->countInstanceOf(type, category->isSingleType(), &instanceCount, NULL)
            >= category->getMaxInstancesPerLevel())
            continue;

        int chunkCount = (int)size();
        if (chunkCount == 0)
            continue;
        int attempts = 12 / chunkCount;
        int groups = attempts / 3;
        if (groups < 1)
            groups = 1;
        int tries = attempts / groups;
        if (tries < 3)
            tries = 3;
        if (category == MobCategory::AMBIENT || category == MobCategory::PHANTOM)
            groups = tries = 1;

        iterator chunk = begin();
        for (; chunk != end(); ++chunk)
        {
            if (chunk->second)
                continue;
            BlockPos spawn = getRandomPosWithin(level, chunk->first.x, chunk->first.z);
            if (level->getBlockState(spawn)->getMaterial()->isSolid())
                continue;
            for (int group = 0; group < groups && instanceCount < (unsigned int)category->getMaxInstancesPerLevel(); ++group)
            {
                for (int attempt = 0; attempt < tries; ++attempt)
                {
                    BlockPos candidate = spawn.offset(level->mRandom->nextInt(6) - level->mRandom->nextInt(6),
                                                      level->mRandom->nextInt(1) - level->mRandom->nextInt(1),
                                                      level->mRandom->nextInt(6) - level->mRandom->nextInt(6));
                    if ((candidate.x >> 4) != chunk->first.x || (candidate.z >> 4) != chunk->first.z)
                        continue;
                    if (!level->hasChunkAt(candidate) || level->hasNearPlayer(candidate.x + 0.5, candidate.y,
                                                                                candidate.z + 0.5, 24.0))
                        continue;
                    if (candidate.distSqr(sharedSpawn) < 576.0)
                        continue;
                    int placementType = 0;
                    if (!isSpawnPositionOk(placementType, level, candidate, group, attempt))
                        continue;
                    boost::shared_ptr<Mob> mob = mMobCache.getTestMob(level, type);
                    if (!mob)
                        continue;
                    DifficultyInstance difficulty = level->getCurrentDifficultyAt(candidate);
                    mob->finalizeMobSpawn(difficulty, NULL, std::wstring());
                    if (!mob->isPersistent())
                        level->addEntity(boost::static_pointer_cast<Entity>(mob));
                    ++instanceCount;
                    ++spawned;
                }
            }
        }
    }
    return spawned;
}

void NaturalMobSpawner::spawnMobsForChunkGeneration(Level* level, Biome* biome, int x, int y,
                                                    int z, int count, Random* random)
{
    return;
}
