#include "game/world/generation/PineFeature.hpp"

#include <cstdlib>

#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/MutableBlockPos.hpp"

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

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
    pineTrunkState = Blocks::log->getBlockState(1);
    pineLeafState = Blocks::leaves->getBlockState(1);
}

bool PineFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    PIXBeginNamedEvent(0.0f, "PineFeature::place");

    const int height = random.nextInt(5) + mTreeType;
    const int leafStart = height - random.nextInt(2) - 3;
    const int leafRadius = random.nextInt(height - leafStart + 1) + 1;

    if (pos.y > 0 && pos.y + height + 1 < 128)
    {
        LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
        if (options != NULL && options->checkIntersects(pos.offset(-leafRadius, -1, -leafRadius),
                                                         pos.offset(leafRadius, height, leafRadius)))
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
                for (int z = pos.z - radius; z <= pos.z + radius; ++z)
                {
                    if (y < 128)
                    {
                        if (!isFree(level->getBlock(MutableBlockPos(x, y, z))))
                        {
                            free = false;
                            break;
                        }
                    }
                    else
                    {
                        free = false;
                        break;
                    }
                }
            }
        }

        Block* ground = level->getBlock(pos.below());
        if (free && (ground == Blocks::grass || ground == Blocks::dirt) &&
            pos.y < 127 - height)
        {
            setDirtAt(level, pos.below());

            MutableBlockPos check;
            int radius = 0;
            int y = pos.y + height;
            const int bottom = pos.y + leafStart;
            while (y >= bottom)
            {
                for (int x = pos.x - radius; x <= pos.x + radius; ++x)
                {
                    for (int z = pos.z - radius; z <= pos.z + radius; ++z)
                    {
                        if (std::abs(x - pos.x) != radius ||
                            std::abs(z - pos.z) != radius || radius < 1)
                        {
                            check.set(x, y, z);
                            Block* block = level->getBlock(check);
                            if (block == Blocks::AIR || block == Blocks::leaves)
                            {
                                setBlock(level, check, mLeafState, 0);
                            }
                        }
                    }
                }

                if (radius < 1 || y != bottom + 1)
                {
                    if (radius < leafRadius)
                    {
                        ++radius;
                    }
                }
                else
                {
                    --radius;
                }
                --y;
            }

            for (int trunkY = 0; trunkY < height - 1; ++trunkY)
            {
                check.set(pos.x, pos.y + trunkY, pos.z);
                if (isFree(level->getBlock(check)))
                {
                    setBlock(level, check, mTrunkState, 0);
                }
            }

            PIXEndNamedEvent();
            return true;
        }
    }

    PIXEndNamedEvent();
    return false;
}
