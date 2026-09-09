#include "game/world/generation/SpruceFeature.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

extern CMinecraftApp* g_pApp;
extern Property* OldLogBlock_VARIANT;
extern Property* OldLeafBlock_VARIANT;
extern Property* OldLeafBlock_DECAYABLE;

static const BlockState* spruceTrunkState = NULL;
static const BlockState* spruceLeafState = NULL;

SpruceFeature::SpruceFeature(bool doBlockNotify)
    : AbstractTreeFeature(doBlockNotify, 6, spruceTrunkState, spruceLeafState, false)
{
}

SpruceFeature::~SpruceFeature()
{
}

void SpruceFeature::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(1));
    spruceTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(
        OldLogBlock_VARIANT, &variant);
    spruceLeafState = Blocks::leaves->defaultBlockState()->setBoxedValue(
        OldLeafBlock_VARIANT, &variant);
    TypedBoxed<bool> decayable(false);
    spruceLeafState = spruceLeafState->setBoxedValue(
        OldLeafBlock_DECAYABLE, &decayable);
}

bool SpruceFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    const int height = random.nextInt(4) + mTreeType;
    const int foliageStart = height - (random.nextInt(2) + 1);
    const int foliageRadius = random.nextInt(2) + 2;

    if (pos.y <= 0 || pos.y + height + 1 >= 257)
    {
        return false;
    }

    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->checkIntersects(pos.offset(-foliageRadius, -1, -foliageRadius),
                                                     pos.offset(foliageRadius, height, foliageRadius)))
    {
        return false;
    }

    for (int y = 0; y <= height + 1; ++y)
    {
        int radius = 0;
        if (y >= foliageStart + 1)
        {
            radius = foliageRadius;
        }
        for (int x = -radius; x <= radius; ++x)
        {
            for (int z = -radius; z <= radius; ++z)
            {
                if (pos.y + y >= 256 ||
                    !isFree(level->getBlock(pos.offset(x, y, z))))
                {
                    return false;
                }
            }
        }
    }

    Block* ground = level->getBlock(pos.below());
    if ((ground != Blocks::grass && ground != Blocks::dirt && ground != Blocks::mycelium) ||
        pos.y >= 255 - height)
    {
        return false;
    }

    setDirtAt(level, pos.below());
    if (!placeTrunk(level, pos, random, height - random.nextInt(3)))
    {
        return false;
    }

    int radius = 0;
    int radiusLimit = 0;
    int topY = pos.y + height;
    for (int y = topY; y >= pos.y + foliageStart; --y)
    {
        const int layer = topY - y;
        if (layer == 0)
        {
            radius = 0;
        }
        else if (layer & 1)
        {
            ++radiusLimit;
        }
        if (radius < foliageRadius)
        {
            ++radius;
        }

        for (int x = pos.x - radius; x <= pos.x + radius; ++x)
        {
            for (int z = pos.z - radius; z <= pos.z + radius; ++z)
            {
                if ((x - pos.x != radius || z - pos.z != radius || radius < 1) &&
                    (x - pos.x != -radius || z - pos.z != -radius || radius < 1) &&
                    (x - pos.x != -radius || z - pos.z != radius || radius < 1) &&
                    (x - pos.x != radius || z - pos.z != -radius || radius < 1))
                {
                    BlockPos leafPos(x, y, z);
                    if (isFree(level->getBlock(leafPos)))
                    {
                        setBlock(level, leafPos, mLeafState, 0);
                    }
                }
            }
        }

        if (radiusLimit > 0 && radius > 1)
        {
            --radius;
            radiusLimit = 0;
        }
    }

    return true;
}
