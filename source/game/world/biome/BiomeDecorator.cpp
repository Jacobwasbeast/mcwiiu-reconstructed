#include "game/world/biome/BiomeDecorator.hpp"

#include "game/world/biome/Biome.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/util/Random.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/ClayFeature.hpp"
#include "game/world/generation/DeadBushFeature.hpp"
#include "game/world/generation/FlowerFeature.hpp"
#include "game/world/generation/HugeMushroomFeature.hpp"
#include "game/world/generation/IcebergFeature.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/world/generation/SandFeature.hpp"
#include "game/world/generation/SeagrassFeature.hpp"
#include "game/world/generation/SpringFeature.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"

class CustomizableSourceSettings
{
public:
    class Builder
    {
    public:
        static Builder* fromString(SuperflatConfig* config);
        CustomizableSourceSettings* build();
    };
    int values[73];
};

static void placeFeature(Feature* feature, Level* level, Random* random, const BlockPos& position)
{
    if (feature != NULL)
        feature->place(level, *random, position);
}

BiomeDecorator::BiomeDecorator()
    : generateLakes(false), position(), settings(NULL), dirtGen(NULL), gravelGen(NULL), graniteGen(NULL),
      dioriteGen(NULL), andesiteGen(NULL), coalGen(NULL), ironGen(NULL), goldGen(NULL), redstoneGen(NULL),
      diamondGen(NULL), lapisGen(NULL), flowerGen(NULL),
      sandPerChunk(3), clayPerChunk(1), gravelPerChunk(1),
      decorationParameter1(0), decorationParameter2(0), decorationParameter3(0), decorationParameter4(0),
      decorationParameter5(0), decorationParameter6(0), decorationParameter7(0), decorationParameter8(0),
      decorationParameter9(0), decorationParameter10(0), decorationParameter11(0), decorationParameter12(0),
      waterlilyPerChunk(0), treesPerChunk(0), extraTreeChance(0.1f), flowersPerChunk(2), grassPerChunk(1), deadBushPerChunk(0),
      mushroomsPerChunk(0), reedsPerChunk(0), cactiPerChunk(0), decorationParameter13(1), sandPerChunk2(3),
      clayPerChunk2(1), bigMushroomsPerChunk(0), icebergsPerChunk(0), decorationParameter14(0),
      decorationParameter15(0), generateLava(true)
{
    dirtGen = new ClayFeature(4);
    gravelGen = new SandFeature(Blocks::sand, 7);
    graniteGen = new SandFeature(Blocks::sand, 6);
    flowerGen = new FlowerFeature(NULL, NULL);
}

BiomeDecorator::~BiomeDecorator()
{
    delete settings;
    delete dirtGen;
    delete gravelGen;
    delete graniteGen;
    delete dioriteGen;
    delete andesiteGen;
    delete coalGen;
    delete ironGen;
    delete goldGen;
    delete redstoneGen;
    delete diamondGen;
    delete lapisGen;
    delete flowerGen;
}

void BiomeDecorator::decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin)
{
    delete settings;
    settings = CustomizableSourceSettings::Builder::fromString(level->getLevelData()->getGeneratorOptions())->build();
    position = origin;
    delete dioriteGen;
    delete andesiteGen;
    delete coalGen;
    delete ironGen;
    delete goldGen;
    delete redstoneGen;
    delete diamondGen;
    delete lapisGen;
    dioriteGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[30]);
    andesiteGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[34]);
    coalGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[42]);
    ironGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[38]);
    goldGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[46]);
    redstoneGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[50]);
    diamondGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[54]);
    lapisGen = new OreFeature(Blocks::stone->defaultBlockState(), settings->values[58]);
    decorate(biome, level, random);
}

void BiomeDecorator::decorateDepthSpan(Level* level, Random* random, int count, Feature* feature, int minY, int maxY)
{
    if (maxY <= minY)
    {
        if (maxY < minY)
        {
            int value = minY;
            minY = maxY;
            maxY = value;
        }
        else if (minY < 255)
        {
            maxY = minY;
            ++minY;
        }
        else
            --minY;
    }
    for (int i = 0; i < count; ++i)
    {
        BlockPos origin = position.offset(random->nextInt(16), minY + random->nextInt(maxY - minY), random->nextInt(16));
        placeFeature(feature, level, random, origin);
    }
}

