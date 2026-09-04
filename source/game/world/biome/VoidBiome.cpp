#include "game/world/biome/VoidBiome.hpp"

VoidBiome::VoidBiome(EBiomeIDs biomeId, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Void, properties)
{
    mMonsters.clear();
    mCreatures.clear();
    mChickens.clear();
    mWolves.clear();
    mSlimes.clear();
    mAmbientCreatures.clear();

    delete mDecorator;
    mDecorator = createDecorator();
}

VoidBiome::~VoidBiome()
{
}

Biome::EBiomeBaseClass VoidBiome::getBaseClass()
{
    return eBiomeBaseClass_Void;
}

bool VoidBiome::isAlwaysValidSpawn()
{
    return true;
}
