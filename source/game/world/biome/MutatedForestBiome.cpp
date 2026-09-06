#include "game/world/biome/MutatedForestBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/generation/BirchFeature.hpp"

MutatedForestBiome::MutatedForestBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : ForestBiome(biomeId, eType_Roofed, properties)
{
}

MutatedForestBiome::~MutatedForestBiome()
{
}

AbstractTreeFeature* MutatedForestBiome::getTreeFeature(Random& random)
{
    return new BirchFeature(false, random.nextBoolean());
}
