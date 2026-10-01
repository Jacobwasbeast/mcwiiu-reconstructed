#include "game/world/block/StandingSignBlock.hpp"

#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<int>* StandingSignBlock::ROTATION =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"rotation", 0, 15));

StandingSignBlock::StandingSignBlock()
    : SignBlock()
{
    DerivedInit();
    TypedBoxed<int> rotation(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(ROTATION, &rotation));
}

void StandingSignBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                        Block* block, const BlockPos& fromPos)
{
    if (!level->getBlockState(pos.below())->isSolid())
    {
        spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }

    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* StandingSignBlock::getBlockState(int data)
{
    TypedBoxed<int> rotation(data);
    return defaultBlockState()->setBoxedValue(ROTATION, &rotation);
}

int StandingSignBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(ROTATION);
}

const BlockState* StandingSignBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<int> value(rotation->rotate(state->getValue(ROTATION), 16));
    return state->setBoxedValue(ROTATION, &value);
}

const BlockState* StandingSignBlock::mirror(const BlockState* state, Mirror* mirror)
{
    TypedBoxed<int> value(mirror->mirror(state->getValue(ROTATION), 16));
    return state->setBoxedValue(ROTATION, &value);
}

BlockStateDefinition* StandingSignBlock::createBlockStateDefinition()
{
    const Property* properties[] = { ROTATION };
    return new BlockStateDefinition(this, properties, 1);
}
