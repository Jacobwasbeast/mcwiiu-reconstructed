#include "game/world/biome/BeachBiome.hpp"

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

static const int SAND_BLOCK_ID = 12;

static const float TURTLE_MIN_TEMPERATURE = 0.5F;

BeachBiome::BeachBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Beach, properties)
{
    mCreatures.clear();
    mChickens.clear();

    mTopBlock.blockId = SAND_BLOCK_ID;
    mTopBlock.blockData = Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState());
    mFillerBlock.blockId = SAND_BLOCK_ID;
    mFillerBlock.blockData = Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState());

    if (getTemperature() > TURTLE_MIN_TEMPERATURE)
    {
        mCreatures.push_back(new MobSpawnerData(8, 0x2D680007ULL, 2, 6));
    }

    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 0;
    mDecorator->reedsPerChunk = 0;
    mDecorator->cactiPerChunk = 0;
}

BeachBiome::~BeachBiome()
{
}

Biome::EBiomeBaseClass BeachBiome::getBaseClass()
{
    return eBiomeBaseClass_Beach;
}
