#include "game/world/FlatLevelSource.hpp"

#include <cstring>

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
#include "game/world/generation/ShipwreckFeature.hpp"
#include "game/world/generation/StrongholdFeature.hpp"
#include "game/world/generation/StructureFeature.hpp"
#include "game/world/generation/VillageFeature.hpp"
#include "game/world/generation/WoodlandMansionFeature.hpp"

void* XPhysicalAlloc(unsigned long size, unsigned long upperAddress, unsigned long alignment, unsigned int protect);
void XPhysicalFree(void* address);
void* XMemCpy(void* destination, const void* source, unsigned int count);

FlatLevelSource::FlatLevelSource(Level* level, long long seed, bool generateStructures, SuperflatConfig* config)
    : mLevelSize(level->getLevelData()->getXZSize()),
      mLevel(level),
      mRandom(seed),
      mPopulationRandom(seed),
      mCaves(NULL),
      mCanyons(NULL),
      mDecorations(false),
      mDungeons(false),
      mLakes(NULL),
      mLavaLakes(NULL),
      mOceanMonument(NULL),
      mElementIds(NULL),
      mElementIdCount(0)
{
    FlatGeneratorInfo generatorInfo(config);

    if (generateStructures)
    {
        mCaves = generatorInfo.getCaves();
        mCanyons = generatorInfo.getCanyons();

        if (generatorInfo.hasVillage())
        {
            mStructureFeatures[L"Village"] = generatorInfo.getVillage(mLevelSize);
        }
        if (generatorInfo.hasBiomeSpecific())
        {
            mStructureFeatures[L"Temple"] = static_cast<StructureFeature*>(static_cast<void*>(generatorInfo.getBiomeSpecific()));
            mStructureFeatures[L"Ocean_Ruin"] = new OceanRuinFeature(config);
        }
        if (generatorInfo.hasMineshaft())
        {
            mStructureFeatures[L"Mineshaft"] = generatorInfo.getMineshaft();
        }
        if (generatorInfo.hasStronghold())
        {
            mStructureFeatures[L"Stronghold"] = generatorInfo.getStronghold();
        }
        if (generatorInfo.hasOceanMonument())
        {
            mOceanMonument = generatorInfo.getOceanMonument();
            mStructureFeatures[L"Monument"] = mOceanMonument;
        }
        if (generatorInfo.hasWoodlandMansion())
        {
            mStructureFeatures[L"Mansion"] = new WoodlandMansionFeature(this, config);
        }
        if (generatorInfo.hasShipwrecks())
        {
            mStructureFeatures[L"Shipwreck"] = generatorInfo.getShipwreck(this, mOceanMonument);
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
    for (std::vector<FlatLayerInfo>::const_iterator layer = layers.begin(); layer != layers.end(); ++layer)
    {
        for (unsigned int y = layer->getStart(); y < layer->getStart() + layer->getHeight(); ++y)
        {
            layer->getBlockState()->getBlock();
        }

        if (layer->getBlockState()->getBlock() == Blocks::AIR)
        {
            airHeight += layer->getHeight();
        }
        else
        {
            seaLevel += layer->getHeight() + airHeight;
            airHeight = 0;
        }
    }
    level->setSeaLevel(seaLevel);

    mElementIdCount = 65536;
    mElementIds = new unsigned short[mElementIdCount]();
    arrayWithLength<unsigned short> elements;
    elements.data = mElementIds;
    elements.length = mElementIdCount;
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
        if (feature->first != L"Shipwreck")
        {
            feature->second->apply(mLevel, chunkX, chunkZ, &primer);
        }
    }
    StructureMap::iterator shipwreck = mStructureFeatures.find(L"Shipwreck");
    if (shipwreck != mStructureFeatures.end())
    {
        shipwreck->second->apply(mLevel, chunkX, chunkZ, &primer);
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

    long long seed = mLevel->getSeed();
    mPopulationRandom.setSeed(seed);
    long long xSeed = mPopulationRandom.nextLong() / 2 * 2 + 1;
    long long zSeed = mPopulationRandom.nextLong() / 2 * 2 + 1;
    mPopulationRandom.setSeed((static_cast<long long>(chunkX) * xSeed + static_cast<long long>(chunkZ) * zSeed) ^ seed);

    ChunkPos chunkPos(chunkX, chunkZ);
    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        bool generated = feature->second->postProcess(mLevel, mPopulationRandom, &chunkPos);
        if (feature->first == L"Village")
        {
            bool villageBiome = biome == Biome::PLAINS || biome == Biome::DESERT || biome == Biome::SAVANNA ||
                                biome == Biome::TAIGA || biome == Biome::COLD_TAIGA || biome == Biome::ICE_PLAINS;
            villageGenerated = generated && villageBiome;
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
            MonsterRoomFeature feature;
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

    g_pApp->processSchematics(mLevel->getChunk(chunkX, chunkZ));
}

bool FlatLevelSource::postProcessLoadedChunk(LevelChunk*, int, int)
{
    return false;
}

std::vector<Biome::MobSpawnerData*> FlatLevelSource::getMobsAt(MobCategory* category, const BlockPos& pos)
{
    Biome* biome = mLevel->getBiome(pos);
    if (mOceanMonument != NULL && category == MobCategory::MONSTER &&
        mOceanMonument->isInsideBoundingFeature(mLevel, pos))
    {
        return *mOceanMonument->getEnemies();
    }
    return *biome->getMobs(category);
}

BlockPos* FlatLevelSource::findNearestMapFeature(Level* level, const std::wstring& featureName, const BlockPos& pos, bool findUnexplored)
{
    StructureMap::iterator feature;

    if (featureName == L"Stronghold" && (feature = mStructureFeatures.find(featureName)) != mStructureFeatures.end())
    {
        return feature->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Stronghold" && mStructureFeatures.find(L"Stronghold") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Stronghold")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Shipwreck" && mStructureFeatures.find(L"Shipwreck") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Shipwreck")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Mansion" && mStructureFeatures.find(L"Mansion") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Mansion")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Monument" && mStructureFeatures.find(L"Monument") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Monument")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Village" && mStructureFeatures.find(L"Village") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Village")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Mineshaft" && mStructureFeatures.find(L"Mineshaft") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Mineshaft")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Temple" && mStructureFeatures.find(L"Temple") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Temple")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Buried_Treasure" && mStructureFeatures.find(L"Buried_Treasure") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Buried_Treasure")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
    }
    if (featureName == L"Ocean_Ruin" && mStructureFeatures.find(L"Ocean_Ruin") != mStructureFeatures.end())
    {
        return mStructureFeatures.find(L"Ocean_Ruin")->second->getNearestGeneratedFeature(level, pos, findUnexplored);
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
    if (mStructureFeatures.empty())
    {
        return;
    }

    for (StructureMap::iterator feature = mStructureFeatures.begin(); feature != mStructureFeatures.end(); ++feature)
    {
        feature->second->apply(mLevel, chunkX, chunkZ, NULL);
    }
}
