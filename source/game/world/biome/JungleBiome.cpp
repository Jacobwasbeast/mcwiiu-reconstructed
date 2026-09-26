#include "game/world/biome/JungleBiome.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/PIXNamedEvents.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/BiomeDecorator.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TallGrassBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/BigTreeFeature.hpp"
#include "game/world/generation/GroundBushFeature.hpp"
#include "game/world/generation/MegaJungleTreeFeature.hpp"
#include "game/world/generation/MelonFeature.hpp"
#include "game/world/generation/TallgrassFeature.hpp"
#include "game/world/generation/TreeFeature.hpp"
#include "game/world/generation/VinesFeature.hpp"

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
        mMonsters.push_back(new MobSpawnerData(2, EntityType::OCELOT, 1, 1));
    }
    mCreatures.push_back(new MobSpawnerData(40, EntityType::PARROT, 1, 2));
    mCreatures.push_back(new MobSpawnerData(10, EntityType::CHICKEN, 4, 4));
}

JungleBiome::~JungleBiome()
{
}

void JungleBiome::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> trunkVariant(PlanksBlock::Variant::JUNGLE);
    sJungleTrunkState =
        Blocks::log->defaultBlockState()->setBoxedValue(OldLogBlock_VARIANT, &trunkVariant);

    TypedBoxed<PlanksBlock::Variant*> jungleLeafVariant(PlanksBlock::Variant::JUNGLE);
    TypedBoxed<bool> jungleLeafDecayable(false);
    sJungleLeafState = Blocks::leaves->defaultBlockState()
                           ->setBoxedValue(OldLeafBlock_VARIANT, &jungleLeafVariant)
                           ->setBoxedValue(LeafBlock::DECAYABLE, &jungleLeafDecayable);

    TypedBoxed<PlanksBlock::Variant*> oakLeafVariant(PlanksBlock::Variant::OAK);
    TypedBoxed<bool> oakLeafDecayable(false);
    sOakLeafState = Blocks::leaves->defaultBlockState()
                        ->setBoxedValue(OldLeafBlock_VARIANT, &oakLeafVariant)
                        ->setBoxedValue(LeafBlock::DECAYABLE, &oakLeafDecayable);
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

    int height = random.nextInt(7);
    return new TreeFeature(false, height + 4, sJungleTrunkState, sJungleLeafState, true);
}

Feature* JungleBiome::getGrassFeature(Random& random)
{
    if (random.nextInt(4) == 0)
    {
        return new TallgrassFeature(TallGrassBlock::Variant::FERN);
    }

    return new TallgrassFeature(TallGrassBlock::Variant::GRASS);
}

void JungleBiome::decorate(Level* level, Random& random, const BlockPos& pos)
{
    Biome::decorate(level, random, pos);

    int x = random.nextInt(16) + 8;
    int z = random.nextInt(16) + 8;
    int melonRange = level->getHeightmapPos(pos.offset(x, 0, z)).y * 2;
    int y = melonRange > 0 ? random.nextInt(melonRange) : 0;
    MelonFeature melon;
    melon.place(level, random, pos.offset(x, y, z));

    PIXBeginNamedEvent(0.0F, "Adding vines");
    VinesFeature vines;
    for (int i = 0; i < 50; ++i)
    {
        int vineX = random.nextInt(16) + 8;
        int vineZ = random.nextInt(16) + 8;
        vines.place(level, random, pos.offset(vineX, 64, vineZ));
    }
    PIXEndNamedEvent();
}

Biome::EBiomeBaseClass JungleBiome::getBaseClass()
{
    return eBiomeBaseClass_Jungle;
}
