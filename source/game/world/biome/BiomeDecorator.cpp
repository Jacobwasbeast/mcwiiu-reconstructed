#include "game/world/biome/BiomeDecorator.hpp"

#include "game/core/Bush.hpp"
#include "game/core/CustomizableSourceSettings.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/PIXNamedEvents.hpp"
#include "game/core/SuperflatConfig.hpp"
#include "game/world/block/Material.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelData.hpp"
#include "game/world/biome/Biome.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/FlowerBlock.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/generation/BlueIceFeature.hpp"
#include "game/world/generation/BushFeature.hpp"
#include "game/world/generation/CactusFeature.hpp"
#include "game/world/generation/ClayFeature.hpp"
#include "game/world/generation/CoralCrustFeature.hpp"
#include "game/world/generation/CoralFeature.hpp"
#include "game/world/generation/CoralHangFeature.hpp"
#include "game/world/generation/DeadBushFeature.hpp"
#include "game/world/generation/FlowerFeature.hpp"
#include "game/world/generation/HugeMushroomFeature.hpp"
#include "game/world/generation/IcebergFeature.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/world/generation/PumpkinFeature.hpp"
#include "game/world/generation/ReedsFeature.hpp"
#include "game/world/generation/SandFeature.hpp"
#include "game/world/generation/SeaAnemoneFeature.hpp"
#include "game/world/generation/SeagrassFeature.hpp"
#include "game/world/generation/SeaPickleFeature.hpp"
#include "game/world/generation/SpringFeature.hpp"
#include "game/world/generation/WaterlilyFeature.hpp"

static void placeFeature(Feature* feature, Level* level, Random* random, const BlockPos& pos)
{
    feature->place(level, *random, pos);
}

BiomeDecorator::BiomeDecorator()
{
    clayGen = new ClayFeature(4);
    sandGen = new SandFeature(Blocks::sand, 7);
    gravelAsSandGen = new SandFeature(Blocks::gravel, 6);
    dirtGen = NULL;
    gravelGen = NULL;
    graniteGen = NULL;
    dioriteGen = NULL;
    andesiteGen = NULL;
    coalGen = NULL;
    ironGen = NULL;
    goldGen = NULL;
    redstoneGen = NULL;
    diamondGen = NULL;
    lapisGen = NULL;
    flowerGen = new FlowerFeature(static_cast<FlowerBlock*>(Blocks::yellow_flower),
                                   FlowerBlock::Flower::DANDELION);
    brownMushroomGen = new BushFeature(static_cast<Bush*>(Blocks::brown_mushroom));
    redMushroomGen = new BushFeature(static_cast<Bush*>(Blocks::red_mushroom));
    bigMushroomGen = new HugeMushroomFeature();
    reedGen = new ReedsFeature();
    cactusGen = new CactusFeature();
    waterlilyGen = new WaterlilyFeature();
    kelpGen = new KelpFeature();
    seagrassGen = new SeagrassFeature();
    coralGen = new CoralFeature();
    seaPickleGen = new SeaPickleFeature();
    icebergGen = new IcebergFeature();
    blueIceGen = new BlueIceFeature();
    coralCrustGen = new CoralCrustFeature();
    coralHangGen = new CoralHangFeature();
    settings = NULL;
    bigMushroomsPerChunk = 0;
    sandPerChunk2 = 3;
    decorating = false;
    extraTreeChance = 0.1f;
    mushroomsPerChunk = 0;
    treesPerChunk = 0;
    cactiPerChunk = 0;
    deadBushPerChunk = 0;
    kelpPerChunk = 0;
    sandPerChunk = 1;
    seagrassPerChunk = 0;
    grassPerChunk = 1;
    clayPerChunk = 1;
    seaAnemoneGen = new SeaAnemoneFeature();
    reedsPerChunk = 0;
    flowersPerChunk = 2;
    waterlilyPerChunk = 0;
    seaGrassPerChunk = 0;
    generateLakes = true;
}

BiomeDecorator::~BiomeDecorator()
{
    delete clayGen;
    delete sandGen;
    delete gravelAsSandGen;
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
    delete brownMushroomGen;
    delete redMushroomGen;
    delete bigMushroomGen;
    delete reedGen;
    delete cactusGen;
    delete waterlilyGen;
    delete settings;
    delete kelpGen;
    delete seagrassGen;
    delete coralGen;
    delete coralCrustGen;
    delete coralHangGen;
    delete seaAnemoneGen;
    delete seaPickleGen;
    delete icebergGen;
    delete blueIceGen;
}

