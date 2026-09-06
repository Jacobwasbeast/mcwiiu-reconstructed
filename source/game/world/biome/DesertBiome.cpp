#include "game/world/biome/DesertBiome.hpp"

#include <vector>

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/DesertWellFeature.hpp"
#include "game/world/generation/FossilFeature.hpp"

DesertBiome::DesertBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Desert, properties)
{
    mCreatures.push_back(new MobSpawnerData(4, 0x2D680005ULL, 2, 3));

    mTopBlock.blockId = 12;
    mTopBlock.blockData = Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState());
    mFillerBlock.blockId = 12;
    mFillerBlock.blockData = Blocks::sand->convertBlockStateToLegacyData(Blocks::sand->defaultBlockState());

    mDecorator->treesPerChunk = -999;
    mDecorator->deadBushPerChunk = 2;
    mDecorator->reedsPerChunk = 50;
    mDecorator->cactiPerChunk = 10;

    for (std::vector<MobSpawnerData*>::iterator iterator = mMonsters.begin();
         iterator != mMonsters.end(); ++iterator)
    {
        MobSpawnerData* data = *iterator;
        if (data->mEntityBaseClass == 0x5D550040ULL || data->mEntityBaseClass == 0x5D550042ULL)
        {
            iterator = mMonsters.erase(iterator);
            delete data;
        }
    }

    mMonsters.push_back(new MobSpawnerData(19, 0x5D550040ULL, 4, 4));
    mMonsters.push_back(new MobSpawnerData(1, 0x5D550042ULL, 1, 1));
    mMonsters.push_back(new MobSpawnerData(80, 0x5D550043ULL, 4, 4));
}

DesertBiome::~DesertBiome()
{
}

void DesertBiome::decorate(Level* level, Random& random, const BlockPos& origin)
{
    Biome::decorate(level, random, origin);

    if (random.nextInt(1000) == 0)
    {
        const int x = random.nextInt(16) + 8;
        const int z = random.nextInt(16) + 8;
        const BlockPos position = origin.offset(x, 0, z);
        const BlockPos heightmapPosition = level->getHeightmapPos(position);
        const BlockPos wellPosition = heightmapPosition.above();
        DesertWellFeature well;
        well.place(level, random, wellPosition);
    }

    if (random.nextInt(64) == 0)
    {
        FossilFeature fossil;
        fossil.place(level, random, origin);
    }
}

Biome::EBiomeBaseClass DesertBiome::getBaseClass()
{
    return eBiomeBaseClass_Desert;
}
