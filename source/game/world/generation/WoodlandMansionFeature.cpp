#include "game/world/generation/WoodlandMansionFeature.hpp"

#include <algorithm>
#include <vector>

#include "game/core/BoundingBox.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/Rotation.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/data/LevelStorage.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/world/GenericOverworldLevelSource.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/WoodlandMansionPieces.hpp"

Rotation* fjGetSuitableRandomRotation(Random& random, LevelData* levelData, int chunkX,
                                      int chunkZ)
{
    const int halfSize = levelData->getXZSize() / 2;
    AABB* bounds = AABB::newTemp(BlockPos(Vec3i(-halfSize, 0, -halfSize)),
                                 BlockPos(Vec3i(halfSize, 2, halfSize)));
    const int start = random.nextInt(4);
    for (int offset = 0; offset < 4; ++offset)
    {
        Rotation* rotation = Rotation::VALUES[(start + offset) % 4];
        BlockPos origin(chunkX, 0, chunkZ);
        BlockPos nearLeft = origin.offset(rotation->fjRotate(Vec3i(-7, 1, -3)));
        BlockPos nearRight = origin.offset(rotation->fjRotate(Vec3i(-7, 1, 3)));
        BlockPos farLeft = origin.offset(rotation->fjRotate(Vec3i(0, 1, -3)));
        BlockPos farRight = origin.offset(rotation->fjRotate(Vec3i(0, 1, 3)));
        if (bounds->contains(nearLeft) && bounds->contains(nearRight) &&
            bounds->contains(farLeft) && bounds->contains(farRight))
            return rotation;
    }
    return Rotation::VALUES[start];
}

static std::vector<Biome*> gWoodlandMansionSpawnBiomes;
static std::vector<Biome*> gWoodlandMansionAllowedBiomes;
static FjFeatureBiomeRequirements gWoodlandMansionBiomeRequirements;

void WoodlandMansionFeature::staticCtor()
{
    gWoodlandMansionSpawnBiomes.push_back(Biome::ROOFED_FOREST);
    gWoodlandMansionSpawnBiomes.push_back(Biome::ROOFED_FOREST_MUTATED);
    for (int i = 0; i < 256; i += 8)
    {
        for (int j = 0; j < 8; ++j)
        {
            Biome* biome = Biome::byId(i + j);
            if (biome != NULL && biome->getScale() < 0.4f)
                gWoodlandMansionAllowedBiomes.push_back(biome);
        }
    }
    gWoodlandMansionBiomeRequirements = FjFeatureBiomeRequirements(
        &gWoodlandMansionSpawnBiomes, 4, &gWoodlandMansionAllowedBiomes, 32);
}

WoodlandMansionFeature::WoodlandMansionFeature(GenericOverworldLevelSource* source)
    : StructureFeature(), mSource(source)
{
    mSpacing = 80;
    mSeparation = 6;
    mFeatureChunk = FjIsFeatureChunk(&gWoodlandMansionBiomeRequirements, 40, 80, 6);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

WoodlandMansionFeature::WoodlandMansionFeature(GenericOverworldLevelSource* source,
                                               SuperflatConfig* config)
    : StructureFeature(), mSource(source)
{
    FlatGeneratorInfo info(config);
    int separation = info.getWoodlandMansionSeparation();
    if (separation < 6)
        separation = 6;
    mSeparation = separation;
    int spacing = info.getWoodlandMansionSpacing();
    int minSpacing = separation + 1;
    if (minSpacing < spacing)
        minSpacing = spacing;
    mSpacing = minSpacing;
    mFeatureChunk = FjIsFeatureChunk(&gWoodlandMansionBiomeRequirements, 40,
                                     minSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

std::wstring WoodlandMansionFeature::getFeatureName()
{
    return L"Mansion";
}

bool WoodlandMansionFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int sizeInChunks = level->getLevelData()->getXZSize();
    FjIsFeatureChunk* featureChunk = FjIsFeatureChunk::UseReducedSpacing(sizeInChunks, flatWorld)
                                         ? &mReducedFeatureChunk : &mFeatureChunk;
    ChunkPos center = featureChunk->getCenterChunk(chunkX, chunkZ);
    bool result = featureChunk->isFeatureChunk(*level->getRandomFor(center.x, center.z, 0x9E7F77),
                                               level->getBiomeSource(), sizeInChunks, chunkX,
                                               chunkZ);
    if (result)
        gApp.AddTerrainFeaturePosition((_eTerrainFeatureType)9, chunkX, chunkZ);
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL)
        result = options->isFeatureChunk(chunkX, chunkZ, eFeatureType_WoodlandMansion, NULL);
    return result;
}

char WoodlandMansionFeature::fjIsFeatureChunk(BiomeSource*, long long, int, int, int)
{
    return false;
}

BlockPos* WoodlandMansionFeature::getNearestGeneratedFeature(Level* levelIn,
                                                              const BlockPos& pos,
                                                              bool skipExistingChunks)
{
    level = levelIn;
    BiomeSource* biomeSource = levelIn->getBiomeSource();
    if (biomeSource->isFixedBiome() && biomeSource->getFixedBiome() != Biome::ROOFED_FOREST)
        return NULL;
    LevelGenerationOptions* options = gApp.getLevelGenerationOptions();
    if (options != NULL &&
        canPlaceFeatureInChunk(false, eFeatureType_WoodlandMansion, pos.x >> 4, pos.z >> 4))
        return options->getClosestFeature(eFeatureType_WoodlandMansion, pos);
    return StructureFeature::findNearestFeaturePositionBySpacing(
        levelIn, this, pos, mSpacing, mSeparation, 0x9E7F77, true, 100,
        skipExistingChunks);
}

StructureStart* WoodlandMansionFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new WoodlandMansionStart(this, level, mSource, random, chunkX, chunkZ);
}