void BiomeDecorator::decorate(Level* level, Random* random, Biome* biome, const BlockPos& origin)
{
    delete settings;
    CustomizableSourceSettings::Builder* builder =
        CustomizableSourceSettings::Builder::fromString(level->getLevelData()->getGeneratorOptions());
    settings = builder->build();
    position = origin;

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

    dirtGen = new OreFeature(Blocks::dirt->defaultBlockState(), settings->dirtCount);
    gravelGen = new OreFeature(Blocks::gravel->defaultBlockState(), settings->gravelCount);
    graniteGen = new OreFeature(Blocks::stone->getBlockState(1), settings->dioriteCount);
    dioriteGen = new OreFeature(Blocks::stone->getBlockState(3), settings->graniteCount);
    andesiteGen = new OreFeature(Blocks::stone->getBlockState(5), settings->andesiteCount);
    coalGen = new OreFeature(Blocks::coal_ore->defaultBlockState(), settings->coalCount);
    ironGen = new OreFeature(Blocks::iron_ore->defaultBlockState(), settings->ironCount);
    goldGen = new OreFeature(Blocks::gold_ore->defaultBlockState(), settings->goldCount);
    redstoneGen = new OreFeature(Blocks::redstone_ore->defaultBlockState(), settings->redstoneCount);
    diamondGen = new OreFeature(Blocks::diamond_ore->defaultBlockState(), settings->diamondCount);
    lapisGen = new OreFeature(Blocks::lapis_ore->defaultBlockState(), settings->lapisCount);

    decorate(biome, level, random);
    decorating = false;
    delete builder;
}

void BiomeDecorator::decorateDepthSpan(Level* level, Random* random, int count, Feature* feature,
                                       int minY, int maxY)
{
    if (maxY <= minY)
    {
        if (maxY < minY)
        {
            int swap = minY;
            minY = maxY;
            maxY = swap;
        }
        else if (minY < 255)
        {
            maxY = minY + 1;
        }
        else
        {
            --minY;
        }
    }

    for (int i = 0; i < count; ++i)
    {
        BlockPos pos(position.x + random->nextInt(16), minY + random->nextInt(maxY - minY),
                     position.z + random->nextInt(16));
        placeFeature(feature, level, random, pos);
    }
}

void BiomeDecorator::decorateDepthAverage(Level* level, Random* random, int count, Feature* feature,
                                          int centreY, int radius)
{
    for (int i = 0; i < count; ++i)
    {
        BlockPos pos(position.x + random->nextInt(16),
                     random->nextInt(radius) + random->nextInt(radius) + centreY - radius,
                     position.z + random->nextInt(16));
        placeFeature(feature, level, random, pos);
    }
}

void BiomeDecorator::decorateOres(Level* level, Random* random)
{
    Level::setInstaTick(true);
    decorateDepthSpan(level, random, settings->dirtSize, dirtGen, settings->dirtMinHeight, settings->dirtMaxHeight);
    decorateDepthSpan(level, random, settings->gravelSize, gravelGen, settings->gravelMinHeight, settings->gravelMaxHeight);
    decorateDepthSpan(level, random, settings->graniteSize, dioriteGen, settings->graniteMinHeight, settings->graniteMaxHeight);
    decorateDepthSpan(level, random, settings->dioriteSize, graniteGen, settings->dioriteMinHeight, settings->dioriteMaxHeight);
    decorateDepthSpan(level, random, settings->andesiteSize, andesiteGen, settings->andesiteMinHeight, settings->andesiteMaxHeight);
    decorateDepthSpan(level, random, settings->coalSize, coalGen, settings->coalMinHeight, settings->coalMaxHeight);
    decorateDepthSpan(level, random, settings->ironSize, ironGen, settings->ironMinHeight, settings->ironMaxHeight);
    decorateDepthSpan(level, random, settings->goldSize, goldGen, settings->goldMinHeight, settings->goldMaxHeight);
    decorateDepthSpan(level, random, settings->redstoneSize, redstoneGen, settings->redstoneMinHeight, settings->redstoneMaxHeight);
    decorateDepthSpan(level, random, settings->diamondSize, diamondGen, settings->diamondMinHeight, settings->diamondMaxHeight);
    decorateDepthAverage(level, random, settings->lapisSize, lapisGen, settings->lapisMinHeight, settings->lapisMaxHeight);
    Level::setInstaTick(false);
}

