#include "game/world/biome/ExtremeHillsBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/MonsterEggBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/chunk/ChunkPrimer.hpp"
#include "game/world/generation/OreFeature.hpp"
#include "game/world/generation/SpruceFeature.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

ExtremeHillsBiome::ExtremeHillsBiome(EBiomeIDs biomeId, EType type, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_ExtremeHills, properties),
      mType(type)
{
    if (type == eType_ExtraTrees)
    {
        mDecorator->treesPerChunk = 3;
    }

    mCreatures.push_back(new MobSpawnerData(5, 0x2D698003ULL, 4, 6));

    TypedBoxed<MonsterEggBlock::Variant*> variant(MonsterEggBlock::Variant::byData(0));
    mMonsterEggFeature = new OreFeature(
        Blocks::monster_egg->defaultBlockState()->setBoxedValue(MonsterEggBlock::VARIANT, &variant),
        9);
}

ExtremeHillsBiome::~ExtremeHillsBiome()
{
    delete mMonsterEggFeature;
}

AbstractTreeFeature* ExtremeHillsBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(3) > 0)
    {
        return new SpruceFeature(false);
    }

    return Biome::getTreeFeature(random);
}

void ExtremeHillsBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    int oreCount = random.nextInt(6) + 3;
    for (int i = 0; i < oreCount; ++i)
    {
        BlockPos orePos = pos.offset(random.nextInt(16), random.nextInt(28) + 4, random.nextInt(16));
        if (level->getBlockState(orePos)->getBlock() == Blocks::stone)
        {
            level->setBlock(orePos, Blocks::emerald_ore->defaultBlockState(), 2, false);
        }
    }

    for (int i = 0; i < 7; ++i)
    {
        mMonsterEggFeature->place(
            level, random, pos.offset(random.nextInt(16), random.nextInt(64), random.nextInt(16)));
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
            mTopBlock.blockData = 0;
            mFillerBlock.blockId = 1;
            mFillerBlock.blockData = 0;
        }

        buildSurfaceAtDefault(level, random, primer, x, z, noise);
    }
    else
    {
        mTopBlock.blockId = 13;
        mTopBlock.blockData = 0;
        mFillerBlock.blockId = 13;
        mFillerBlock.blockData = 0;

        buildSurfaceAtDefault(level, random, primer, x, z, noise);
    }

    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass ExtremeHillsBiome::getBaseClass()
{
    return eBiomeBaseClass_ExtremeHills;
}
