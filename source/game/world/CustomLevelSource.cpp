#include "game/world/CustomLevelSource.hpp"

#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/Level.hpp"
#include "game/world/chunk/LevelChunk.hpp"
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

void CustomLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
}

std::vector<Biome::MobSpawnerData*> CustomLevelSource::getMobsAt(MobCategory*, const BlockPos&)
{
    return std::vector<Biome::MobSpawnerData*>();
}

bool CustomLevelSource::isPosInFeature(Level*, const std::wstring&, const BlockPos&)
{
    return false;
}

BlockPos* CustomLevelSource::findNearestMapFeature(Level*, const std::wstring&, const BlockPos&, bool)
{
    return NULL;
}

void CustomLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int, int)
{
}
