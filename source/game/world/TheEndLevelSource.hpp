#ifndef GAME_WORLD_THEENDLEVELSOURCE_HPP
#define GAME_WORLD_THEENDLEVELSOURCE_HPP

#include <string>
#include <vector>

#include "game/core/PerlinNoise.hpp"
#include "game/core/SimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"
#include "game/world/generation/Feature.hpp"

class Biome;
class ChunkPrimer;
class Level;
class LevelChunk;
class MobCategory;

class TheEndLevelSource : public ChunkGenerator
{
public:
    TheEndLevelSource(Level* level, bool generateStructures, long long seed, const BlockPos& origin);
    virtual ~TheEndLevelSource();

    double fjIslandProximity(int x, int z);
    bool fjIslandCheck(int x, int z);
    double getIslandHeightValue(int x, int z, int xOffset, int zOffset);
    arrayWithLength<double> getHeights(arrayWithLength<double> values, int x, int y, int z, int xSize, int ySize, int zSize);
    virtual void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(ChunkPrimer* primer);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual void postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*> getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos, bool findUnexplored);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    bool isIslandChunk(int x, int z);

private:
    int mLevelHeight;
    Random* mRandom;
    Random* mPopulationRandom;
    PerlinNoise* mNoise1;
    PerlinNoise* mNoise2;
    PerlinNoise* mNoise3;
    PerlinNoise* mNoise4;
    PerlinNoise* mNoise5;
    Level* mLevel;
    bool mGenerateStructures;
    BlockPos mOrigin;
    void* mEndCityFeature;
    SimplexNoise* mIslandNoise;
    arrayWithLength<double> mNoiseField;
    arrayWithLength<Biome*> mBiomes;
};

#endif
