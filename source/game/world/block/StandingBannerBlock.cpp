#include "game/world/block/StandingBannerBlock.hpp"

#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* gStandingBannerRotation =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"rotation", 0, 15));
static const AABB* gStandingBannerShape = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 1.0, 0.75);

StandingBannerBlock::StandingBannerBlock()
    : BannerBlock()
{
    DerivedInit();
    TypedBoxed<int> rotation(0);
    registerDefaultState(mStateDefinition->any()->setBoxedValue(gStandingBannerRotation, &rotation));
}

StandingBannerBlock::~StandingBannerBlock()
{
}

const AABB* StandingBannerBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gStandingBannerShape;
}

const BlockState* StandingBannerBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<int> value(rotation->rotate(state->getValue(gStandingBannerRotation), 16));
    return state->setBoxedValue(gStandingBannerRotation, &value);
}

const BlockState* StandingBannerBlock::mirror(const BlockState* state, Mirror* mirror)
{
    TypedBoxed<int> value(mirror->mirror(state->getValue(gStandingBannerRotation), 16));
    return state->setBoxedValue(gStandingBannerRotation, &value);
}

void StandingBannerBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                          Block* block, const BlockPos& fromPos)
{
    const BlockState* below = level->getBlockState(pos.below());
    if (!below->isSolidBlockingCube() && !Block::isExceptionForTopSolidBlocking(below->getBlock()))
    {
        Block::spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* StandingBannerBlock::getBlockState(int data)
{
    TypedBoxed<int> rotation(data);
    return defaultBlockState()->setBoxedValue(gStandingBannerRotation, &rotation);
}

int StandingBannerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gStandingBannerRotation);
}

BlockStateDefinition* StandingBannerBlock::createBlockStateDefinition()
{
    const Property* properties[] = { gStandingBannerRotation };
    return new BlockStateDefinition(this, properties, 1);
}