void BiomeDecorator::decorate(Biome* biome, Level* level, Random* random)
{
    PIXBeginNamedEvent(0.0f, "Decorate ores");
    decorateOres(level, random);
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate sand clay gravel");
    for (int i = 0; i < sandPerChunk2; ++i)
    {
        BlockPos pos = level->getTopSolidBlockPos(
            position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        placeFeature(sandGen, level, random, pos);
    }
    for (int i = 0; i < clayPerChunk; ++i)
    {
        BlockPos pos = level->getTopSolidBlockPos(
            position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        placeFeature(clayGen, level, random, pos);
    }
    for (int i = 0; i < sandPerChunk; ++i)
    {
        BlockPos pos = level->getTopSolidBlockPos(
            position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        placeFeature(gravelAsSandGen, level, random, pos);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate forests");
    int treeCount = treesPerChunk;
    if (random->nextFloat() < extraTreeChance)
        ++treeCount;
    for (int i = 0; i < treeCount; ++i)
    {
        AbstractTreeFeature* tree = biome->getTreeFeature(*random);
        tree->init();
        BlockPos pos = level->getHeightmapPos(
            position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        if (tree->place(level, *random, pos))
            tree->postPlaceTree(level, *random, pos);
        delete tree;
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate mushrooms flowers grass");
    for (int i = 0; i < bigMushroomsPerChunk; ++i)
    {
        BlockPos pos = level->getHeightmapPos(
            position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
        placeFeature(bigMushroomGen, level, random, pos);
    }
    for (int i = 0; i < flowersPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y + 32 > 0)
        {
            BlockPos pos = position.offset(x + 8, random->nextInt(top.y + 32), z + 8);
            FlowerBlock::Flower* flower = biome->getRandomFlower(*random, pos);
            Block* block = flower->getCategory()->getBlock();
            if (block->defaultBlockState()->getMaterial() != Material::AIR)
            {
                flowerGen->setNextFlower(static_cast<FlowerBlock*>(block), flower);
                placeFeature(flowerGen, level, random, pos);
            }
        }
    }
    for (int i = 0; i < grassPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            BlockPos pos = position.offset(x + 8, random->nextInt(top.y * 2), z + 8);
            MemSect(50);
            Feature* grass = biome->getGrassFeature(*random);
            MemSect(0);
            placeFeature(grass, level, random, pos);
            delete grass;
        }
    }
    for (int i = 0; i < seagrassPerChunk; ++i)
    {
        BlockPos pos = position.offset(random->nextInt(16), 54, random->nextInt(16));
        placeFeature(seagrassGen, level, random, pos);
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate bush waterlily mushroom");
    DeadBushFeature* deadBush = NULL;
    if (deadBushPerChunk > 0)
        deadBush = new DeadBushFeature(Blocks::deadbush);
    for (int i = 0; i < deadBushPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
            placeFeature(deadBush, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
    }
    delete deadBush;

    for (int i = 0; i < waterlilyPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            BlockPos pos = position.offset(x + 8, random->nextInt(top.y * 2), z + 8);
            while (pos.y > 0 && level->isEmptyBlock(pos.below()))
                pos = pos.below();
            placeFeature(waterlilyGen, level, random, pos);
        }
    }

    for (int i = 0; i < mushroomsPerChunk; ++i)
    {
        if (random->nextInt(4) == 0)
        {
            BlockPos pos = level->getHeightmapPos(
                position.offset(random->nextInt(16) + 8, 0, random->nextInt(16) + 8));
            placeFeature(brownMushroomGen, level, random, pos);
        }
        if (random->nextInt(8) == 0)
        {
            int x = random->nextInt(16);
            int z = random->nextInt(16);
            BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
            if (top.y * 2 > 0)
                placeFeature(redMushroomGen, level, random,
                             position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
        }
    }
    if (random->nextInt(4) == 0)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
            placeFeature(brownMushroomGen, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
    }
    if (random->nextInt(8) == 0)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
            placeFeature(redMushroomGen, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
    }
    if (random->nextInt(18) == 0)
    {
        for (int i = 0; i < kelpPerChunk; ++i)
        {
            BlockPos pos = position.offset(random->nextInt(35), 43, random->nextInt(35));
            placeFeature(kelpGen, level, random, pos);
        }
    }
    for (int i = 0; i < reedsPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
            placeFeature(reedGen, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
    }
    for (int i = 0; i < 10; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
            placeFeature(reedGen, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
    }
    if (random->nextInt(32) == 0)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
        {
            Feature* pumpkin = new PumpkinFeature();
            placeFeature(pumpkin, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
            delete pumpkin;
        }
    }
    for (int i = 0; i < cactiPerChunk; ++i)
    {
        int x = random->nextInt(16);
        int z = random->nextInt(16);
        BlockPos top = level->getHeightmapPos(position.offset(x + 8, 0, z + 8));
        if (top.y * 2 > 0)
            placeFeature(cactusGen, level, random,
                         position.offset(x + 8, random->nextInt(top.y * 2), z + 8));
    }
    PIXEndNamedEvent();

    PIXBeginNamedEvent(0.0f, "Decorate liquids");
    if (generateLakes)
    {
        SpringFeature waterSpring(Blocks::FLOWING_WATER);
        for (int i = 0; i < 50; ++i)
        {
            int y = random->nextInt(120) + 8;
            placeFeature(&waterSpring, level, random,
                         position.offset(random->nextInt(16) + 8, random->nextInt(y),
                                         random->nextInt(16) + 8));
        }

        SpringFeature lavaSpring(Blocks::flowing_lava);
        for (int i = 0; i < 20; ++i)
        {
            int y = random->nextInt(random->nextInt(112) + 8) + 8;
            placeFeature(&lavaSpring, level, random,
                         position.offset(random->nextInt(16) + 8, random->nextInt(y),
                                         random->nextInt(16) + 8));
        }
    }
    PIXEndNamedEvent();
}
