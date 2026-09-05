#include "game/world/biome/JungleBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/GroundBushFeature.hpp"
#include "game/world/generation/MegaJungleTreeFeature.hpp"
#include "game/world/generation/MelonFeature.hpp"
#include "game/world/generation/TallgrassFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/world/generation/VinesFeature.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

extern TallGrassBlock::Variant* gTallGrass;
extern TallGrassBlock::Variant* gFern;

static const BlockState* sJungleTrunkState;
static const BlockState* sJungleLeafState;
static const BlockState* sOakLeafState;

JungleBiome::JungleBiome(EBiomeIDs biomeId, bool isEdge, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Jungle, properties),
      mIsEdge(isEdge)
{
    mDecorator->treesPerChunk = isEdge ? 2 : 50;
    mDecorator->flowersPerChunk = 4;
    mDecorator->grassPerChunk = 25;

    if (!isEdge)
    {
        mMonsters.push_back(new MobSpawnerData(2, 0x2D6C0001ULL, 1, 1));
    }
    mCreatures.push_back(new MobSpawnerData(40, 0x2D6C0003ULL, 1, 2));
    mCreatures.push_back(new MobSpawnerData(10, 0x0D680004ULL, 4, 4));
}

JungleBiome::~JungleBiome()
{
}

void JungleBiome::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> jungleVariant(PlanksBlock::Variant::byData(3));
    sJungleTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(
        OldLogBlock_VARIANT, &jungleVariant);

    sJungleLeafState = Blocks::leaves->defaultBlockState()->setBoxedValue(
        OldLeafBlock_VARIANT, &jungleVariant);
    TypedBoxed<bool> decayable(false);
    sJungleLeafState = sJungleLeafState->setBoxedValue(
        OldLeafBlock_DECAYABLE, &decayable);

    TypedBoxed<PlanksBlock::Variant*> oakVariant(PlanksBlock::Variant::byData(0));
    sOakLeafState = Blocks::leaves->defaultBlockState()->setBoxedValue(
        OldLeafBlock_VARIANT, &oakVariant);
    sOakLeafState = sOakLeafState->setBoxedValue(
        OldLeafBlock_DECAYABLE, &decayable);
}

AbstractTreeFeature* JungleBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(10) == 0)
    {
        return new BigTreeFeature(false);
    }

    if (random.nextInt(2) == 0)
    {
        return new GroundBushFeature(sJungleTrunkState, sOakLeafState);
    }

    if (!mIsEdge && random.nextInt(3) == 0)
    {
        return new MegaJungleTreeFeature(false, 10, 20, sJungleTrunkState, sJungleLeafState);
    }

    const int height = random.nextInt(7);
    return new TreeFeature(false, height + 4, sJungleTrunkState, sJungleLeafState, true);
}

Feature* JungleBiome::getGrassFeature(Random& random)
{
    if (random.nextInt(4) == 0)
    {
        return new TallgrassFeature(gFern);
    }

    return new TallgrassFeature(gTallGrass);
}

void JungleBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    const int x = random.nextInt(16);
    const int z = random.nextInt(16);
    BlockPos melonPos = pos.offset(x + 8, 0, z + 8);
    melonPos = level->getHeightmapPos(melonPos);
    const int y = melonPos.y > 0 ? random.nextInt() : 0;
    MelonFeature melon;
    melon.place(level, random, pos.offset(x + 8, y, z + 8));

    PIXBeginNamedEvent(0.0F, "Adding vines");
    VinesFeature vines;
    int count = 6;
    do
    {
        for (int i = 0; i < 8; ++i)
        {
            vines.place(level, random, pos.offset(random.nextInt(16) + 8, 64,
                                                   random.nextInt(16) + 8));
        }
        --count;
    }
    while (count != 0);

    count = 2;
    do
    {
        vines.place(level, random, pos.offset(random.nextInt(16) + 8, 64,
                                               random.nextInt(16) + 8));
        --count;
    }
    while (count != 0);
    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass JungleBiome::getBaseClass()
{
    return eBiomeBaseClass_Jungle;
}
