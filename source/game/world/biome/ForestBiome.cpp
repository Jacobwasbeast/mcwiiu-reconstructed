#include "game/world/biome/ForestBiome.hpp"
#include "game/core/EntityType.hpp"

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

ForestBiome::ForestBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Forest, properties), mType(type)
{
    if (mType == eType_Flower)
    {
        mDecorator->treesPerChunk = 6;
        mDecorator->grassPerChunk = 1;
        mDecorator->flowersPerChunk = 100;
        mCreatures.push_back(new MobSpawnerData(4, EntityType::RABBIT, 2, 3));
    }
    else
    {
        mDecorator->grassPerChunk = 2;
        mDecorator->treesPerChunk = 10;
        if (mType == eType_Normal)
            mWolves.push_back(new MobSpawnerData(5, EntityType::WOLF, 4, 4));
    }

    if (mType == eType_Roofed)
        mDecorator->treesPerChunk = -999;
}

ForestBiome::~ForestBiome()
{
}

AbstractTreeFeature* ForestBiome::getTreeFeature(Random& random)
{
    if (mType == eType_Roofed)
    {
        if (random.nextInt(3) > 0)
            return new RoofTreeFeature(false);
    }

    if (mType != eType_Birch && random.nextInt(5) != 0)
    {
        if (random.nextInt(10) != 0)
            return new TreeFeature(false);
        return new BigTreeFeature(false);
    }

    return new BirchFeature(false, false);
}

FlowerBlock::Flower* ForestBiome::getRandomFlower(Random& random, const BlockPos& pos)
{
    if (mType != eType_Flower)
        return Biome::getRandomFlower(random, pos);

    double noise = BIOME_INFO_NOISE->getValue(pos.x / 48.0, pos.z / 48.0);
    double value = Mth::clamp((noise + 1.0) * 0.5, 0.0, 0.9999);
    FlowerBlock::Flower* flower = FlowerBlock::Flower::FLOWERS[(int)(value * 10.0)];
    if (flower == FlowerBlock::Flower::BLUE_ORCHID)
        return FlowerBlock::Flower::POPPY;
    return flower;
}

void ForestBiome::addMushrooms(Level* level, Random& random, const BlockPos& pos)
{
    for (int row = 0; row < 4; ++row)
    {
        for (int column = 0; column < 4; ++column)
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
                    tree->postPlaceTree(level, random, origin);
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
            DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::VARIANTS[1]);
        else if (plant == 1)
            DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::VARIANTS[4]);
        else if (plant == 2)
            DOUBLE_PLANT_FEATURE->setNextPlantType(DoublePlantBlock::VARIANTS[5]);

        for (int attempt = 0; attempt < 5; ++attempt)
        {
            int x = random.nextInt(16);
            int z = random.nextInt(16);
            BlockPos candidate = pos.offset(x + 8, 0, z + 8);
            BlockPos heightmap = level->getHeightmapPos(candidate);
            int y = random.nextInt(heightmap.y + 32);
            if (DOUBLE_PLANT_FEATURE->place(level, random, BlockPos(pos.x + x + 8, y, pos.z + z + 8)))
                break;
        }
    }
}

void ForestBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if (mType == eType_Roofed)
        addMushrooms(level, random, pos);

    int count = random.nextInt(5) - 3;
    if (mType == eType_Flower)
        count += 2;
    addDoublePlants(level, random, pos, count);
    Biome::decorate(level, random, pos);
}

unsigned int ForestBiome::getGrassColor(const BlockPos& pos)
{
    unsigned int grass = Biome::getGrassColor(pos);
    if (mType == eType_Roofed)
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
