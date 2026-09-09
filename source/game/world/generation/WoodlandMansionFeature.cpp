#include "game/world/generation/WoodlandMansionFeature.hpp"

#include <vector>

#include "game/core/BoundingBox.hpp"
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
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/StructureManager.hpp"
#include "game/world/generation/WoodlandMansionPieces.hpp"

Rotation* fjGetSuitableRandomRotation(Random& random, LevelData* levelData, int chunkX,
                                      int chunkZ)
{
    const int halfSize = levelData->getXZSize() / 2;
    AABB* bounds = AABB::newTemp(BlockPos(-halfSize, 0, -halfSize),
                                 BlockPos(halfSize, 2, halfSize));
    const int start = random.nextInt(4);
    for (int offset = 0; offset < 4; ++offset)
    {
        Rotation* rotation = Rotation::VALUES[(start + offset) % 4];
        BlockPos origin(chunkX, 0, chunkZ);
        if (bounds->contains(origin.offset(rotation->fjRotate(BlockPos(-7, 1, -3))))
            && bounds->contains(origin.offset(rotation->fjRotate(BlockPos(-7, 1, 3))))
            && bounds->contains(origin.offset(rotation->fjRotate(BlockPos(0, 1, -3))))
            && bounds->contains(origin.offset(rotation->fjRotate(BlockPos(0, 1, 3)))))
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
    : StructureFeature(), mSpacing(0x50), mSeparation(6), mSource(source)
{
    mFeatureChunk = FjIsFeatureChunk(&gWoodlandMansionBiomeRequirements, 40,
                                     mSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

WoodlandMansionFeature::WoodlandMansionFeature(GenericOverworldLevelSource* source,
                                               SuperflatConfig* config)
    : StructureFeature(), mSource(source)
{
    FlatGeneratorInfo info(config);
    mSeparation = info.getWoodlandMansionSeparation();
    if (mSeparation < 6)
        mSeparation = 6;
    int spacing = info.getWoodlandMansionSpacing();
    mSpacing = spacing > mSeparation + 1 ? spacing : mSeparation + 1;
    mFeatureChunk = FjIsFeatureChunk(&gWoodlandMansionBiomeRequirements, 40,
                                     mSpacing, mSeparation);
    mReducedFeatureChunk = FjIsFeatureChunk::CreateReduced(mFeatureChunk);
}

WoodlandMansionFeature::~WoodlandMansionFeature()
{
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
    Random& chunkRandom = *level->getRandomFor(center.x, center.z, 0x9E7F77);
    bool result = featureChunk->isFeatureChunk(chunkRandom, level->getBiomeSource(),
                                               sizeInChunks, chunkX, chunkZ);
    result = canPlaceFeatureInChunk(result, eFeatureType_WoodlandMansion, chunkX, chunkZ);
    if (result)
        g_pApp->AddTerrainFeaturePosition((_eTerrainFeatureType)9, chunkX, chunkZ);
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL)
        result = options->isFeatureChunk(chunkX, chunkZ, eFeatureType_WoodlandMansion, NULL);
    return result;
}

bool WoodlandMansionFeature::fjIsFeatureChunk(BiomeSource*, long long, int, int, int)
{
    return false;
}

BlockPos* WoodlandMansionFeature::getNearestGeneratedFeature(Level* levelIn,
                                                              const BlockPos& pos,
                                                              bool skipExistingChunks)
{
    level = levelIn;
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
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
    : StructureStart(), mFeature(NULL), mValid(false)
{
}

WoodlandMansionFeature::WoodlandMansionStart::~WoodlandMansionStart()
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
    int height = 5;
    int x0;
    int x1;
    int z0;
    int z1;
    if (rotation->getType() == Rotation::eTYPE_None)
    {
        x0 = primer.getHighestNonAirPos(7, 7);
        x1 = primer.getHighestNonAirPos(7, 12);
        z0 = primer.getHighestNonAirPos(2, 7);
        z1 = primer.getHighestNonAirPos(2, 12);
    }
    else if (rotation->getType() == Rotation::eTYPE_Clockwise180)
    {
        x0 = primer.getHighestNonAirPos(7, 7);
        x1 = primer.getHighestNonAirPos(7, 2);
        z0 = primer.getHighestNonAirPos(2, 7);
        z1 = primer.getHighestNonAirPos(2, 2);
    }
    else
    {
        if (rotation->getType() == Rotation::eTYPE_CounterClockwise90)
            height = -5;
        x0 = primer.getHighestNonAirPos(7, 7);
        x1 = primer.getHighestNonAirPos(7, height + 7);
        z0 = primer.getHighestNonAirPos(12, 7);
        z1 = primer.getHighestNonAirPos(12, height + 7);
    }
    int minHeight = z0 < z1 ? z0 : z1;
    if (x0 < x1)
        minHeight = x0 < minHeight ? x0 : minHeight;
    int baseHeight = x0;
    if (minHeight < 60 && !source->isFlat())
    {
        mValid = false;
        return;
    }
    BlockPos origin(chunkX * 16 + 8, minHeight + 1, chunkZ * 16 + 8);
    std::vector<WoodlandMansionPieces::WoodlandMansionPiece*> pieces;
    WoodlandMansionPieces::generateMansion(
        levelIn->getLevelStorage()->getStructureManager(), origin, rotation, pieces, randomIn);
    for (std::vector<WoodlandMansionPieces::WoodlandMansionPiece*>::iterator it = pieces.begin();
         it != pieces.end(); ++it)
        push_back(*it);
    calculateBoundingBox();
    mValid = true;
}

void WoodlandMansionFeature::WoodlandMansionStart::postProcess(Level* levelIn, Random& randomIn,
                                                                BoundingBox* box)
{
    StructureStart::postProcess(levelIn, randomIn, box);
    int minX = box->x0;
    int maxX = box->x1;
    int minZ = box->z0;
    int maxZ = box->z1;
    int ground = getBoundingBox()->y0 + 1;
    if (maxX < minX)
        return;
    for (int x = minX; x <= maxX; ++x)
    {
        for (int z = minZ; z <= maxZ; ++z)
        {
            BlockPos pos(x, ground, z);
            if (levelIn->isEmptyBlock(pos) && getBoundingBox()->isInside(Vec3i(x, ground, z)))
            {
                for (std::vector<StructurePiece*>::iterator it = begin(); it != end(); ++it)
                {
                    if ((*it)->getBoundingBox()->isInside(Vec3i(x, ground, z)))
                    {
                        int y = ground - 1;
                        while (y > 1 && levelIn->isEmptyBlock(BlockPos(x, y, z)))
                            --y;
                        if (!levelIn->isEmptyBlock(BlockPos(x, y, z)))
                            levelIn->setBlock(BlockPos(x, ground, z), Blocks::stone->defaultBlockState(), 2, false);
                        break;
                    }
                }
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
