#include "game/world/biome/SavannaBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/generation/DoublePlantFeature.hpp"
#include "game/world/generation/SavannaTreeFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"

extern DoublePlantFeature* gDoublePlantFeature;
extern DoublePlantBlock::Variant* gDoublePlantGrass;

SavannaBiome::SavannaBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Savanna, properties)
{
    mCreatures.push_back(new MobSpawnerData(1, 0x2D690001ULL, 2, 6));
    mCreatures.push_back(new MobSpawnerData(1, 0x2D698001ULL, 1, 1));
    if (getDepth() > 1.1F)
    {
        mCreatures.push_back(new MobSpawnerData(8, 0x2D698003ULL, 4, 4));
    }

    mDecorator->treesPerChunk = 1;
    mDecorator->flowersPerChunk = 4;
    mDecorator->grassPerChunk = 20;
}

SavannaBiome::~SavannaBiome()
{
}

AbstractTreeFeature* SavannaBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(5) > 0)
    {
        return new SavannaTreeFeature(false);
    }

    return new TreeFeature(false);
}

void SavannaBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    gDoublePlantFeature->setNextPlantType(gDoublePlantGrass);

    for (int i = 0; i < 6; ++i)
    {
        int x = random.nextInt(16);
        int z = random.nextInt(16);
        BlockPos plantPos = pos.offset(x + 8, 0, z + 8);
        plantPos = level->getHeightmapPos(plantPos);
        int y = random.nextInt(plantPos.y + 32);
        gDoublePlantFeature->place(level, random, pos.offset(x + 8, y, z + 8));
    }

    Biome::decorate(level, random, pos);
}

Biome::EBiomeBaseClass SavannaBiome::getBaseClass()
{
    return eBiomeBaseClass_Savanna;
}
