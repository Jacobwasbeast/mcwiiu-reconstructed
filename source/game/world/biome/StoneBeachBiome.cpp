#include "game/world/biome/StoneBeachBiome.hpp"

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

static const int STONE_BLOCK_ID = 1;

StoneBeachBiome::StoneBeachBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_StoneBeach, properties)
{
    mCreatures.clear();

    mTopBlock.blockId = STONE_BLOCK_ID;
    mTopBlock.blockData = Blocks::stone->convertBlockStateToLegacyData(Blocks::stone->defaultBlockState());
    mFillerBlock.blockId = STONE_BLOCK_ID;
    mFillerBlock.blockData = Blocks::stone->convertBlockStateToLegacyData(Blocks::stone->defaultBlockState());

    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 0;
    mDecorator->reedsPerChunk = 0;
    mDecorator->cactiPerChunk = 0;
}

StoneBeachBiome::~StoneBeachBiome()
{
}

Biome::EBiomeBaseClass StoneBeachBiome::getBaseClass()
{
    return eBiomeBaseClass_StoneBeach;
}
