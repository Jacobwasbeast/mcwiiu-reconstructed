#include "game/world/FlatLevelSource.hpp"

#include <algorithm>
#include <cstring>

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/entity/mob/MobCategory.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/BiomeSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPos.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/chunk/LevelChunk.hpp"
#include "game/world/generation/Feature.hpp"
#include "game/world/generation/CanyonFeature.hpp"
#include "game/world/generation/FlatGeneratorInfo.hpp"
#include "game/world/generation/FlatLayerInfo.hpp"
#include "game/world/generation/LakeFeature.hpp"
#include "game/world/generation/LargeCaveFeature.hpp"
#include "game/world/generation/LargeFeature.hpp"
#include "game/world/generation/MineShaftFeature.hpp"
#include "game/world/generation/MonsterRoomFeature.hpp"
#include "game/world/generation/OceanMonumentFeature.hpp"
#include "game/world/generation/OceanRuinFeature.hpp"
#include "game/world/generation/RandomScatteredLargeFeature.hpp"
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"
#include "game/core/XMem.hpp"

static const std::wstring kStrongholdName(L"StrongHold");
static const std::wstring kVillageName(L"Village");
static const std::wstring kMonumentName(L"Monument");
static const std::wstring kMansionName(L"Mansion");
static const std::wstring kTempleName(L"Temple");
static const std::wstring kMineshaftName(L"Mineshaft");
static const std::wstring kBuriedTreasureName(L"BuriedTreasure");
static const std::wstring kShipwreckName(L"Shipwreck");
static const std::wstring kRuinName(L"Ruin");

FlatLevelSource::FlatLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config)
    : mRandom(),
      mPopulationRandom(),
      mOceanMonument(NULL),
      mElementIds(NULL),
      mElementIdCount(0)
{
    mLevelSize = level->getLevelData()->getXZSize();
    mLevel = level;
    mRandom = Random(seed);
    mPopulationRandom = Random(seed);
    FlatGeneratorInfo generatorInfo(config);
    mCanyons = NULL;
    mCaves = NULL;

    if (generateStructures)
    {
        mCaves = generatorInfo.getCaves();
        mCanyons = generatorInfo.getCanyons();

        if (generatorInfo.hasVillage())
        {
            mStructureFeatures[kVillageName] = generatorInfo.getVillage(mLevelSize);
        }
        if (generatorInfo.hasBiomeSpecific())
        {
            mStructureFeatures[kTempleName] = generatorInfo.getBiomeSpecific();
            mStructureFeatures[kRuinName] = new OceanRuinFeature(config);
        }
        if (generatorInfo.hasMineshaft())
        {
            mStructureFeatures[kMineshaftName] = generatorInfo.getMineshaft();
        }
        if (generatorInfo.hasStronghold())
        {
            mStructureFeatures[kStrongholdName] = generatorInfo.getStronghold();
        }
        if (generatorInfo.hasOceanMonument())
        {
            mOceanMonument = generatorInfo.getOceanMonument();
            mStructureFeatures[kMonumentName] = mOceanMonument;
        }
        if (generatorInfo.hasWoodlandMansion())
        {
            mStructureFeatures[kMansionName] = new WoodlandMansionFeature(this, config);
        }
        if (generatorInfo.hasShipwrecks())
        {
            mStructureFeatures[kShipwreckName] = generatorInfo.getShipwreck(this, mOceanMonument);
        }
    }

    mDecorations = generatorInfo.hasDecorations();
    mDungeons = generatorInfo.hasDungeons();
    mLakes = generatorInfo.getLakes();
    mLavaLakes = generatorInfo.getLavaLakes();

    std::memset(mLayers, 0, sizeof(mLayers));
    generatorInfo.getLayers(mLayers);

    std::vector<FlatLayerInfo> layers;
    generatorInfo.getLayers(layers);
    int seaLevel = 0;
    int airHeight = 0;
    for (std::vector<FlatLayerInfo>::iterator it = layers.begin(); it != layers.end(); ++it)
    {
        FlatLayerInfo layer = *it;
        for (int y = layer.getStart(); y < static_cast<int>(layer.getStart() + layer.getHeight()); ++y)
        {
            layer.getBlockState()->getBlock();
        }

        if (layer.getBlockState()->getBlock() == Blocks::air)
        {
            airHeight += layer.getHeight();
        }
        else
        {
            seaLevel += layer.getHeight() + airHeight;
            airHeight = 0;
        }
    }
    level->setSeaLevel(seaLevel);

    arrayWithLength<unsigned short> elements(65536, true);
    mElementIds = elements.data;
    mElementIdCount = elements.length;
    ChunkPrimer primer(false, elements);
    for (int y = 0; y < 128; ++y)
    {
        if (mLayers[y] != NULL)
        {
            for (int x = 0; x < 16; ++x)
            {
                for (int z = 0; z < 16; ++z)
                {
                    primer.setState(x, y, z, mLayers[y]);
                }
            }
        }
    }
}

