#include "game/world/generation/AbstractTreeFeature.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractTreeFeature::AbstractTreeFeature(bool doBlockNotify, int treeType,
                                         const BlockState* trunkState,
                                         const BlockState* leafState, bool hasVines)
    : Feature(doBlockNotify),
      mTreeType(treeType),
      mHasVines(hasVines),
      mTrunkState(trunkState),
      mLeafState(leafState)
{
}

AbstractTreeFeature::~AbstractTreeFeature()
{
}

bool AbstractTreeFeature::isFree(Block* block)
{
    const BlockState* state = block->defaultBlockState();
    Block* stateBlock = state->getBlock();
    return stateBlock == Blocks::AIR || stateBlock == Blocks::leaves ||
           stateBlock == Blocks::leaves2 || block == Blocks::WATER ||
           block == Blocks::FLOWING_WATER || block == Blocks::snow_layer ||
           block == Blocks::vine;
}

void AbstractTreeFeature::postPlaceTree(Level*, Random&, const BlockPos&)
{
}

void AbstractTreeFeature::setDirtAt(Level* level, const BlockPos& pos)
{
    const BlockState* state = level->getBlockState(pos);
    if (state->getBlock() != Blocks::dirt)
    {
        setBlock(level, pos, Blocks::dirt->defaultBlockState(), 0);
    }
}

void AbstractTreeFeature::placeFallenTrunk(Level* level, const BlockPos& pos,
                                           Random& random, int length)
{
    MutableBlockPos cursor(pos);
    const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    cursor.move(direction, random.nextInt(2) + 2);
    int height = level->getHeightmap(cursor.x, cursor.z);
    int remaining = length - 2;
    if (height > pos.y + 1 || remaining <= 0)
    {
        return;
    }

    cursor.setY(height);
    int unsupported = 0;
    for (int i = 0; i < remaining; ++i)
    {
        cursor.move(direction);
        Block* block = level->getBlock(cursor);
        if (!isFree(block))
        {
            return;
        }
        if (!level->isSolidBlockingBlockInLoadedChunk(cursor.below(), false))
        {
            ++unsupported;
            if (unsupported > 2)
            {
                return;
            }
        }
        else
        {
            unsupported = 0;
        }
    }

    cursor.setY(height);
    const Direction::Axis* axis = direction->getAxis();
    for (int i = 0; i < remaining / 2; ++i)
    {
        cursor.move(direction);
        const BlockState* trunkState = mTrunkState;
        setBlock(level, cursor, trunkState, 0);
        if (random.nextInt(10) == 0 && isFree(level->getBlock(cursor.above())))
        {
            setBlock(level, cursor.above(), random.nextFloat() >= 0.5f ?
                    Blocks::leaves->defaultBlockState() : Blocks::leaves2->defaultBlockState(), 0);
        }
        cursor.move(direction);
        trunkState = mTrunkState;
        setBlock(level, cursor, trunkState, 0);
        if (random.nextInt(10) == 0 && isFree(level->getBlock(cursor.above())))
        {
            setBlock(level, cursor.above(), random.nextFloat() >= 0.5f ?
                    Blocks::leaves->defaultBlockState() : Blocks::leaves2->defaultBlockState(), 0);
        }
    }
    if ((remaining & 1) != 0)
    {
        cursor.move(direction);
        setBlock(level, cursor, mTrunkState, 0);
    }
    (void)axis;
}

void AbstractTreeFeature::addVine(Level* level, const BlockPos& pos,
                                  const BooleanProperty* property)
{
    TypedBoxed<bool> value(true);
    const BlockState* state = Blocks::vine->defaultBlockState()->setBoxedValue(property, &value);
    setBlock(level, pos, state, 0);
}

bool AbstractTreeFeature::placeTrunk(Level* level, const BlockPos& pos,
                                     Random& random, int height)
{
    bool fallen = random.nextInt(80) == 0;
    float chance = 0.0f;
    if (mTrunkState != NULL && mTrunkState->getBlock() != Blocks::AIR)
    {
        if (fallen)
        {
            chance = 0.75f;
            placeFallenTrunk(level, pos, random, height);
            height = 1;
        }
        else if (mHasVines)
        {
            chance = 1.0f / 3.0f;
        }
        else if (random.nextInt(12) == 0)
        {
            chance = 1.0f;
        }
    }
    else if (fallen)
    {
        placeFallenTrunk(level, pos, random, height);
        height = 1;
    }

    for (int y = 0; y < height; ++y)
    {
        BlockPos trunkPos = pos.above(y);
        if (isFree(level->getBlock(trunkPos)))
        {
            setBlock(level, trunkPos, mTrunkState, 0);
            if (chance > 0.0f)
            {
                BlockPos side;
                side = pos.offset(-1, y, 0);
                if (level->getBlock(side) != NULL && random.nextFloat() < chance) addVine(level, side, NULL);
                side = pos.offset(1, y, 0);
                if (level->getBlock(side) != NULL && random.nextFloat() < chance) addVine(level, side, NULL);
                side = pos.offset(0, y, -1);
                if (level->getBlock(side) != NULL && random.nextFloat() < chance) addVine(level, side, NULL);
                side = pos.offset(0, y, 1);
                if (level->getBlock(side) != NULL && random.nextFloat() < chance) addVine(level, side, NULL);
            }
        }
    }
    return !fallen;
}
