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

void CustomLevelSource::prepareHeights(int, int, ChunkPrimer*)
{
}

LevelChunk* CustomLevelSource::createChunk(int, int)
{
    return NULL;
}

void CustomLevelSource::lightChunk(LevelChunk*)
{
}

void CustomLevelSource::postProcess(int, int)
{
}

bool CustomLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* CustomLevelSource::getMobsAt(MobCategory*, const BlockPos&)
{
    return NULL;
}

bool CustomLevelSource::isPosInFeature(Level*, const std::wstring& feature, const BlockPos& pos)
{
    if (!mGenerateStructures) return false;
    if (feature == L"StrongHold" && mStronghold != NULL) return mStronghold->isInsideFeature(pos);
    if (feature == L"Shipwreck" && mShipwreck != NULL) return mShipwreck->isInsideFeature(pos);
    if (feature == L"Mansion" && mMansion != NULL) return mMansion->isInsideFeature(pos);
    if (feature == L"Monument" && mMonument != NULL) return mMonument->isInsideFeature(pos);
    if (feature == L"Village" && mVillage != NULL) return mVillage->isInsideFeature(pos);
    if (feature == L"Mineshaft" && mMineshaft != NULL) return mMineshaft->isInsideFeature(pos);
    if (feature == L"Temple" && mTemple != NULL) return mTemple->isInsideFeature(pos);
    if (feature == L"Ruin" && mOceanRuin != NULL) return mOceanRuin->isInsideFeature(pos);
    return false;
}

BlockPos* CustomLevelSource::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return NULL;
}

void CustomLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}
