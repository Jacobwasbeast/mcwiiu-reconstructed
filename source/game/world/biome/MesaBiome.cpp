#include "game/world/biome/MesaBiome.hpp"
#include "game/core/ColourTable.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/PerlinSimplexNoise.hpp"
#include "game/util/Math.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/OreFeature.hpp"
#include <cmath>

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static Biome::BlockPair sClay(172, 0);
static Biome::BlockPair sOrange(159, 1);
static Biome::BlockPair sYellow(159, 4);
static Biome::BlockPair sBrown(159, 3);
static Biome::BlockPair sWhite(159, 0);

MesaBiome::MesaBiome(EBiomeIDs id, bool brycePillars, bool hasForest, BiomeProperties* properties)
    : Biome(id, eBiomeType_Mesa, properties), mClayBands(NULL), mSeed(0),
      mPillarNoise(NULL), mPillarRoofNoise(NULL), mNoise(NULL),
      mHasForest(hasForest), mBrycePillars(brycePillars)
{
    mTopBlock = BlockPair(12, 1);
    mFillerBlock = BlockPair(172, 0);
    delete mDecorator;
    mDecorator = new MesaDecorator();
    mDecorator->cactiPerChunk = 20;
    mDecorator->sandPerChunk2 = 3;
    mDecorator->clayPerChunk = 5;
    mDecorator->mushroomsPerChunk = 0;
    mDecorator->grassPerChunk = hasForest ? 5 : -999;
}

MesaBiome::~MesaBiome()
{
    delete mPillarNoise;
    delete mPillarRoofNoise;
    delete mNoise;
    delete[] mClayBands;
}

void MesaBiome::staticCtor()
{
    sClay = BlockPair(172, Blocks::hardened_clay->convertBlockStateToLegacyData(Blocks::hardened_clay->defaultBlockState()));
    sOrange = BlockPair(159, Blocks::stained_hardened_clay->getBlockState(1)->mElementId);
    sYellow = BlockPair(159, Blocks::stained_hardened_clay->getBlockState(4)->mElementId);
    sBrown = BlockPair(159, Blocks::stained_hardened_clay->getBlockState(12)->mElementId);
    sWhite = BlockPair(159, Blocks::stained_hardened_clay->getBlockState(0)->mElementId);
}

AbstractTreeFeature* MesaBiome::getTreeFeature(Random& random)
{
    return Biome::getTreeFeature(random);
}

unsigned int MesaBiome::getFoliageColor(const BlockPos& pos)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Foliage_Mesa);
}

unsigned int MesaBiome::getGrassColor(const BlockPos& pos)
{
    return Minecraft::GetInstance()->getColourTable()->getColour(eMinecraftColour_Grass_Mesa);
}

Biome::BlockPair MesaBiome::getBand(int x, int y, int z)
{
    double value = mNoise->getValue((double)x * 0.001953125, (double)z * 0.001953125);
    int index = y + (int)Math::round(value * 2.0) + 64;
    index = (index - (((index >> 31) & 63) + index & ~63)) * 8;
    return mClayBands[index / 8];
}

void MesaBiome::generateBands(long long seed)
{
    delete[] mClayBands;
    mClayBands = new BlockPair[64];
    int i;
    for (i = 0; i < 64; ++i) mClayBands[i] = sClay;
    Random random(seed);
    mNoise = new PerlinSimplexNoise(&random, 1);
    i = 0;
    while (i < 64)
    {
        i += random.nextInt(5) + 1;
        if (i < 64) mClayBands[i] = sOrange;
        ++i;
    }
    int count = random.nextInt(4) + 2;
    for (i = 0; i < count; ++i)
    {
        int length = random.nextInt(3) + 1;
        int start = random.nextInt(64);
        int j;
        for (j = 0; j < length && start + j < 64; ++j) mClayBands[start + j] = sYellow;
    }
    count = random.nextInt(3) + 3;
    int band = 0;
    for (i = 0; i < count; ++i)
    {
        band += random.nextInt(16) + 4;
        if (band < 64)
        {
            mClayBands[band] = sWhite;
            if (band > 1 && random.nextBoolean()) mClayBands[band - 1] = sBrown;
            if (band < 63 && random.nextBoolean()) mClayBands[band + 1] = sBrown;
        }
    }
}

void MesaBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(4.5F, "MesaBiome::buildSurfaceAtDefault");
    long long seed = level->getSeed();
    if (mClayBands == NULL || mSeed != seed) generateBands(seed);
    if (mPillarNoise == NULL || mPillarRoofNoise == NULL)
    {
        Random pillarRandom(seed);
        mPillarNoise = new PerlinSimplexNoise(&pillarRandom, 4);
        mPillarRoofNoise = new PerlinSimplexNoise(&pillarRandom, 1);
    }
    mSeed = seed;
    int localX = x & 15;
    int localZ = z & 15;
    double pillarHeight = 64.0;
    if (mBrycePillars)
    {
        double pillar = mPillarNoise->getValue((double)(x & 0xfffffff0) * 0.25,
                                                (double)(z & 0xfffffff0) * 0.25);
        if (std::fabs(noise) - pillar < 0.0) pillar = std::fabs(noise);
        if (pillar > 0.25)
        {
            double roof = mPillarRoofNoise->getValue((double)(x & 0xfffffff0) * 0.001953125,
                                                      (double)(z & 0xfffffff0) * 0.001953125);
            pillarHeight = pillar * pillar * 50.0;
            double cap = std::ceil(std::fabs(roof) * 14.0);
            if (cap + 4.0 < pillarHeight) pillarHeight = cap + 4.0;
            pillarHeight += 2.5;
        }
    }
    int depth = (int)(noise / 3.0 + 3.0 + random.nextDouble() * 0.25);
    int runDepth = -1;
    int y;
    for (y = 127; y >= 0; --y)
    {
        int index = localZ << 11 | localX << 7 | y;
        if (y <= random.nextInt(2) + 1)
        {
            primer->setBlockAndData(index, 7, 0);
            continue;
        }
        const BlockState* state = primer->getState(localX, y, localZ);
        Block* block = state == NULL ? NULL : state->getBlock();
        if (block != Blocks::stone)
        {
            runDepth = -1;
            continue;
        }
        if (mBrycePillars && (double)y < pillarHeight)
        {
            primer->setBlockAndData(index, sClay.blockId, sClay.blockData);
        }
        if (runDepth == -1)
        {
            runDepth = depth;
            if (y >= 62)
                primer->setBlockAndData(index, mTopBlock.blockId, mTopBlock.blockData);
            else
                primer->setBlockAndData(index, mFillerBlock.blockId, mFillerBlock.blockData);
        }
        else if (runDepth > 0)
        {
            --runDepth;
            if (y < 62)
            {
                BlockPair band = getBand(x, y, z);
                primer->setBlockAndData(index, band.blockId, band.blockData);
            }
            else
                primer->setBlockAndData(index, mTopBlock.blockId, mTopBlock.blockData);
        }
    }
    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass MesaBiome::getBaseClass()
{
    return eBiomeBaseClass_Mesa;
}

MesaBiome::MesaDecorator::MesaDecorator() : BiomeDecorator()
{
}

MesaBiome::MesaDecorator::~MesaDecorator()
{
}

void MesaBiome::MesaDecorator::decorateOres(Level* level, Random* random)
{
    BiomeDecorator::decorateOres(level, random);
    decorateDepthSpan(level, random, 20, dirtGen, 32, 80);
}
