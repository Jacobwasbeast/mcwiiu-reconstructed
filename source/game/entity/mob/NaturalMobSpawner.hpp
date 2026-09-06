#ifndef GAME_ENTITY_MOB_NATURALMOBSPAWNER_HPP
#define GAME_ENTITY_MOB_NATURALMOBSPAWNER_HPP

#include <boost/unordered_map.hpp>

#include "game/entity/mob/Mob.hpp"
#include "game/entity/mob/MobCache.hpp"
#include "game/world/chunk/ChunkPos.hpp"

class Biome;
class BlockPos;
class BlockState;
class Level;
class Random;
class ServerLevel;

class NaturalMobSpawner
    : public boost::unordered_map<ChunkPos, bool, ChunkPosKeyHash, ChunkPosKeyEq>
{
public:
    NaturalMobSpawner(int chunkRadius, ServerLevel* level);

    void addChunkPos(int chunkX, int chunkZ);
    void addEdgeChunkPos(int chunkX, int chunkZ);

    static BlockPos getRandomPosWithin(Level* level, int chunkX, int chunkZ);
    static bool isValidEmptyUnderWaterSpawnBlock(const BlockState* state);
    static bool isValidEmptySpawnBlock(const BlockState* state);
    static bool isSpawnPositionOk(Mob::SpawnPlacementType placementType, Level* level, const BlockPos& pos,
                                  int xSize, int zSize);

    unsigned int tick(ServerLevel* level, bool spawnFriendlies, bool spawnEnemies, bool spawnPersistent);

    static void spawnMobsForChunkGeneration(Level* level, Biome* biome, int x, int y,
                                            int z, int count, Random* random);

private:
    MobCache mMobCache;
    long long mLastSpawnTime;
    int mMinChunk;
    int mMaxChunk;
    ServerLevel* mLevel;
};

#endif
