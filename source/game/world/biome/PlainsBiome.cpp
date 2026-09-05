#include "game/world/biome/PlainsBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/core/PerlinSimplexNoise.hpp"

static PerlinSimplexNoise* sTemperatureNoise = new PerlinSimplexNoise(new Random(1234LL), 1);
static DoublePlantFeature* sDoublePlantFeature = new DoublePlantFeature();

static double getPlainsTemperatureNoise(int x, int z)
{
    return sTemperatureNoise->getValue(x / 200.0, z / 200.0);
}

PlainsBiome::PlainsBiome(EBiomeIDs biomeId, bool sunflower, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Plains, properties),
      mIsSunflower(sunflower)
{
    mMonsters.push_back(new MobSpawnerData(5, 0x2D690001ULL, 2, 6));
    mMonsters.push_back(new MobSpawnerData(1, 0x2D698001ULL, 1, 3));

    mDecorator->treesPerChunk = 0;
    mDecorator->extraTreeChance = 0.05F;
    mDecorator->flowersPerChunk = 4;
    mDecorator->grassPerChunk = 10;
}

PlainsBiome::~PlainsBiome()
{
}

FlowerBlock::Flower* PlainsBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    const double temperature = getPlainsTemperatureNoise(pos.x, pos.z);
    if (temperature >= -0.8)
    {
        if (random.nextInt(3) == 0)
        {
            return FlowerBlock::Flower::DANDELION;
        }

        switch (random.nextInt(3))
        {
        case 0:
            return FlowerBlock::Flower::POPPY;
        case 1:
            return FlowerBlock::Flower::FLOWERS[FlowerBlock::Flower::FLOWER_HOUSTONIA];
        default:
            return FlowerBlock::Flower::FLOWERS[FlowerBlock::Flower::FLOWER_OXEYE_DAISY];
        }
    }

    return FlowerBlock::Flower::RED_FLOWERS[random.nextInt(4)];
}

AbstractTreeFeature* PlainsBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(3) == 0)
    {
        return new BigTreeFeature(false);
    }

    return new TreeFeature(false);
}

void PlainsBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    const double temperature = getPlainsTemperatureNoise(pos.x + 8, pos.z + 8);

    if (temperature >= -0.8)
    {
        mDecorator->flowersPerChunk = 4;
        mDecorator->grassPerChunk = 10;
        sDoublePlantFeature->setNextPlantType(DoublePlantBlock::Variant::byData(2));

        for (int i = 0; i < 7; ++i)
        {
            const int x = random.nextInt(16);
            const int z = random.nextInt(16);
            BlockPos plantPos = pos.offset(x + 8, 0, z + 8);
            plantPos = level->getHeightmapPos(plantPos);
            const int y = random.nextInt(plantPos.y + 32);
            sDoublePlantFeature->place(level, random, pos.offset(x + 8, y, z + 8));
        }
    }
    else
    {
        mDecorator->flowersPerChunk = 15;
        mDecorator->grassPerChunk = 5;
    }

    if (mIsSunflower)
    {
        sDoublePlantFeature->setNextPlantType(DoublePlantBlock::Variant::byData(0));
        for (int i = 0; i < 6; ++i)
        {
            const int x = random.nextInt(16);
            const int z = random.nextInt(16);
            BlockPos plantPos = pos.offset(x + 8, 0, z + 8);
            plantPos = level->getHeightmapPos(plantPos);
            const int y = random.nextInt(plantPos.y + 32);
            sDoublePlantFeature->place(level, random, pos.offset(x + 8, y, z + 8));
        }
    }

    Biome::decorate(level, random, pos);
}

Biome::EBiomeBaseClass PlainsBiome::getBaseClass()
{
    return eBiomeBaseClass_Plains;
}
