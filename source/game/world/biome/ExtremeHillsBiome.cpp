#include "game/world/biome/ExtremeHillsBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/Feature.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

ExtremeHillsBiome::ExtremeHillsBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_ExtremeHills, properties),
      mEmeraldOreFeature(NULL),
      mType(type)
{
}

ExtremeHillsBiome::~ExtremeHillsBiome()
{
    delete mEmeraldOreFeature;
}

AbstractTreeFeature* ExtremeHillsBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(3) > 0)
    {
        return Biome::getTreeFeature(random);
    }

    return Biome::getTreeFeature(random);
}

void ExtremeHillsBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    int count = random.nextInt(6) + 3;
    for (int i = 0; i < count; ++i)
    {
        BlockPos emeraldPos = pos.offset(random.nextInt(16), random.nextInt(28) + 4, random.nextInt(16));
        if (level->getBlockState(emeraldPos)->getBlock() == Blocks::AIR)
        {
            level->setBlock(emeraldPos, Blocks::emerald_ore->defaultBlockState(), 2, false);
        }

        emeraldPos = pos.offset(random.nextInt(16), random.nextInt(28) + 4, random.nextInt(16));
        if (level->getBlockState(emeraldPos)->getBlock() == Blocks::AIR)
        {
            level->setBlock(emeraldPos, Blocks::emerald_ore->defaultBlockState(), 2, false);
        }
    }

    if ((count & 1) != 0)
    {
        BlockPos emeraldPos = pos.offset(random.nextInt(16), random.nextInt(28) + 4, random.nextInt(16));
        if (level->getBlockState(emeraldPos)->getBlock() == Blocks::AIR)
        {
            level->setBlock(emeraldPos, Blocks::emerald_ore->defaultBlockState(), 2, false);
        }
    }
}

void ExtremeHillsBiome::buildSurfaceAt(Level* level, Random& random, ChunkPrimer* primer, int x, int z, double noise)
{
    PIXBeginNamedEvent(0.0F, "ExtremeHillsBiome::buildSurfaceAtDefault");

    mTopBlock.blockId = 2;
    mTopBlock.blockData = 0;
    mFillerBlock.blockId = 3;
    mFillerBlock.blockData = 0;

    if ((noise >= -1.0 && noise <= 2.0) || mType != eType_Mutated)
    {
        if (noise > 1.0 && mType != eType_ExtraTrees)
        {
            mTopBlock.blockId = 1;
            mFillerBlock.blockId = 1;
        }

        Biome::buildSurfaceAt(level, random, primer, x, z, noise);
    }
    else
    {
        mTopBlock.blockId = 13;
        mTopBlock.blockData = 0;
        mFillerBlock.blockId = 13;
        mFillerBlock.blockData = 0;
        Biome::buildSurfaceAt(level, random, primer, x, z, noise);
    }

    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass ExtremeHillsBiome::getBaseClass()
{
    return eBiomeBaseClass_ExtremeHills;
}
