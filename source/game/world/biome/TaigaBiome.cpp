#include "game/world/biome/TaigaBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/block/BlockBlobFeature.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/MegaPineTreeFeature.hpp"
#include "game/world/generation/PineFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/world/generation/TallgrassFeature.hpp"

extern DoublePlantFeature* gDoublePlantFeature;
extern DoublePlantBlock::Variant* gDoublePlantGrass;
extern TallGrassBlock::Variant* gTallGrass;
extern TallGrassBlock::Variant* gFern;

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

TaigaBiome::TaigaBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Taiga, properties),
      mType(type)
{
    mMonsters.push_back(new MobSpawnerData(8, 0x0D6C0002ULL, 4, 4));
    mCreatures.push_back(new MobSpawnerData(4, 0x2D680005ULL, 2, 3));
    mDecorator->treesPerChunk = 10;

    if (type == eType_MegaTaiga || type == eType_Mutated)
    {
        mDecorator->flowersPerChunk = 7;
        mDecorator->grassPerChunk = 1;
        mDecorator->deadBushPerChunk = 3;
    }
    else
    {
        mDecorator->flowersPerChunk = 1;
        mDecorator->deadBushPerChunk = 1;
    }
}

TaigaBiome::~TaigaBiome()
{
}

AbstractTreeFeature* TaigaBiome::getTreeFeature(Random& random)
{
    if ((mType == eType_MegaTaiga || mType == eType_Mutated) && random.nextInt(3) == 0)
    {
        if (mType != eType_Mutated && random.nextInt(13) != 0)
        {
            return new MegaPineTreeFeature(false, false);
        }
        return new MegaPineTreeFeature(false, true);
    }

    if (random.nextInt(3) == 0)
    {
        return new PineFeature();
    }
    return new SpruceFeature(false);
}

Feature* TaigaBiome::getGrassFeature(Random& random)
{
    if (random.nextInt(5) > 0)
    {
        return new TallgrassFeature(gTallGrass);
    }
    return new TallgrassFeature(gFern);
}

void TaigaBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if ((mType == eType_MegaTaiga || mType == eType_Mutated))
    {
        const int blobCount = random.nextInt(3);
        for (int i = 0; i < blobCount; ++i)
        {
            BlockPos blobPos = level->getHeightmapPos(pos.offset(random.nextInt(16) + 8, 0,
                                                                  random.nextInt(16) + 8));
            BlockBlobFeature blob(Blocks::stone, 0);
            blob.place(level, random, blobPos);
        }
    }

    gDoublePlantFeature->setNextPlantType(gDoublePlantGrass);
    for (int i = 0; i < 8; ++i)
    {
        const int x = random.nextInt(16);
        const int z = random.nextInt(16);
        BlockPos plantPos = level->getHeightmapPos(pos.offset(x + 8, 0, z + 8));
        const int y = random.nextInt(plantPos.y + 32);
        gDoublePlantFeature->place(level, random, pos.offset(x + 8, y, z + 8));
    }

    Biome::decorate(level, random, pos);
}

void TaigaBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer,
                                int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "TaigaBiome::buildSurfaceAtDefault");

    if (mType == eType_MegaTaiga || mType == eType_Mutated)
    {
        mTopBlock.blockId = 2;
        mTopBlock.blockData = 0;
        mFillerBlock.blockId = 3;
        mFillerBlock.blockData = 0;

        if (noise > 1.75)
        {
            mTopBlock.blockId = 3;
            mTopBlock.blockData = 1;
        }
        else if (noise < -0.95)
        {
            mTopBlock.blockId = 3;
            mTopBlock.blockData = 2;
        }
    }

    Biome::buildSurfaceAt(level, random, primer, x, z, noise);
    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass TaigaBiome::getBaseClass()
{
    return eBiomeBaseClass_Taiga;
}
