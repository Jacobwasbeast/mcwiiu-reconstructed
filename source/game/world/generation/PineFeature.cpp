#include "game/world/generation/PineFeature.hpp"

#include <cstdlib>

#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/OldLeafBlock.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/core/PIXNamedEvents.hpp"

extern CMinecraftApp* g_pApp;

static const BlockState* pineTrunkState = NULL;
static const BlockState* pineLeafState = NULL;

PineFeature::PineFeature()
    : AbstractTreeFeature(false, 7, pineTrunkState, pineLeafState, false)
{
}

PineFeature::~PineFeature()
{
}

void PineFeature::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> trunkVariant(PlanksBlock::Variant::SPRUCE);
    pineTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(OldLogBlock::VARIANT, &trunkVariant);
    TypedBoxed<PlanksBlock::Variant*> leafVariant(PlanksBlock::Variant::SPRUCE);
    TypedBoxed<bool> decayable(false);
    pineLeafState = Blocks::leaves->defaultBlockState()
                        ->setBoxedValue(OldLeafBlock::VARIANT, &leafVariant)
                        ->setBoxedValue(LeafBlock::DECAYABLE, &decayable);
}

bool PineFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "PineFeature::place");

    const int height = random.nextInt(5) + mBaseHeight;
    const int leafStart = height - random.nextInt(2) - 3;
    const int leafRadius = random.nextInt(height - leafStart + 1) + 1;

    if (pos.y > 0 && pos.y + height + 1 <= 128)
    {
        if (g_pApp->getLevelGenerationOptions() != NULL &&
            g_pApp->getLevelGenerationOptions()->checkIntersects(
                BlockPos(pos.x - leafRadius, pos.y - 1, pos.z - leafRadius),
                BlockPos(pos.x + leafRadius, pos.y + height, pos.z + leafRadius)))
        {
            PIXEndNamedEvent();
            return false;
        }

        bool free = true;
        for (int y = pos.y; y <= pos.y + height + 1 && free; ++y)
        {
            int radius = 0;
            if (y - pos.y >= leafStart)
            {
                radius = leafRadius;
            }
            for (int x = pos.x - radius; x <= pos.x + radius && free; ++x)
            {
                for (int z = pos.z - radius; z <= pos.z + radius && free; ++z)
                {
                    if (y >= 0 && y < 128)
                    {
                        if (!isFree(level->getBlockState(BlockPos(x, y, z))->getBlock()))
                        {
                            free = false;
                        }
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

        Block* ground = level->getBlockState(pos.below())->getBlock();
        if ((ground == Blocks::grass || ground == Blocks::dirt) && pos.y < 127 - height)
        {
            setDirtAt(level, pos.below());

            int radius = 0;
            MutableBlockPos check(0, 0, 0);
            for (int y = pos.y + height; y >= pos.y + leafStart; --y)
            {
                for (int x = pos.x - radius; x <= pos.x + radius; ++x)
                {
                    for (int z = pos.z - radius; z <= pos.z + radius; ++z)
                    {
                        if (std::abs(x - pos.x) != radius || std::abs(z - pos.z) != radius || radius < 1)
                        {
                            check.set(x, y, z);
                            if (!level->getBlockState(check)->isSolid())
                            {
                                setBlock(level, check, mLeafState, Block::UPDATE_NONE);
                            }
                        }
                    }
                }

                if (radius >= 1 && y == pos.y + leafStart + 1)
                {
                    --radius;
                }
                else if (radius < leafRadius)
                {
                    ++radius;
                }
            }

            for (int trunkY = 0; trunkY < height - 1; ++trunkY)
            {
                Material* material = level->getBlockState(pos.above(trunkY))->getMaterial();
                if (material == Material::AIR || material == Material::LEAVES)
                {
                    setBlock(level, pos.above(trunkY), mTrunkState, Block::UPDATE_NONE);
                }
            }

            PIXEndNamedEvent();
            return true;
        }
    }

    PIXEndNamedEvent();
    return false;
}
