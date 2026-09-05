#include "game/world/generation/BirchFeature.hpp"

#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/MutableBlockPos.hpp"

#include <cstdlib>

void PIXBeginNamedEvent(float colour, const char* name, ...);
void PIXEndNamedEvent();

class CMinecraftApp
{
public:
    LevelGenerationOptions* getLevelGenerationOptions();
};

class LevelGenerationOptions
{
public:
    bool checkIntersects(const BlockPos& min, const BlockPos& max);
};

extern CMinecraftApp* g_pApp;
extern Property* OldLogBlock_VARIANT;
extern Property* OldLeafBlock_VARIANT;
extern Property* OldLeafBlock_DECAYABLE;

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
    TypedBoxed<PlanksBlock::Variant*> birchVariant(PlanksBlock::Variant::byData(2));
    birchTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(
        OldLogBlock_VARIANT, &birchVariant);

    birchLeafState = Blocks::leaves->defaultBlockState()->setBoxedValue(
        OldLeafBlock_VARIANT, &birchVariant);
    TypedBoxed<bool> decayable(false);
    birchLeafState = birchLeafState->setBoxedValue(
        OldLeafBlock_DECAYABLE, &decayable);
}

bool BirchFeature::place(Level* level, Random& random, const BlockPos& origin)
{
    PIXBeginNamedEvent(0.0f, "BirchFeature::place");
    int height = random.nextInt(3) + mTreeType;
    if (mTallBirch)
    {
        height += random.nextInt(7);
    }

    bool free = true;
    if (origin.y > 0 && origin.y + height + 1 < 257)
    {
        MutableBlockPos check(0, 0, 0);
        for (int y = origin.y; y <= origin.y + height + 1 && free; ++y)
        {
            if (y < 256)
            {
                int radius = y == origin.y ? 0 : (y >= origin.y + height - 1 ? 2 : 1);
                for (int x = origin.x - radius; x <= origin.x + radius && free; ++x)
                {
                    for (int z = origin.z - radius; z <= origin.z + radius; ++z)
                    {
                        check.set(x, y, z);
                        if (!isFree(level->getBlock(check)))
                        {
                            free = false;
                            break;
                        }
                    }
                }
            }
        }

        Block* ground = level->getBlock(origin.below());
        if (free && (ground == Blocks::grass || ground == Blocks::dirt ||
                     ground == Blocks::mycelium) && origin.y < 255 - height)
        {
            LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
            if (options != NULL && options->checkIntersects(
                    origin.offset(-3, -1, -3), origin.offset(3, height, 3)))
            {
                free = false;
            }
            if (free)
            {
                setDirtAt(level, origin.below());
                if (!placeTrunk(level, origin, random, height))
                {
                    PIXEndNamedEvent();
                    return true;
                }

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
                                Block* block = level->getBlock(leaves);
                                if (block == Blocks::leaves || block == Blocks::leaves2)
                                {
                                    setBlock(level, leaves, mLeafState, 0);
                                }
                            }
                        }
                    }
                }
                PIXEndNamedEvent();
                return true;
            }
        }
    }

    PIXEndNamedEvent();
    return false;
}