FlatLevelSource::~FlatLevelSource()
{
    delete mCaves;
    delete mCanyons;

    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        delete feature->second;
    }

    delete mLakes;
    delete mLavaLakes;
    delete[] mElementIds;
}

void FlatLevelSource::prepareHeights(int, int, ChunkPrimer* primer)
{
    std::memcpy(primer->getElementIds().data, mElementIds, mElementIdCount * sizeof(unsigned short));
}

bool FlatLevelSource::isFlat()
{
    return true;
}

LevelChunk* FlatLevelSource::createChunk(int chunkX, int chunkZ)
{
    unsigned short* elementIds = static_cast<unsigned short*>(XPhysicalAlloc(mElementIdCount * sizeof(unsigned short), 0xFFFFFFFF, 0x1000, 4));
    XMemCpy(elementIds, mElementIds, mElementIdCount * sizeof(unsigned short));

    arrayWithLength<unsigned short> elements;
    elements.data = elementIds;
    elements.length = mElementIdCount;
    ChunkPrimer primer(false, elements);

    if (mCaves != NULL)
    {
        mCaves->apply(mLevel, chunkX, chunkZ, &primer);
    }
    if (mCanyons != NULL)
    {
        mCanyons->apply(mLevel, chunkX, chunkZ, &primer);
    }

    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        if (feature->first != kShipwreckName)
        {
            feature->second->apply(mLevel, chunkX, chunkZ, &primer);
        }
    }
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        if (feature->first == kShipwreckName)
        {
            feature->second->apply(mLevel, chunkX, chunkZ, &primer);
        }
    }

    LevelChunk* chunk = new LevelChunk(mLevel, &primer, chunkX, chunkZ);
    XPhysicalFree(elementIds);

    arrayWithLength<Biome*> biomes;
    mLevel->getBiomeSource()->getBiomeBlock(biomes, chunkX << 4, chunkZ << 4, 16, 16, true);
    arrayWithLength<unsigned char> biomeIds = chunk->getBiomes();
    for (unsigned int index = 0; index < biomeIds.length; ++index)
    {
        biomeIds.data[index] = static_cast<unsigned char>(biomes.data[index]->id);
    }
    delete[] biomes.data;

    chunk->recalcHeightmap();
    return chunk;
}

