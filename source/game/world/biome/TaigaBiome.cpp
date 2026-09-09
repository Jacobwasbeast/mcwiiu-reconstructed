#include "game/world/biome/TaigaBiome.hpp"

#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockBlobFeature.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirtBlock.hpp"
#include "game/world/block/DoublePlantBlock.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/MegaPineTreeFeature.hpp"
#include "game/world/generation/PineFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"
#include "game/world/generation/TallgrassFeature.hpp"

extern DoublePlantFeature* gDoublePlantFeature;

TaigaBiome::TaigaBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Taiga, properties),
      mType(type)
{
    mWolves.push_back(new MobSpawnerData(8, 0x0D6C0002ULL, 4, 4));
    mCreatures.push_back(new MobSpawnerData(4, 0x2D680005ULL, 2, 3));

    mDecorator->treesPerChunk = 10;

    if (type == eType_MegaTaiga || type == eType_Mutated)
    {
        mDecorator->grassPerChunk = 7;
        mDecorator->deadBushPerChunk = 1;
        mDecorator->mushroomsPerChunk = 3;
    }
    else
    {
        mDecorator->grassPerChunk = 1;
        mDecorator->mushroomsPerChunk = 1;
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
        return new TallgrassFeature(gFern);
    }

    return new TallgrassFeature(gTallGrass);
}

void TaigaBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if (mType == eType_MegaTaiga || mType == eType_Mutated)
    {
        int blobCount = random.nextInt(3);
        for (int i = 0; i < blobCount; ++i)
        {
            int x = random.nextInt(16) + 8;
            int z = random.nextInt(16) + 8;
            BlockPos blobPos = level->getHeightmapPos(pos.offset(x, 0, z));
            BlockBlobFeature blob(Blocks::mossy_cobblestone, 0);
            blob.place(level, random, blobPos);
        }
    }

    gDoublePlantFeature->setNextPlantType(DoublePlantBlock::VARIANTS[3]);

    for (int i = 0; i < 7; ++i)
    {
        int x = random.nextInt(16) + 8;
        int z = random.nextInt(16) + 8;
        int y = random.nextInt(level->getHeightmapPos(pos.offset(x, 0, z)).y + 32);
        gDoublePlantFeature->place(level, random, pos.offset(x, y, z));
    }

    Biome::decorate(level, random, pos);
}

void TaigaBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer,
                                int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "TaigaBiome::buildSurfaceAtDefault");

    if (mType == eType_MegaTaiga || mType == eType_Mutated)
    {
        mTopBlock = BlockPair(2, 0);
        mFillerBlock = BlockPair(3, 0);

        if (noise > 1.75)
        {
            TypedBoxed<DirtBlock::Variant*> variant(gVariants[1]);
            const BlockState* coarseDirt =
                Blocks::dirt->defaultBlockState()->setBoxedValue(gVariantProperty, &variant);
            mTopBlock.blockId = 3;
            mTopBlock.blockData = Blocks::dirt->convertBlockStateToLegacyData(coarseDirt);
        }
        else if (noise > -0.95)
        {
            TypedBoxed<DirtBlock::Variant*> variant(gVariants[2]);
            const BlockState* podzol =
                Blocks::dirt->defaultBlockState()->setBoxedValue(gVariantProperty, &variant);
            mTopBlock.blockData = Blocks::dirt->convertBlockStateToLegacyData(podzol);
            mTopBlock.blockId = 3;
        }
    }

    buildSurfaceAtDefault(level, random, primer, x, z, noise);
    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass TaigaBiome::getBaseClass()
{
    return eBiomeBaseClass_Taiga;
}
