#include "game/world/biome/MushroomIslandBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

static const int MYCELIUM_BLOCK_ID = 110;

static const int MUSHROOM_FEATURE_DISABLED = -100;

MushroomIslandBiome::MushroomIslandBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_MushroomIsland, properties)
{
    mDecorator->treesPerChunk = MUSHROOM_FEATURE_DISABLED;
    mDecorator->flowersPerChunk = MUSHROOM_FEATURE_DISABLED;
    mDecorator->grassPerChunk = MUSHROOM_FEATURE_DISABLED;
    mDecorator->mushroomsPerChunk = 1;
    mDecorator->bigMushroomsPerChunk = 1;

    mTopBlock.blockId = MYCELIUM_BLOCK_ID;
    mTopBlock.blockData = Blocks::mycelium->convertBlockStateToLegacyData(Blocks::mycelium->defaultBlockState());

    mMonsters.clear();
    mCreatures.clear();
    mWaterCreatures.clear();
    mChickens.clear();
    mWolves.clear();

    mMushroomCows.push_back(new MobSpawnerData(8, EntityType::MUSHROOM_COW, 4, 8));
}

MushroomIslandBiome::~MushroomIslandBiome()
{
}

Biome::EBiomeBaseClass MushroomIslandBiome::getBaseClass()
{
    return eBiomeBaseClass_MushroomIsland;
}
