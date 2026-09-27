#include "game/world/generation/BirchFeature.hpp"

#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/core/PIXNamedEvents.hpp"

#include <cstdlib>

extern CMinecraftApp* g_pApp;
static const BlockState* birchTrunkState = NULL;
static const BlockState* birchLeafState = NULL;

BirchFeature::BirchFeature(bool doBlockNotify, bool tallBirch)
    : AbstractTreeFeature(doBlockNotify, 5, birchTrunkState, birchLeafState, false),
      mTallBirch(tallBirch)
{
}

BirchFeature::~BirchFeature()
{
}

void BirchFeature::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> trunkVariant(PlanksBlock::Variant::BIRCH);
    birchTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(OldLogBlock::VARIANT, &trunkVariant);
    TypedBoxed<PlanksBlock::Variant*> leafVariant(PlanksBlock::Variant::BIRCH);
    TypedBoxed<bool> decayable(false);
    birchLeafState = Blocks::leaves->defaultBlockState()
                         ->setBoxedValue(OldLeafBlock::VARIANT, &leafVariant)
                         ->setBoxedValue(LeafBlock::DECAYABLE, &decayable);
}

bool BirchFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "BirchFeature::place");
    int height = random.nextInt(3) + mBaseHeight;
    if (mTallBirch)
    {
        height += random.nextInt(7);
    }

    bool free = true;
    if (origin.y > 0 && origin.y + height + 1 < 257)
    {
        MutableBlockPos check(0, 0, 0);
        for (int y = origin.y; y <= origin.y + height + 1; ++y)
        {
            int radius = 1;
            if (y == origin.y)
            {
                radius = 0;
            }
            if (y >= origin.y + height - 1)
            {
                radius = 2;
            }
            for (int x = origin.x - radius; x <= origin.x + radius && free; ++x)
            {
                for (int z = origin.z - radius; z <= origin.z + radius && free; ++z)
                {
                    if (y >= 0 && y < 256)
                    {
                        check.set(x, y, z);
                        if (!isFree(level->getBlockState(check)->getBlock()))
                            free = false;
                    }
                    else
                    {
                        free = false;
                    }
                }
            }
        }

        if (!free)
        {
            PIXEndNamedEvent();
            return false;
        }

        Block* ground = level->getBlockState(origin.below())->getBlock();
        if ((ground == Blocks::grass || ground == Blocks::dirt || ground == Blocks::farmland) &&
            origin.y < 255 - height)
        {
            if (g_pApp->getLevelGenerationOptions() != NULL &&
                g_pApp->getLevelGenerationOptions()->checkIntersects(origin.offset(-3, -1, -3),
                                                                     origin.offset(3, height, 3)))
            {
                PIXEndNamedEvent();
                return false;
            }

            setDirtAt(level, origin.below());
            if (!placeTrunk(level, origin, random, height))
                return true;

            MutableBlockPos leaves(0, 0, 0);
            const int top = origin.y + height;
            for (int y = origin.y + height - 3; y <= top; ++y)
            {
                const int radius = 1 - (y - top) / 2;
                for (int x = origin.x - radius; x <= origin.x + radius; ++x)
                {
                    for (int z = origin.z - radius; z <= origin.z + radius; ++z)
                    {
                        if (std::abs(x - origin.x) != radius ||
                            std::abs(z - origin.z) != radius ||
                            (random.nextInt(2) != 0 && y != top))
                        {
                            leaves.set(x, y, z);
                            Material* material = level->getBlockState(leaves)->getMaterial();
                            if (material == Material::AIR || material == Material::LEAVES)
                            {
                                setBlock(level, leaves, mLeafState, Block::UPDATE_NONE);
                            }
                        }
                    }
                }
            }
            PIXEndNamedEvent();
            return true;
        }
    }

    PIXEndNamedEvent();
    return false;
}
