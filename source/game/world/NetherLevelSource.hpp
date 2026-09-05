#ifndef GAME_WORLD_NETHERLEVELSOURCE_HPP
#define GAME_WORLD_NETHERLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/core/PerlinNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/generation/NetherBridgeFeature.hpp"

class BlockPos;
class ChunkPrimer;
class Feature;
class Level;
class LevelChunk;
class MobCategory;
class OreFeature;
class HellSpringFeature;
class BushFeature;
class StructureFeature;
class LargeFeature;

class NetherLevelSource : public ChunkGenerator
{
public:
    NetherLevelSource(Level* level, bool generateStructures, long long seed);
    virtual ~NetherLevelSource();

    void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer);
    virtual void lightChunk(LevelChunk* chunk);
    arrayWithLength<double> getHeights(arrayWithLength<double> values, int x, int y, int z,
                                       int xSize, int ySize, int zSize);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual void postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*> getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature,
                                             const BlockPos& pos, bool findUnexplored);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);

private:
    int mLevelHeight;
    Random mRandom;
    Random mPopulationRandom;
    bool mGenerateStructures;
    Level* mLevel;
    PerlinNoise* mNoise1;
    PerlinNoise* mNoise2;
    PerlinNoise* mNoise3;
    PerlinNoise* mNoise4;
    PerlinNoise* mNoise5;
    PerlinNoise* mNoise6;
    PerlinNoise* mNoise7;
    Feature* mFeature1;
    Feature* mFeature2;
    Feature* mFeature3;
    OreFeature* mQuartzOre;
    OreFeature* mGlowstoneOre;
    HellSpringFeature* mLavaSpring;
    HellSpringFeature* mLavaSpringOpen;
    BushFeature* mBrownMushroom;
    BushFeature* mRedMushroom;
    NetherBridgeFeature* mNetherBridgeFeature;
    LargeFeature* mCaves;
};

#endif
