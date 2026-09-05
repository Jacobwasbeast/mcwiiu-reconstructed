#include "game/world/biome/IceBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/AbstractTreeFeature.hpp"
#include "game/world/generation/Feature.hpp"
#include "game/world/generation/IcePatchFeature.hpp"
#include "game/world/generation/IceSpikeFeature.hpp"

class SpruceFeature;

IceBiome::IceBiome(EBiomeIDs biomeId, bool hasIceSpikes, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Ice, properties),
      mHasIceSpikes(hasIceSpikes),
      mIceSpikeFeature(new IceSpikeFeature),
      mIcePatchFeature(new IcePatchFeature(4))
{
    if (hasIceSpikes)
    {
        mTopBlock.blockId = Block::getId(Blocks::snow);
        mTopBlock.blockData = Blocks::snow->convertBlockStateToLegacyData(Blocks::snow->defaultBlockState());
    }
}

IceBiome::~IceBiome()
{
}

float IceBiome::getCreatureProbability()
{
    return 0.07F;
}

void IceBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    if (mHasIceSpikes)
    {
        BlockPos featurePos = pos.offset(random.nextInt(16) + 8, 0, random.nextInt(16) + 8);
        featurePos = level->getHeightmapPos(featurePos);
        mIceSpikeFeature->place(level, random, featurePos);

        featurePos = pos.offset(random.nextInt(16) + 8, 0, random.nextInt(16) + 8);
        featurePos = level->getHeightmapPos(featurePos);
        mIceSpikeFeature->place(level, random, featurePos);

        featurePos = pos.offset(random.nextInt(16) + 8, 0, random.nextInt(16) + 8);
        featurePos = level->getHeightmapPos(featurePos);
        mIceSpikeFeature->place(level, random, featurePos);

        featurePos = pos.offset(random.nextInt(16) + 8, 0, random.nextInt(16) + 8);
        featurePos = level->getHeightmapPos(featurePos);
        mIcePatchFeature->place(level, random, featurePos);

        featurePos = pos.offset(random.nextInt(16) + 8, 0, random.nextInt(16) + 8);
        featurePos = level->getHeightmapPos(featurePos);
        mIcePatchFeature->place(level, random, featurePos);
    }

    Biome::decorate(level, random, pos);
}

AbstractTreeFeature* IceBiome::getTreeFeature(Random& random)
{
    return Biome::getTreeFeature(random);
}

Biome::EBiomeBaseClass IceBiome::getBaseClass()
{
    return eBiomeBaseClass_Ice;
}
