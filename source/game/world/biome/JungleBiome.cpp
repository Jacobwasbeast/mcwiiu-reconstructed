#include "game/world/biome/JungleBiome.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/MegaJungleTreeFeature.hpp"
#include "game/world/generation/MelonFeature.hpp"
#include "game/world/generation/TallgrassFeature.hpp"
#include "game/world/generation/VinesFeature.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

static const BlockState* sJungleTrunkState;
static const BlockState* sJungleLeafState;
static const BlockState* sJungleLeafStateAlt;

JungleBiome::JungleBiome(EBiomeIDs biomeId, bool isEdge, BiomeProperties* properties)
    : Biome(biomeId, eBiomeType_Jungle, properties),
      mIsEdge(isEdge)
{
    mDecorator->treesPerChunk = isEdge ? 2 : 50;
    mDecorator->grassPerChunk = 4;
    mDecorator->flowersPerChunk = 25;

    if (!isEdge)
    {
        mCreatures.push_back(new MobSpawnerData(2, 0x2D6C0001ULL, 1, 1));
    }
    mCreatures.push_back(new MobSpawnerData(40, 0x2D6C0003ULL, 1, 2));
    mCreatures.push_back(new MobSpawnerData(10, 0x0D680004ULL, 4, 4));
}

JungleBiome::~JungleBiome()
{
}

void JungleBiome::staticCtor()
{
    sJungleTrunkState = Blocks::log2->defaultBlockState();
    sJungleLeafState = Blocks::leaves2->defaultBlockState();
    sJungleLeafStateAlt = Blocks::leaves2->defaultBlockState();
}

AbstractTreeFeature* JungleBiome::getTreeFeature(Random& random)
{
    if (random.nextInt(10) == 0)
    {
        return Biome::getTreeFeature(random);
    }

    if (random.nextInt(2) == 0)
    {
        return Biome::getTreeFeature(random);
    }

    if (!mIsEdge && random.nextInt(3) == 0)
    {
        return new MegaJungleTreeFeature(false, 0, 10, sJungleTrunkState, sJungleLeafStateAlt);
    }

    random.nextInt(7);
    return Biome::getTreeFeature(random);
}

Feature* JungleBiome::getGrassFeature(Random& random)
{
    if (random.nextInt(4) == 0)
    {
        return Biome::getGrassFeature(random);
    }

    return Biome::getGrassFeature(random);
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
