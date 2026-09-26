#include "game/world/biome/OceanWarmBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/generation/CoralCrustFeature.hpp"
#include "game/world/generation/CoralFeature.hpp"
#include "game/world/generation/CoralHangFeature.hpp"
#include "game/world/generation/SeaAnemoneFeature.hpp"
#include "game/world/generation/SeaPickleFeature.hpp"
#include "game/entity/mob/Entity.hpp"

namespace
{
}

OceanWarmBiome::OceanWarmBiome(EBiomeIDs biomeId, bool deep, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ocean, properties),
      mDeep(deep)
{
    clearDefaultFriendlies();

    if (!mDeep)
    {
        mWaterCreatures.push_back(new MobSpawnerData(25, EntityType::PUFFERFISH, 3, 5));
        mWaterCreatures.push_back(new MobSpawnerData(75, EntityType::TROPICAL_FISH, 6, 10));
        mWaterCreatures.push_back(new MobSpawnerData(25, EntityType::TROPICAL_FISH, 3, 5));
    }

    mSquid.push_back(new MobSpawnerData(10, EntityType::SQUID, 2, 4));
    mWaterCreatures.push_back(new MobSpawnerData(7, EntityType::DOLPHIN, 3, 5));
    mMonsters.push_back(new MobSpawnerData(10, EntityType::DROWNED, 2, 4));

    mOceanRuinConfig.biomeType = OceanRuin::eRuinType_Warm;
    mOceanRuinConfig.smallProbability = 0.3F;
    mOceanRuinConfig.clusterProbability = 0.9F;
    mOceanRuinConfig.enabled = true;

    mDecorator->seaGrassPerChunk = 4;
    mDecorator->seagrassPerChunk = 12;
}

OceanWarmBiome::~OceanWarmBiome()
{
}

float OceanWarmBiome::getCreatureProbability()
{
    return 0.1F;
}

void OceanWarmBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    for (int i = 0; i < 4; ++i)
    {
        const int x = random.nextInt(16);
        const int z = random.nextInt(16);
        mDecorator->coralGen->place(level, random, pos.offset(x, 45, z));
    }

    if (random.nextInt(3) == 0)
    {
        const int x = random.nextInt(10);
        const int z = random.nextInt(10);
        mDecorator->coralCrustGen->place(level, random, pos.offset(x, 45, z));
    }

    for (int i = 0; i < 16; ++i)
    {
        const int x = random.nextInt(16);
        const int z = random.nextInt(16);
        mDecorator->coralHangGen->place(level, random, pos.offset(x, 62, z));
    }

    for (int i = 0; i < 20; ++i)
    {
        const int x = random.nextInt(16);
        const int z = random.nextInt(16);
        mDecorator->seaAnemoneGen->place(level, random, pos.offset(x, 62, z));
    }

    if (random.nextInt(6) == 0)
    {
        for (int i = 0; i < mDecorator->seaGrassPerChunk; ++i)
        {
            const int x = random.nextInt(24);
            const int z = random.nextInt(24);
            MutableBlockPos position(pos.offset(x, 0, z));
            int depth = level->getHeightmap(position.x, position.z) * 2;
            position.setY(depth != 0 ? random.nextInt(depth) : 0);
            mDecorator->seaPickleGen->place(level, random, position);
        }
    }
}

Biome::EBiomeBaseClass OceanWarmBiome::getBaseClass()
{
    return eBiomeBaseClass_OceanWarm;
}

Biome::TemperatureCategory OceanWarmBiome::getTemperatureCategory() const
{
    return eTemperatureCategory_Ocean;
}