void FlatLevelSource::postProcess(int chunkX, int chunkZ)
{
    BlockPos origin(chunkX * 16, 0, chunkZ * 16);
    Biome* biome = mLevel->getBiome(BlockPos(chunkX * 16 + 16, 0, chunkZ * 16 + 16));
    bool villageGenerated = false;

    mPopulationRandom.setSeed(mLevel->getSeed());
    long long xSeed = mPopulationRandom.nextLong() / 2 * 2 + 1;
    long long zSeed = mPopulationRandom.nextLong() / 2 * 2 + 1;
    mPopulationRandom.setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed) ^
                              mLevel->getSeed());

    ChunkPos chunkPos(chunkX, chunkZ);
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        bool generated = feature->second->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (dynamic_cast<VillageFeature*>(feature->second) != NULL)
        {
            std::vector<Biome*>::iterator allowed =
                std::find(VillageFeature::allowedBiomes.begin(), VillageFeature::allowedBiomes.end(), biome);
            villageGenerated = villageGenerated || (generated && allowed != VillageFeature::allowedBiomes.end());
        }
    }

    if (mLakes != NULL && !villageGenerated && mPopulationRandom.nextInt(4) == 0)
    {
        BlockPos pos = origin.offset(mPopulationRandom.nextInt(16) + 8,
                                     mPopulationRandom.nextInt(128),
                                     mPopulationRandom.nextInt(16) + 8);
        mLakes->place(mLevel, mPopulationRandom, pos);
    }

    if (mLavaLakes != NULL && !villageGenerated && mRandom.nextInt(8) == 0)
    {
        BlockPos pos = origin.offset(mPopulationRandom.nextInt(16) + 8,
                                     mPopulationRandom.nextInt(mPopulationRandom.nextInt(120) + 8),
                                     mPopulationRandom.nextInt(16) + 8);
        if (pos.y < mLevel->getSeaLevel() || mPopulationRandom.nextInt(10) == 0)
        {
            mLavaLakes->place(mLevel, mPopulationRandom, pos);
        }
    }

    if (mDungeons)
    {
        for (int index = 0; index < 8; ++index)
        {
            MonsterRoomFeature feature = MonsterRoomFeature();
            BlockPos pos = origin.offset(mPopulationRandom.nextInt(16) + 8,
                                         mPopulationRandom.nextInt(128),
                                         mPopulationRandom.nextInt(16) + 8);
            feature.place(mLevel, mPopulationRandom, pos);
        }
    }

    if (mDecorations)
    {
        biome->decorate(mLevel, mPopulationRandom, origin);
    }

    gApp.processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

bool FlatLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*>* FlatLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    if (mOceanMonument != NULL && category == MobCategory::MONSTER &&
        mOceanMonument->isInsideBoundingFeature(mLevel, pos))
    {
        return mOceanMonument->getEnemies();
    }
    return biome->getMobs(category);
}

BlockPos* FlatLevelSource::findNearestMapFeature(Level* level, const std::wstring& featureName, const BlockPos& pos, bool findUnexplored)
{
    if (kStrongholdName == featureName && mStructureFeatures.find(kStrongholdName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kStrongholdName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kStrongholdName == featureName && mStructureFeatures.find(kStrongholdName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kStrongholdName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kShipwreckName == featureName && mStructureFeatures.find(kShipwreckName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kShipwreckName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kMansionName == featureName && mStructureFeatures.find(kMansionName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kMansionName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kMonumentName == featureName && mStructureFeatures.find(kMonumentName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kMonumentName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kVillageName == featureName && mStructureFeatures.find(kVillageName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kVillageName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kMineshaftName == featureName && mStructureFeatures.find(kMineshaftName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kMineshaftName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kTempleName == featureName && mStructureFeatures.find(kTempleName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kTempleName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kBuriedTreasureName == featureName && mStructureFeatures.find(kBuriedTreasureName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kBuriedTreasureName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (kRuinName == featureName && mStructureFeatures.find(kRuinName) != mStructureFeatures.end())
    {
        return mStructureFeatures.find(kRuinName)->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    return NULL;
}

bool FlatLevelSource::isPosInFeature(Level*, const std::wstring& featureName, const BlockPos& pos)
{
    StructureMap::iterator feature = mStructureFeatures.find(featureName);
    if (feature != mStructureFeatures.end())
    {
        return feature->second->isInsideFeature(pos);
    }
    return false;
}

void FlatLevelSource::recreateLogicStructuresForChunk(LevelChunk*, int chunkX, int chunkZ)
{
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        feature->second->apply(mLevel, chunkX, chunkZ, NULL);
    }
}