WoodlandMansionFeature::WoodlandMansionStart::WoodlandMansionStart(
    WoodlandMansionFeature* feature, Level* levelIn, GenericOverworldLevelSource* source,
    Random& randomIn, int chunkX, int chunkZ)
    : StructureStart(chunkX, chunkZ), mFeature(feature), mValid(false)
{
    create(levelIn, source, randomIn, chunkX, chunkZ);
}

WoodlandMansionFeature::WoodlandMansionStart::WoodlandMansionStart()
    : StructureStart()
{
}

StructureStart* WoodlandMansionFeature::WoodlandMansionStart::Create()
{
    return new WoodlandMansionStart();
}

void WoodlandMansionFeature::WoodlandMansionStart::create(
    Level* levelIn, GenericOverworldLevelSource* source, Random& randomIn,
    int chunkX, int chunkZ)
{
    Rotation* rotation = fjGetSuitableRandomRotation(randomIn, levelIn->getLevelData(),
                                                      chunkX, chunkZ);
    arrayWithLength<unsigned short> elementIds(0x10000, true);
    ChunkPrimer primer(true, elementIds);
    source->prepareHeights(chunkX, chunkZ, &primer);
    int offset = 5;
    int corner0;
    int corner1;
    int corner2;
    int corner3;
    if (rotation == Rotation::CLOCKWISE_90)
    {
        corner0 = primer.getHighestNonAirPos(7, 7);
        corner1 = primer.getHighestNonAirPos(7, 12);
        corner2 = primer.getHighestNonAirPos(2, 7);
        corner3 = primer.getHighestNonAirPos(2, 12);
    }
    else if (rotation == Rotation::CLOCKWISE_180)
    {
        corner0 = primer.getHighestNonAirPos(7, 7);
        corner1 = primer.getHighestNonAirPos(7, 2);
        corner2 = primer.getHighestNonAirPos(2, 7);
        corner3 = primer.getHighestNonAirPos(2, 2);
    }
    else
    {
        if (rotation == Rotation::COUNTERCLOCKWISE_90)
            offset = -5;
        corner0 = primer.getHighestNonAirPos(7, 7);
        corner1 = primer.getHighestNonAirPos(7, offset + 7);
        corner2 = primer.getHighestNonAirPos(12, 7);
        corner3 = primer.getHighestNonAirPos(12, offset + 7);
    }
    int low = std::min(corner0, corner1);
    int high = std::min(corner2, corner3);
    int minHeight = low < high ? low : high;
    if (minHeight < 60 && !source->isFlat())
    {
        mValid = false;
        return;
    }
    BlockPos origin(chunkX * 16 + 8, minHeight + 1, chunkZ * 16 + 8);
    std::vector<WoodlandMansionPieces::WoodlandMansionPiece*> pieces;
    WoodlandMansionPieces::generateMansion(
        levelIn->getLevelStorage()->getStructureManager(), origin, rotation, pieces, randomIn);
    mPieces.insert(mPieces.empty() ? mPieces.begin() : mPieces.end() - 1, pieces.begin(), pieces.end());
    calculateBoundingBox();
    mValid = true;
}

void WoodlandMansionFeature::WoodlandMansionStart::postProcess(Level* levelIn, Random& randomIn,
                                                                BoundingBox* box)
{
    StructureStart::postProcess(levelIn, randomIn, box);
    int y = getBoundingBox()->y0;
    for (int x = box->x0; x <= box->x1; ++x)
    {
        for (int z = box->z0; z <= box->z1; ++z)
        {
            BlockPos pos(x, y, z);
            if (levelIn->isEmptyBlock(pos) || !getBoundingBox()->isInside(pos))
                continue;

            for (std::vector<StructurePiece*>::iterator it = mPieces.begin(); it != mPieces.end(); ++it)
            {
                if (!(*it)->getBoundingBox()->isInside(pos))
                    continue;

                for (int fillY = y - 1; fillY > 1; --fillY)
                {
                    BlockPos fillPos(x, fillY, z);
                    if (!levelIn->isEmptyBlock(fillPos) && !levelIn->getBlockState(fillPos)->getMaterial()->isLiquid())
                        break;
                    levelIn->setBlock(fillPos, Blocks::cobblestone->defaultBlockState(), Block::UPDATE_CLIENTS, false);
                }
                break;
            }
        }
    }
}

bool WoodlandMansionFeature::WoodlandMansionStart::isValid()
{
    return mValid;
}

EStructureStart WoodlandMansionFeature::WoodlandMansionStart::GetType()
{
    return eStructureStart_WoodlandMansion;
}
