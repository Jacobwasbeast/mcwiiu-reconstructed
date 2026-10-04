#include "game/world/block/StandingBannerBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/IntegerProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"

static AbstractProperty<int>* gStandingBannerRotation =
    static_cast<AbstractProperty<int>*>(IntegerProperty::create(L"rotation", 0, 15));
static const AABB* gStandingBannerShape = AABB::newPermanent(0.25, 0.0, 0.25, 0.75, 1.0, 0.75);

StandingBannerBlock::StandingBannerBlock()
    : BannerBlock()
{
    DerivedInit();
    registerDefaultState(mStateDefinition->any()->setValue(gStandingBannerRotation, 0));
}

const AABB* StandingBannerBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return gStandingBannerShape;
}

const BlockState* StandingBannerBlock::rotate(const BlockState* state, Rotation* rotation)
{
    DirectionProperty* facing = BannerBlock::FACING;
    return state->setValue(facing, rotation->rotate(state->getValue(facing)));
}

const BlockState* StandingBannerBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(BannerBlock::FACING)));
}

void StandingBannerBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                          Block* block, const BlockPos& fromPos)
{
    const BlockState* below = level->getBlockState(pos.below());
    if (!below->getMaterial()->isSolid())
    {
        static_cast<Block*>(this)->spawnResources(level, pos, state, 0);
        level->removeBlock(pos);
    }
    Block::neighborChanged(state, level, pos, block, fromPos);
}

const BlockState* StandingBannerBlock::getBlockState(int data)
{
    return defaultBlockState()->setValue(gStandingBannerRotation, data);
}

int StandingBannerBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(gStandingBannerRotation);
}

BlockStateDefinition* StandingBannerBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { gStandingBannerRotation };
    return new BlockStateDefinition(this, properties, 1);
}
