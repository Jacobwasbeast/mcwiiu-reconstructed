#include "game/world/generation/StrongholdFeature.hpp"

#include <cmath>
#include <limits>

#include "game/core/CMinecraftApp.hpp"
#include "game/core/StrongholdPieces.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3i.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"

extern CMinecraftApp* g_pApp;

StrongholdFeature::StrongholdFeature()
    : StructureFeature()
{
    _init();
}

StrongholdFeature::StrongholdFeature(SuperflatConfig* config)
    : StructureFeature()
{
    _init();
    FlatGeneratorInfo info(config);
    mDistance = static_cast<double>(info.getStrongholdDistance());
    mPositions.clear();
    for (int i = 0; i < info.getStrongholdCount(); ++i)
    {
        mPositions.push_back(NULL);
    }
    mVariant = 1;
}

void StrongholdFeature::_init()
{
    mAllowedBiomes.clear();
    for (int group = 0; group < 5; ++group)
    {
        for (int index = 0; index < 8; ++index)
        {
            Biome* biome = Biome::getBiomes()[group * 8 + index];
            if (biome != NULL && biome->getDepth() > 0.0f)
            {
                mAllowedBiomes.push_back(biome);
            }
        }
    }
    mDistance = 32.0;
    mVariant = 3;
    mPositions.push_back(NULL);
    mPositionsGenerated = false;
}

StrongholdFeature::~StrongholdFeature()
{
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        delete *it;
    }
}

std::wstring StrongholdFeature::getFeatureName()
{
    return L"StrongHold";
}

void StrongholdFeature::generatePositions()
{
    restoreSavedData(level);
    Random positionRandom;
    positionRandom.setSeed(level->getSeed());
    double angle = positionRandom.nextDouble() * 6.2831854820251465;
    bool foundPosition = false;

    for (int attempt = 0; attempt < 10; ++attempt)
    {
        for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
        {
            double distance;
            if (level->getOriginalSaveVersion() < 5)
            {
                distance = (positionRandom.nextDouble() + 1.25) *
                    (static_cast<double>(positionRandom.nextInt(7)) + 5.0);
            }
            else
            {
                distance = (positionRandom.nextDouble() + 1.25) *
                    static_cast<double>(positionRandom.nextInt(4) + 3);
            }

            int x = Math::round(std::cos(angle) * distance);
            int z = Math::round(std::sin(angle) * distance);
            BlockPos* biomePosition = level->getBiomeSource()->findBiome(
                x * 16 + 8, z * 16 + 8, 112, mAllowedBiomes, &positionRandom);
            if (biomePosition != NULL)
            {
                x = biomePosition->x >> 4;
                z = biomePosition->z >> 4;
                g_pApp->AddTerrainFeaturePosition(eTerrainFeatureType_Stronghold, x, z);
                foundPosition = true;
                delete biomePosition;
            }

            delete *it;
            *it = new ChunkPos(x, z);
            int count = static_cast<int>(mPositions.size());
            angle += 6.2831854820251465 / static_cast<double>(count);
        }

        if (foundPosition)
        {
            break;
        }
    }

    if (!foundPosition && !mPositions.empty() && mPositions.front() != NULL)
    {
        g_pApp->AddTerrainFeaturePosition(eTerrainFeatureType_Stronghold,
                                          mPositions.front()->x, mPositions.front()->z);
    }
}

BlockPos* StrongholdFeature::getNearestGeneratedFeature(Level* sourceLevel,
                                                        const BlockPos& pos,
                                                        bool)
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && canPlaceFeatureInChunk(false, eFeatureType_Stronghold,
                                                   pos.x >> 4, pos.z >> 4))
    {
        return options->getClosestFeature(eFeatureType_Stronghold, pos);
    }

    level = sourceLevel;
    if (!mPositionsGenerated)
    {
        generatePositions();
        mPositionsGenerated = true;
    }

    BlockPos* nearest = NULL;
    double nearestDistance = std::numeric_limits<double>::max();
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        if (*it == NULL)
        {
            continue;
        }
        BlockPos candidate((*it)->x * 16 + 8, 32, (*it)->z * 16 + 8);
        double distance = candidate.distSqr(pos);
        if (nearest == NULL || distance < nearestDistance)
        {
            if (nearest == NULL)
            {
                nearest = new BlockPos(candidate);
            }
            else
            {
                *nearest = candidate;
            }
            nearestDistance = distance;
        }
    }
    return nearest;
}

bool StrongholdFeature::isFeatureChunk(int chunkX, int chunkZ, bool)
{
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && canPlaceFeatureInChunk(false, eFeatureType_Stronghold, chunkX, chunkZ))
    {
        return options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Stronghold, NULL);
    }

    if (!mPositionsGenerated)
    {
        generatePositions();
        mPositionsGenerated = true;
    }
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        if (options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_Stronghold, NULL))
        {
            return true;
        }
        if (*it != NULL && (*it)->x == chunkX && (*it)->z == chunkZ)
        {
            return true;
        }
    }
    return false;
}

std::vector<BlockPos>* StrongholdFeature::getGuesstimatedFeaturePositions()
{
    std::vector<BlockPos>* positions = new std::vector<BlockPos>();
    for (std::vector<ChunkPos*>::iterator it = mPositions.begin(); it != mPositions.end(); ++it)
    {
        if (*it != NULL)
        {
            positions->push_back((*it)->getMiddleBlockPosition(64));
        }
    }
    return positions;
}

StructureStart* StrongholdFeature::createStructureStart(int chunkX, int chunkZ)
{
    for (;;)
    {
        StrongholdStart* start = new StrongholdStart(level, random, chunkX, chunkZ);
        if (!start->empty() && static_cast<StrongholdPieces::StartPiece*>(start->front())->mPortalRoom != NULL)
            return start;
        delete start;
    }
}

StrongholdFeature::StrongholdStart::StrongholdStart(Level* sourceLevel, Random& sourceRandom,
                                                     int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    StrongholdPieces::resetPieces();
    StrongholdPieces::StartPiece* piece = new StrongholdPieces::StartPiece(
        0, sourceRandom, chunkX * 16 + 2, chunkZ * 16 + 2, sourceLevel);
    push_back(piece);
    piece->addChildren(piece, this, sourceRandom);
    while (!piece->mPendingChildren.empty())
    {
        int index = sourceRandom.nextInt(static_cast<int>(piece->mPendingChildren.size()));
        StructurePiece* child = piece->mPendingChildren[index];
        piece->mPendingChildren.erase(piece->mPendingChildren.begin() + index);
        child->addChildren(piece, this, sourceRandom);
    }
    calculateBoundingBox();
    moveBelowSeaLevel(sourceLevel, sourceRandom, 10);
}

StrongholdFeature::StrongholdStart::StrongholdStart()
    : StructureStart()
{
}

StrongholdFeature::StrongholdStart::~StrongholdStart()
{
}

EStructureStart StrongholdFeature::StrongholdStart::GetType()
{
    return eStructureStart_Stronghold;
}

StructureStart* StrongholdFeature::StrongholdStart::Create()
{
    return new StrongholdStart();
}
