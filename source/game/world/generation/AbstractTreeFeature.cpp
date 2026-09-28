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
#include "game/world/block/EnumProperty.hpp"
#include "game/world/block/LogBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/MutableBlockPos.hpp"
#include "game/world/block/OldLogBlock.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/VineBlock.hpp"

AbstractTreeFeature::AbstractTreeFeature(bool doBlockNotify, int baseHeight,
                                         const BlockState* trunkState,
                                         const BlockState* leafState, bool hasVines)
    : Feature(doBlockNotify),
      mBaseHeight(baseHeight),
      mHasVines(hasVines),
      mTrunkState(trunkState),
      mLeafState(leafState)
{
}

bool AbstractTreeFeature::isFree(Block* block)
{
    Material* material = block->defaultBlockState()->getMaterial();
    return material == Material::AIR || material == Material::LEAVES ||
           block == Blocks::grass || block == Blocks::dirt ||
           block == Blocks::log || block == Blocks::log2 ||
           block == Blocks::sapling || block == Blocks::vine;
}

void AbstractTreeFeature::postPlaceTree(Level*, Random&, const BlockPos&)
{
}

void AbstractTreeFeature::setDirtAt(Level* level, const BlockPos& pos)
{
    if (level->getBlockState(pos)->getBlock() != Blocks::dirt)
    {
        setBlock(level, pos, Blocks::dirt->defaultBlockState(), Block::UPDATE_NONE);
    }
}

void AbstractTreeFeature::placeFallenTrunk(Level* level, const BlockPos& pos,
                                           Random& random, int length)
{
    MutableBlockPos start(pos.x, pos.y, pos.z);
    const Direction* direction = Direction::Plane::HORIZONTAL->getRandomDirection(random);
    int remaining = length - 2;
    BlockPos moved = *start.move(direction, random.nextInt(2) + 2);

    int height = level->getHeightmap(moved.x, moved.z);
    if (height > pos.y + 1)
    {
        return;
    }

    MutableBlockPos cursor(start);
    cursor.set(moved.x, height, moved.z);

    int unsupported = 0;
    for (int i = 0; i < remaining; ++i)
    {
        cursor.move(direction, 1);
        if (level->getBlockState(cursor)->getMaterial() != Material::AIR)
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

    cursor.set(moved.x, height, moved.z);
    for (int i = 0; i < remaining; ++i)
    {
        cursor.move(direction, 1);

        LogBlock::LogAxis* logAxis = direction->getAxis()->getId() == Direction::Axis::eAxis_X ?
                LogBlock::X : LogBlock::Z;
        TypedBoxed<LogBlock::LogAxis*> axisValue(logAxis);
        setBlock(level, cursor, mTrunkState->setBoxedValue(LogBlock::AXIS, &axisValue), Block::UPDATE_NONE);

        if (random.nextInt(10) == 0 &&
            level->getBlockState(cursor.above())->getMaterial() == Material::AIR)
        {
            if (random.nextFloat() >= 0.5f)
            {
                setBlock(level, cursor.above(), Blocks::red_mushroom->defaultBlockState(), Block::UPDATE_NONE);
            }
            else
            {
                setBlock(level, cursor.above(), Blocks::brown_mushroom->defaultBlockState(), Block::UPDATE_NONE);
            }
        }
    }
}

void AbstractTreeFeature::addVine(Level* level, const BlockPos& pos,
                                  const BooleanProperty* property)
{
    TypedBoxed<bool> value(true);
    const BlockState* state = Blocks::vine->defaultBlockState()->setBoxedValue(property, &value);
    setBlock(level, pos, state, Block::UPDATE_NONE);
}

bool AbstractTreeFeature::placeTrunk(Level* level, const BlockPos& pos,
                                     Random& random, int height)
{
    bool fallen = random.nextInt(80) == 0;

    float vineChance = 0.0f;
    if (mTrunkState->getValue(OldLogBlock::VARIANT) != PlanksBlock::Variant::BIRCH)
    {
        if (fallen)
        {
            vineChance = 0.75f;
        }
        else if (mHasVines)
        {
            vineChance = 1.0f / 3.0f;
        }
        else if (random.nextInt(12) == 0)
        {
            vineChance = 1.0f;
        }
    }

    if (fallen)
    {
        placeFallenTrunk(level, pos, random, height);
        height = 1;
    }

    for (int i = 0; i < height; ++i)
    {
        Material* material = level->getBlockState(pos.above(i))->getMaterial();
        if (material == Material::AIR || material == Material::LEAVES ||
            material == Material::REPLACEABLE_PLANT || material == Material::DIRT ||
            material == Material::GRASS)
        {
            setBlock(level, pos.above(i), mTrunkState, Block::UPDATE_NONE);

            if (vineChance != 0.0f)
            {
                if (random.nextFloat() < vineChance && level->isEmptyBlock(pos.offset(-1, i, 0)))
                {
                    addVine(level, pos.offset(-1, i, 0),
                            static_cast<const BooleanProperty*>(VineBlock::EAST));
                }
                if (random.nextFloat() < vineChance && level->isEmptyBlock(pos.offset(1, i, 0)))
                {
                    addVine(level, pos.offset(1, i, 0),
                            static_cast<const BooleanProperty*>(VineBlock::WEST));
                }
                if (random.nextFloat() < vineChance && level->isEmptyBlock(pos.offset(0, i, -1)))
                {
                    addVine(level, pos.offset(0, i, -1),
                            static_cast<const BooleanProperty*>(VineBlock::SOUTH));
                }
                if (random.nextFloat() < vineChance && level->isEmptyBlock(pos.offset(0, i, 1)))
                {
                    addVine(level, pos.offset(0, i, 1),
                            static_cast<const BooleanProperty*>(VineBlock::NORTH));
                }
            }
        }
    }

    return !fallen;
}
