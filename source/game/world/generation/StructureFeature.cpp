#include "game/world/generation/StructureFeature.hpp"

#include <typeinfo>

#include "game/core/BoundingBox.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/data/ByteArrayTag.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/SavedData.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/LevelType.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/StructureFeatureIO.hpp"
#include "game/world/generation/StructureFeatureSavedData.hpp"
#include "game/world/generation/StructureStart.hpp"

extern CMinecraftApp* g_pApp;

StructureFeature::StructureFeature()
{
}

StructureFeature::~StructureFeature()
{
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        delete it->second;
    }
}

void StructureFeature::addFeature(Level* level, int chunkX, int chunkZ, int originChunkX,
                                  int originChunkZ, ChunkPrimer* primer)
{
    restoreSavedData(level);

    long long key = ChunkPos::asLong(chunkX, chunkZ);
    if (mStructureMap.find(key) != mStructureMap.end())
    {
        return;
    }

    random.nextInt();
    bool flatWorld = level->getLevelData()->getGeneratorType() == LevelType::FLAT;
    if (isFeatureChunk(chunkX, chunkZ, flatWorld))
    {
        StructureStart* start = createStructureStart(chunkX, chunkZ);
        mStructureMap[key] = start;
        if (start->isValid())
        {
            saveFeature(chunkX, chunkZ, start);
        }
    }
}

bool StructureFeature::postProcess(Level* level, Random& randomSource, ChunkPos* chunkPos)
{
    restoreSavedData(level);
    int minX = chunkPos->x * 16;
    int minZ = chunkPos->z * 16;
    if (mStructureMap.empty())
    {
        return false;
    }

    bool processed = false;
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid() && start->isValidForPostProcess(chunkPos) &&
            start->getBoundingBox()->intersects(minX, minZ, minX + 15, minZ + 15))
        {
            BoundingBox box(minX, minZ, minX + 15, minZ + 15);
            start->postProcess(level, randomSource, &box);
            start->notifyPostProcessAt(chunkPos);
            processed = true;
            if (start->isDirty())
            {
                saveFeature(start->getChunkX(), start->getChunkZ(), start);
            }
        }
    }
    return processed;
}

bool StructureFeature::isIntersection(int x, int z)
{
    if (mStructureMap.empty())
    {
        return false;
    }

    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid() && start->getBoundingBox()->intersects(x, z, x, z))
        {
            start->getPieces();
            for (std::vector<StructurePiece*>::iterator piece = start->begin();
                 piece != start->end(); ++piece)
            {
                if ((*piece)->getBoundingBox()->intersects(x, z, x, z))
                {
                    return true;
                }
            }
        }
    }
    return false;
}

StructureStart* StructureFeature::getStructureAt(const BlockPos& pos)
{
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid() && start->getBoundingBox()->isInside(Vec3i(pos.x, pos.y, pos.z)))
        {
            start->getPieces();
            for (std::vector<StructurePiece*>::iterator piece = start->begin();
                 piece != start->end(); ++piece)
            {
                if ((*piece)->getBoundingBox()->isInside(Vec3i(pos.x, pos.y, pos.z)))
                {
                    return start;
                }
            }
        }
    }
    return NULL;
}

bool StructureFeature::isInsideFeature(const BlockPos& pos)
{
    return getStructureAt(pos) != NULL;
}

bool StructureFeature::isInsideBoundingFeature(Level* level, const BlockPos& pos)
{
    restoreSavedData(level);
    for (boost::unordered_map<long long, StructureStart*>::iterator it = mStructureMap.begin();
         it != mStructureMap.end(); ++it)
    {
        StructureStart* start = it->second;
        if (start->isValid() && start->getBoundingBox()->isInside(Vec3i(pos.x, pos.y, pos.z)))
        {
            return true;
        }
    }
    return false;
}

std::vector<BlockPos>* StructureFeature::getGuesstimatedFeaturePositions()
{
    return NULL;
}

void StructureFeature::restoreSavedData(Level* levelToRestore)
{
    if (mSavedData || levelToRestore == NULL)
    {
        return;
    }

    boost::shared_ptr<SavedData> saved =
        levelToRestore->getSavedData(typeid(StructureFeatureSavedData), getFeatureName());
    mSavedData = boost::dynamic_pointer_cast<StructureFeatureSavedData>(saved);
    if (!mSavedData)
    {
        mSavedData.reset(new StructureFeatureSavedData(getFeatureName()));
        levelToRestore->setSavedData(getFeatureName(), mSavedData);
        return;
    }

    CompoundTag* tags = mSavedData->getFullTag();
    boost::unordered_map<std::wstring, Tag*>* entries = tags->getAllTagsMap();
    for (boost::unordered_map<std::wstring, Tag*>::iterator it = entries->begin();
         it != entries->end(); ++it)
    {
        if (it->second->getId() == 7)
        {
            StructureStart* start = StructureFeatureIO::loadStaticStart(
                static_cast<ByteArrayTag*>(it->second), levelToRestore);
            if (start != NULL)
            {
                mStructureMap[ChunkPos::asLong(start->getChunkX(), start->getChunkZ())] = start;
            }
        }
    }
}

void StructureFeature::saveFeature(int chunkX, int chunkZ, StructureStart* start)
{
    mSavedData->putFeatureTag(start->createTag(chunkX, chunkZ), chunkX, chunkZ);
    mSavedData->setDirty();
    start->clearDirty();
}

BlockPos* StructureFeature::findNearestFeaturePositionBySpacing(
    Level* levelToSearch, StructureFeature* feature, const BlockPos& pos, int spacing,
    int separation, int salt, bool skipKnownStructures, int maxDistance, bool skipExistingChunks)
{
    if (maxDistance < 0)
    {
        return NULL;
    }

    int centerX = pos.x >> 4;
    int centerZ = pos.z >> 4;
    int halfSize = levelToSearch->getLevelData()->getXZSize() / 2;
    for (int radius = 0; radius <= maxDistance; ++radius)
    {
        for (int xOffset = -radius; xOffset <= radius; ++xOffset)
        {
            for (int zOffset = -radius; zOffset <= radius; ++zOffset)
            {
                if (radius != 0 && xOffset != -radius && xOffset != radius &&
                    zOffset != -radius && zOffset != radius)
                {
                    continue;
                }

                int chunkX = centerX + xOffset;
                int chunkZ = centerZ + zOffset;
                bool flatWorld = levelToSearch->getLevelData()->getGeneratorType() == LevelType::FLAT;
                if (chunkX < -halfSize || chunkX > halfSize || chunkZ < -halfSize || chunkZ > halfSize ||
                    !feature->isFeatureChunk(chunkX, chunkZ, flatWorld))
                {
                    continue;
                }
                if (skipExistingChunks && levelToSearch->isChunkPostPostProcessed(chunkX, chunkZ))
                {
                    continue;
                }
                return new BlockPos(chunkX * 16 + 8, 64, chunkZ * 16 + 8);
            }
        }
    }
    return NULL;
}

bool StructureFeature::canPlaceFeatureInChunk(bool defaultValue, EFeatureTypes type, int chunkX, int chunkZ)
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->hasFeature(type) && chunkX > -27 && chunkZ < 27)
    {
        return options->isFeatureChunk(chunkX, chunkZ, type, NULL);
    }
    return defaultValue;
}
