#include "game/world/generation/SwampTreeFeature.hpp"

#include <cstdlib>

#include "game/util/Random.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/world/LevelGenerationOptions.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

extern CMinecraftApp* g_pApp;
extern Property* OldLogBlock_VARIANT;
extern Property* OldLeafBlock_VARIANT;
extern Property* OldLeafBlock_DECAYABLE;

static const BlockState* swampTrunkState = NULL;
static const BlockState* swampLeafState = NULL;

SwampTreeFeature::SwampTreeFeature()
    : AbstractTreeFeature(false, 5, swampTrunkState, swampLeafState, false)
{
}

SwampTreeFeature::~SwampTreeFeature()
{
}

void SwampTreeFeature::staticCtor()
{
    TypedBoxed<PlanksBlock::Variant*> variant(PlanksBlock::Variant::byData(0));
    swampTrunkState = Blocks::log->defaultBlockState()->setBoxedValue(
        OldLogBlock_VARIANT, &variant);
    swampLeafState = Blocks::leaves->defaultBlockState()->setBoxedValue(
        OldLeafBlock_VARIANT, &variant);
    TypedBoxed<bool> decayable(false);
    swampLeafState = swampLeafState->setBoxedValue(
        OldLeafBlock_DECAYABLE, &decayable);
}

void SwampTreeFeature::addVine(Level* level, const BlockPos& pos,
                               const BooleanProperty* property)
{
    TypedBoxed<bool> value(true);
    const BlockState* state = Blocks::vine->defaultBlockState();
    if (property != NULL)
    {
        state = state->setBoxedValue(property, &value);
    }
    setBlock(level, pos, state, 0);
    BlockPos vinePos = pos.below();
    for (int i = 4; i > 0; --i)
    {
        if (level->getBlock(vinePos) != Blocks::AIR)
        {
            break;
        }
        setBlock(level, vinePos, state, 0);
        vinePos = vinePos.below();
    }
}

bool SwampTreeFeature::place(Level* level, Random& random, const BlockPos& pos)
{
    int height = random.nextInt(4) + mBaseHeight;
    BlockPos cursor = pos;
    while (level->getBlock(cursor.below()) == Blocks::WATER)
    {
        cursor = cursor.below();
    }

    if (cursor.y <= 0 || cursor.y + height + 1 >= 128)
    {
        return false;
    }
    LevelGenerationOptions* options = g_pApp->getLevelGenerationOptions();
    if (options != NULL && options->checkIntersects(cursor.offset(-3, -1, -3), cursor.offset(3, height, 3)))
    {
        return false;
    }

    MutableBlockPos check;
    for (int y = cursor.y; y <= cursor.y + height + 1; ++y)
    {
        int radius = y == cursor.y ? 0 : (y >= cursor.y + height - 1 ? 3 : 1);
        for (int x = cursor.x - radius; x <= cursor.x + radius; ++x)
        {
            for (int z = cursor.z - radius; z <= cursor.z + radius; ++z)
            {
                check.set(x, y, z);
                if (!isFree(level->getBlock(check)))
                {
                    return false;
                }
            }
        }
    }

    Block* ground = level->getBlock(cursor.below());
    if ((ground != Blocks::grass && ground != Blocks::dirt) || cursor.y >= 127 - height)
    {
        return false;
    }
    setDirtAt(level, cursor.below());

    for (int y = cursor.y + height - 3; y <= cursor.y + height; ++y)
    {
        int radius = 2 - (y - (cursor.y + height));
        for (int x = cursor.x - radius; x <= cursor.x + radius; ++x)
        {
            for (int z = cursor.z - radius; z <= cursor.z + radius; ++z)
            {
                if ((std::abs(x - cursor.x) != radius || std::abs(z - cursor.z) != radius || radius <= 0 || random.nextInt(2) != 0))
                {
                    check.set(x, y, z);
                    if (level->getBlock(check) == Blocks::AIR)
                    {
                        setBlock(level, check, mLeafState, 0);
                    }
                }
            }
        }
    }

    for (int y = 0; y < height; ++y)
    {
        check.set(cursor.x, cursor.y + y, cursor.z);
        if (isFree(level->getBlock(check)))
        {
            setBlock(level, check, mTrunkState, 0);
        }
    }

    for (int y = 0; y < height / 2; ++y)
    {
        BlockPos vinePos = cursor.above(y * 2);
        if (isFree(level->getBlock(vinePos.west()))) addVine(level, vinePos.west(), NULL);
        if (isFree(level->getBlock(vinePos.east()))) addVine(level, vinePos.east(), NULL);
        if (isFree(level->getBlock(vinePos.north()))) addVine(level, vinePos.north(), NULL);
        if (isFree(level->getBlock(vinePos.south()))) addVine(level, vinePos.south(), NULL);
    }

    return true;
}
