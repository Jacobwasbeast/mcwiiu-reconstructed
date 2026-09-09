#include "game/world/generation/RandomScatteredLargeFeature.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/ScatteredFeaturePieces.hpp"

static std::vector<Biome*> gRandomScatteredLargeFeatureBiomes;

RandomScatteredLargeFeature::RandomScatteredLargeFeature()
    : StructureFeature()
{
    _init();
}

RandomScatteredLargeFeature::RandomScatteredLargeFeature(SuperflatConfig* config)
    : StructureFeature()
{
    _init();
    int spacing = (int)config->maybeOption(0x21, 0, (unsigned char)mSpacing);
    mSpacing = spacing > mSeparation + 1 ? spacing : mSeparation + 1;
}

RandomScatteredLargeFeature::~RandomScatteredLargeFeature()
{
}

void RandomScatteredLargeFeature::staticCtor()
{
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::DESERT);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::DESERT_HILLS);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::JUNGLE);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::JUNGLE_HILLS);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::SWAMPLAND);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::ICE_PLAINS);
    gRandomScatteredLargeFeatureBiomes.push_back(Biome::COLD_TAIGA);
}

void RandomScatteredLargeFeature::_init()
{
    mSpacing = 0x20;
    mSeparation = 8;
    mSwamphutEnemies.push_back(new Biome::MobSpawnerData(1, 0x5D540046ULL, 1, 1));
}

std::wstring RandomScatteredLargeFeature::getFeatureName()
{
    return std::wstring(L"Temple");
}

bool RandomScatteredLargeFeature::isFeatureChunk(int chunkX, int chunkZ, bool flatWorld)
{
    int spacing;
    if (!flatWorld)
    {
        mSpacing = 0x10;
        spacing = 0x10;
    }
    else
    {
        spacing = mSpacing;
    }
    int regionX = chunkX < 0 ? chunkX + 1 - spacing : chunkX;
    int regionZ = chunkZ < 0 ? chunkZ + 1 - spacing : chunkZ;
    Random& random = *level->getRandomFor(regionX / spacing, regionZ / spacing, 0xDB1471);
    int offsetX = random.nextInt(mSpacing - mSeparation);
    int offsetZ = random.nextInt(mSpacing - mSeparation);
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if ((options != NULL && options->isFeatureChunk(chunkX, chunkZ, eFeatureType_ScatteredFeature, NULL)) ||
        (chunkX == regionX / spacing * mSpacing + offsetX &&
         chunkZ == regionZ / spacing * mSpacing + offsetZ))
    {
        BlockPos pos(chunkX * 16 + 8, 0, chunkZ * 16 + 8);
        Biome* biome = level->getBiome(pos);
        for (std::vector<Biome*>::const_iterator it = gRandomScatteredLargeFeatureBiomes.begin();
             it != gRandomScatteredLargeFeatureBiomes.end(); ++it)
        {
            if (biome == *it)
            {
                return true;
            }
        }
    }
    return false;
}

BlockPos* RandomScatteredLargeFeature::getNearestGeneratedFeature(Level* levelIn,
                                                                    const BlockPos& pos,
                                                                    bool skipExistingChunks)
{
    level = levelIn;
    return StructureFeature::findNearestFeaturePositionBySpacing(levelIn, this, pos,
                                                                    mSpacing, mSeparation,
                                                                    0xDB1471, false, 100,
                                                                    skipExistingChunks);
}

StructureStart* RandomScatteredLargeFeature::createStructureStart(int chunkX, int chunkZ)
{
    return new ScatteredFeatureStart(level, random, chunkX, chunkZ);
}

bool RandomScatteredLargeFeature::isSwamphut(const BlockPos& pos)
{
    StructureStart* start = getStructureAt(pos);
    ScatteredFeatureStart* scattered = dynamic_cast<ScatteredFeatureStart*>(start);
    if (scattered == NULL || scattered->empty())
    {
        return false;
    }
    return dynamic_cast<ScatteredFeaturePieces::SwamplandHut*>((*scattered)[0]) != NULL;
}

std::vector<Biome::MobSpawnerData*>* RandomScatteredLargeFeature::getSwamphutEnemies()
{
    return &mSwamphutEnemies;
}

RandomScatteredLargeFeature::ScatteredFeatureStart::ScatteredFeatureStart(Level* levelIn,
                                                                            Random& randomIn,
                                                                            int chunkX,
                                                                            int chunkZ)
    : StructureStart(chunkX, chunkZ)
{
    BlockPos pos(chunkX * 16 + 8, 0, chunkZ * 16 + 8);
    Biome* biome = levelIn->getBiome(pos);
    int x = chunkX * 16;
    int z = chunkZ * 16;
    if (biome == Biome::JUNGLE || biome == Biome::JUNGLE_HILLS)
    {
        push_back(new ScatteredFeaturePieces::JunglePyramidPiece(levelIn, randomIn, x, z));
    }
    else if (biome == Biome::SWAMPLAND)
    {
        push_back(new ScatteredFeaturePieces::SwamplandHut(levelIn, randomIn, x, z));
    }
    else if (biome == Biome::DESERT || biome == Biome::DESERT_HILLS)
    {
        push_back(new ScatteredFeaturePieces::DesertPyramidPiece(levelIn, randomIn, x, z));
    }
    else if (biome == Biome::ICE_PLAINS || biome == Biome::COLD_TAIGA)
    {
        push_back(new ScatteredFeaturePieces::Igloo(levelIn, randomIn, x, z));
    }
    calculateBoundingBox();
}

RandomScatteredLargeFeature::ScatteredFeatureStart::ScatteredFeatureStart()
    : StructureStart()
{
}

RandomScatteredLargeFeature::ScatteredFeatureStart::~ScatteredFeatureStart()
{
}

EStructureStart RandomScatteredLargeFeature::ScatteredFeatureStart::GetType()
{
    return eStructureStart_ScatteredFeature;
}

StructureStart* RandomScatteredLargeFeature::ScatteredFeatureStart::Create()
{
    return new ScatteredFeatureStart();
}
