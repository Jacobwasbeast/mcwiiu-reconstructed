#ifndef GAME_WORLD_OVERWORLDLEVELSOURCE_HPP
#define GAME_WORLD_OVERWORLDLEVELSOURCE_HPP

#include <string>
#include <vector>

#include <boost/unordered_map.hpp>

#include "game/util/Random.hpp"
#include "game/world/GenericOverworldLevelSource.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/chunk/ChunkGenerator.hpp"

class Block;
class BlockPos;
class BlockState;
class ChunkPrimer;
class CustomizableSourceSettings;
class Level;
class LevelChunk;
class LargeFeature;
class MineShaftFeature;
class MobCategory;
class OceanMonumentFeature;
class OceanRuinFeature;
class PerlinNoise;
class PerlinSimplexNoise;
class RandomScatteredLargeFeature;
class StructureFeature;
class SuperflatConfig;

class OverworldLevelSource : public ChunkGenerator, public GenericOverworldLevelSource
{
public:
    OverworldLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config);
    virtual ~OverworldLevelSource();

    double getHeightFalloff(int x, int z, int* heights);
    void getHeights(int x, int y, int z, arrayWithLength<Biome*>& biomes, arrayWithLength<double>& heights);
    virtual void prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer);
    void buildSurfaces(int chunkX, int chunkZ, ChunkPrimer* primer, arrayWithLength<Biome*> biomes);
    virtual LevelChunk* createChunk(int chunkX, int chunkZ);
    virtual void lightChunk(LevelChunk* chunk);
    bool isWater(const BlockPos& pos);
    int calcWaterDepths(int x, int z);
    virtual void postProcess(int chunkX, int chunkZ);
    virtual void postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    virtual std::vector<Biome::MobSpawnerData*> getMobsAt(MobCategory* category, const BlockPos& pos);
    virtual bool isPosInFeature(Level* level, const std::wstring& feature, const BlockPos& pos);
    virtual BlockPos* findNearestMapFeature(Level* level, const std::wstring& feature, const BlockPos& pos, bool takeBlock);
    virtual void recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ);
    static void staticCtor();

private:
    int mLevelSize;
    Random mRandom;
    Random mPopulationRandom;
    PerlinNoise* mDepthNoise;
    PerlinNoise* mMainNoise;
    PerlinNoise* mScaleNoise;
    PerlinSimplexNoise* mTemperatureNoise;
    PerlinNoise* mForestNoise;
    PerlinNoise* mDetailNoise;
    Level* mLevel;
    bool mGenerateStructures;
    unsigned char mUnused6a[3];
    int mSeaLevel;
    float mNoiseData[40];
    float mDepthData[3];
    float mScaleData[3];
    float mTemperatureData[3];
    float mForestData[3];
    float mDetailData[3];
    int mUnused7c;
    int mUnused80;
    int mUnused84;
    int mUnused88;
    int mUnused8c;
    int mUnused90;
    int mUnused94;
    int mUnused98;
    int mUnused9c;
    int mUnuseda0;
    int mUnuseda4;
    int mUnuseda8;
    int mUnusedac;
    int mUnusedb0;
    int mUnusedb4;
    int mUnusedb8;
    int mUnusedbc;
    int mUnusedc0;
    int mUnusedc4;
    int mUnusedc8;
    int mUnusedcc;
    int mUnusedd0;
    CustomizableSourceSettings* mSettings;
    const BlockState* mDefaultBlock;
    LargeFeature* mCaves;
    StructureFeature* mStronghold;
    StructureFeature* mVillage;
    MineShaftFeature* mMineshaft;
    RandomScatteredLargeFeature* mTemple;
    LargeFeature* mRavines;
    LargeFeature* mCanyons;
    OceanMonumentFeature* mMonument;
    StructureFeature* mMansion;
    StructureFeature* mBuriedTreasure;
    StructureFeature* mShipwreck;
    OceanRuinFeature* mOceanRuin;
};

#endif
