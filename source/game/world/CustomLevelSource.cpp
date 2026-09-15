#include "game/world/CustomLevelSource.hpp"

#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/entity/mob/MobCategory.hpp"

CustomLevelSource::~CustomLevelSource()
{
}

void CustomLevelSource::prepareHeights(int chunkX, int chunkZ, ChunkPrimer* primer)
{
    static_cast<void>(chunkX);
    static_cast<void>(chunkZ);
    static_cast<void>(primer);
}

LevelChunk* CustomLevelSource::createChunk(int chunkX, int chunkZ)
{
    static_cast<void>(chunkX);
    static_cast<void>(chunkZ);
    return NULL;
}

void CustomLevelSource::lightChunk(LevelChunk* chunk)
{
    static_cast<void>(chunk);
}

void CustomLevelSource::postProcess(int chunkX, int chunkZ)
{
    static_cast<void>(chunkX);
    static_cast<void>(chunkZ);
}

bool CustomLevelSource::postProcessLoadedChunk(LevelChunk* chunk, int chunkX, int chunkZ)
{
    static_cast<void>(chunk);
    static_cast<void>(chunkX);
    static_cast<void>(chunkZ);
    return false;
}

std::vector<Biome::MobSpawnerData*> CustomLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    static_cast<void>(category);
    static_cast<void>(pos);
    return std::vector<Biome::MobSpawnerData*>();
}

bool CustomLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    if (!mGenerateStructures) return false;
    if (feature == L"Stronghold" && mStronghold != NULL) return mStronghold->isInsideFeature(pos);
    if (feature == L"Shipwreck" && mShipwreck != NULL) return mShipwreck->isInsideFeature(pos);
    if (feature == L"Mansion" && mMansion != NULL) return mMansion->isInsideFeature(pos);
    if (feature == L"Monument" && mMonument != NULL) return mMonument->isInsideFeature(pos);
    if (feature == L"Village" && mVillage != NULL) return mVillage->isInsideFeature(pos);
    if (feature == L"Mineshaft" && mMineshaft != NULL) return mMineshaft->isInsideFeature(pos);
    if (feature == L"Temple" && mTemple != NULL) return mTemple->isInsideFeature(pos);
    if (feature == L"Ocean_Ruin" && mOceanRuin != NULL) return mOceanRuin->isInsideFeature(pos);
    return false;
}

BlockPos* CustomLevelSource::findNearestMapFeature(Level* level, const std::wstring& feature,
                                                   const BlockPos& pos, bool findUnexplored)
{
    static_cast<void>(level);
    static_cast<void>(feature);
    static_cast<void>(pos);
    static_cast<void>(findUnexplored);
    return NULL;
}

void CustomLevelSource::recreateLogicStructuresForChunk(LevelChunk* chunk, int chunkX, int chunkZ)
{
    static_cast<void>(chunk);
    static_cast<void>(chunkX);
    static_cast<void>(chunkZ);
}
