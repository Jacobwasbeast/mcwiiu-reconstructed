#include "game/world/biome/ForestBiome.hpp"

#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/Mth.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/BirchFeature.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/HugeMushroomFeature.hpp"
#include "game/world/generation/RoofTreeFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"

static DoublePlantFeature* sDoublePlantFeature = new DoublePlantFeature();
static PerlinSimplexNoise* sTemperatureNoise = new PerlinSimplexNoise(new Random(1234LL), 1);

ForestBiome::ForestBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Forest, properties), mType(type)
{
    if (mType == eType_Birch)
    {
        mDecorator->treesPerChunk = 6;
        mDecorator->grassPerChunk = 1;
        mDecorator->flowersPerChunk = 100;
        mCreatures.push_back(new MobSpawnerData(4, 0x2D680005ULL, 2, 3));
    }
    else
    {
        mDecorator->grassPerChunk = 10;
        mDecorator->treesPerChunk = 2;
        if (mType == eType_Normal)
            mCreatures.push_back(new MobSpawnerData(5, 0x0D6C0002ULL, 4, 4));
    }

    if (mType == eType_Flower)
        mDecorator->grassPerChunk = -999;
}

ForestBiome::~ForestBiome()
{
}

AbstractTreeFeature* ForestBiome::getTreeFeature(Random& random)
{
    if (mType == eType_Flower)
    {
        if (random.nextInt(3) > 0)
            return new RoofTreeFeature(false);
    }

    if (mType != eType_Roofed && random.nextInt(5) != 0)
    {
        if (random.nextInt(10) != 0)
            return new TreeFeature(false);
        return new BigTreeFeature(false);
    }

    return new BirchFeature(false, false);
}

FlowerBlock::Flower* ForestBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    if (mType != eType_Birch)
        return Biome::getRandomFlower(random, pos);

    double noise = sTemperatureNoise->getValue(pos.x / 48.0, pos.z / 48.0);
    double value = Mth::clamp((noise + 1.0) * 0.5, 0.0, 0.9999);
    int index = (int)(value * 10.0);
    if (index == 0)
        return FlowerBlock::Flower::DANDELION;
    return FlowerBlock::Flower::FLOWERS[index];
}

void ForestBiome::addMushrooms(Level* level, Random& random, const BlockPos& pos)
{
    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 2; ++column)
        {
            const int x = row * 4 + 9 + random.nextInt(3);
            const int z = column * 4 + 9 + random.nextInt(3);
            BlockPos origin = pos.offset(x, 0, z);
            origin = level->getHeightmapPos(origin);
            if (random.nextInt(20) == 0)
            {
                HugeMushroomFeature* mushroom = new HugeMushroomFeature();
                mushroom->place(level, random, origin);
                delete mushroom;
            }
            else
            {
                AbstractTreeFeature* tree = getTreeFeature(random);
                tree->init();
                if (tree->place(level, random, origin))
                    tree->placeFallenTrunk(level, origin, random, 3);
                delete tree;
            }
        }
    }
}

void ForestBiome::addDoublePlants(Level* level, Random& random, const BlockPos& pos, int count)
{
    while (count-- > 0)
    {
        int plant = random.nextInt(3);
        if (plant == 0)
            sDoublePlantFeature->setNextPlantType(DoublePlantBlock::Variant::byData(2));
        else if (plant == 1)
            sDoublePlantFeature->setNextPlantType(DoublePlantBlock::Variant::byData(3));
        else
            sDoublePlantFeature->setNextPlantType(DoublePlantBlock::Variant::byData(4));

        for (int attempt = 0; attempt < 3; ++attempt)
        {
            int x = random.nextInt(16);
            int z = random.nextInt(16);
            BlockPos candidate = pos.offset(x + 8, 0, z + 8);
            BlockPos heightmap = level->getHeightmapPos(candidate);
            int y = random.nextInt(heightmap.y + 32);
            if (sDoublePlantFeature->place(level, random, BlockPos(pos.x + x + 8, y, pos.z + z + 8)))
                break;
        }
    }
}

void ForestBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if (mType == eType_Flower)
        addMushrooms(level, random, pos);

    int count = random.nextInt(5) - 3;
    if (mType == eType_Birch)
        count += 2;
    addDoublePlants(level, random, pos, count);
    Biome::decorate(level, random, pos);
}

unsigned int ForestBiome::getGrassColor(const BlockPos& pos)
{
    unsigned int grass = Biome::getGrassColor(pos);
    if (mType == eType_Flower)
    {
        unsigned int modifier = Minecraft::GetInstance()->getColourTable()->getColour(
            eMinecraftColour_Grass_Roofed_Forest_Modifier);
        return ((grass & 0xFEFEFE) + (modifier & 0xFEFEFE)) >> 1;
    }
    return grass;
}

Biome::EBiomeBaseClass ForestBiome::getBaseClass()
{
    return eBiomeBaseClass_Forest;
}