void BiomeDecorator::decorateDepthAverage(Level* level, Random* random, int count, Feature* feature, int centreY, int radius)
{
    for (int i = 0; i < count; ++i)
    {
        BlockPos origin = position.offset(random->nextInt(16), random->nextInt(radius) + random->nextInt(radius) + centreY - radius, random->nextInt(16));
        placeFeature(feature, level, random, origin);
    }
}

void BiomeDecorator::decorateOres(Level* level, Random* random)
{
    Level::setInstaTick(true);
    decorateDepthSpan(level, random, settings->values[30], dirtGen, settings->values[31], settings->values[32]);
    decorateDepthSpan(level, random, settings->values[34], gravelGen, settings->values[35], settings->values[36]);
    decorateDepthSpan(level, random, settings->values[42], dioriteGen, settings->values[43], settings->values[44]);
    decorateDepthSpan(level, random, settings->values[38], graniteGen, settings->values[39], settings->values[40]);
    decorateDepthSpan(level, random, settings->values[46], andesiteGen, settings->values[47], settings->values[48]);
    decorateDepthSpan(level, random, settings->values[50], coalGen, settings->values[51], settings->values[52]);
    decorateDepthSpan(level, random, settings->values[54], ironGen, settings->values[55], settings->values[56]);
    decorateDepthSpan(level, random, settings->values[58], goldGen, settings->values[59], settings->values[60]);
    decorateDepthSpan(level, random, settings->values[62], redstoneGen, settings->values[63], settings->values[64]);
    decorateDepthSpan(level, random, settings->values[66], diamondGen, settings->values[67], settings->values[68]);
    decorateDepthAverage(level, random, settings->values[70], lapisGen, settings->values[71], settings->values[72]);
    Level::setInstaTick(false);
}

void BiomeDecorator::decorate(Biome* biome, Level* level, Random* random)
{
    decorateOres(level, random);
    for (int i = 0; i < sandPerChunk; ++i)
        placeFeature(gravelGen, level, random, level->getTopSolidBlockPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8)));
    for (int i = 0; i < clayPerChunk; ++i)
        placeFeature(dirtGen, level, random, level->getTopSolidBlockPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8)));
    int treeCount = treesPerChunk + (random->nextFloat() < extraTreeChance ? 1 : 0);
    for (int i = 0; i < treeCount; ++i)
    {
        AbstractTreeFeature* tree = biome->getTreeFeature(*random);
        tree->init();
        BlockPos origin = level->getHeightmapPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        if (tree->place(level, *random, origin))
            tree->placeFallenTrunk(level, origin, *random, 3);
        delete tree;
    }
    for (int i = 0; i < mushroomsPerChunk; ++i)
        placeFeature(NULL, level, random, level->getHeightmapPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8)));
    for (int i = 0; i < flowersPerChunk; ++i)
    {
        BlockPos origin = level->getHeightmapPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        if (origin.getY() + 32 > 0)
        {
            FlowerBlock::Flower* flower = biome->getRandomFlower(*random, origin);
            flowerGen->setNextFlower(flower->getCategory()->getBlock(), flower);
            placeFeature(flowerGen, level, random, position.offset(random->nextInt(16) + 8, random->nextInt(), random->nextInt(16) + 8));
        }
    }
    for (int i = 0; i < grassPerChunk; ++i)
    {
        BlockPos origin = level->getHeightmapPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        if (origin.getY() * 2 > 0)
            placeFeature(biome->getGrassFeature(*random), level, random, position.offset(random->nextInt(16) + 8, random->nextInt(), random->nextInt(16) + 8));
    }
    for (int i = 0; i < reedsPerChunk; ++i)
        placeFeature(NULL, level, random, position);
    for (int i = 0; i < cactiPerChunk; ++i)
        placeFeature(dirtGen, level, random, level->getHeightmapPos(position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8)));
    if (generateLava)
    {
        SpringFeature spring(Blocks::WATER);
        for (int i = 0; i < 50; ++i)
            placeFeature(&spring, level, random, position.offset(random->nextInt(16) + 8, random->nextInt(120) + 8, random->nextInt(16) + 8));
        SpringFeature lowerSpring(Blocks::FLOWING_WATER);
        for (int i = 0; i < 20; ++i)
            placeFeature(&lowerSpring, level, random, position.offset(random->nextInt(16) + 8, random->nextInt(112) + 8, random->nextInt(16) + 8));
    }
}
