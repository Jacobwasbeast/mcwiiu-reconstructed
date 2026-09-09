#include "game/world/block/PistonMagicBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/util/AABB.hpp"
#include "game/util/Random.hpp"
#include "game/util/Vec3.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/AbstractProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"

PistonMagicBlock::PistonMagicBlock()
    : BaseEntityBlock(NULL), EntityBlock()
{
    DerivedInit();
    const BlockState* state = defaultBlockState();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> extended(false);
    if (state != NULL)
    {
        state = state->setBoxedValue(PistonBaseBlock::FACING, &facing);
        state = state->setBoxedValue(PistonBaseBlock::EXTENDED, &extended);
        registerDefaultState(state);
    }
    setDestroyTime(0.4f);
}

PistonMagicBlock::~PistonMagicBlock()
{
}

boost::shared_ptr<BlockEntity> PistonMagicBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<BlockEntity>();
}

boost::shared_ptr<BlockEntity> PistonMagicBlock::newMovingBlockEntity(const BlockState* state,
                                                                       const Direction* direction,
                                                                       bool extending, bool source,
                                                                       CompoundTag* lastTick)
{
    return boost::shared_ptr<BlockEntity>(new PistonMovingBlockEntity(state, direction, extending, source, lastTick));
}

void PistonMagicBlock::blockStaticCtor()
{
}

void PistonMagicBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    boost::shared_ptr<PistonMovingBlockEntity> moving = boost::dynamic_pointer_cast<PistonMovingBlockEntity>(entity);
    if (moving)
        moving->finalTick();
    else
        BaseEntityBlock::onRemove(level, pos, state);
}

bool PistonMagicBlock::mayPlace(Level*, const BlockPos&)
{
    return false;
}

bool PistonMagicBlock::mayPlace(Level*, const BlockPos&, const Direction*)
{
    return false;
}

void PistonMagicBlock::destroy(Level* level, const BlockPos& pos, const BlockState* state)
{
    const Direction* direction = state->getValue(PistonBaseBlock::FACING);
    BlockPos adjacent = pos.relative(direction->getOpposite());
    const BlockState* adjacentState = level->getBlockState(adjacent);
    if (adjacentState != NULL && adjacentState->getBlock() == this)
    {
        if (adjacentState->getValue(PistonBaseBlock::EXTENDED))
            level->removeBlock(adjacent);
    }
}

bool PistonMagicBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool PistonMagicBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool PistonMagicBlock::use(Level* level, const BlockPos& pos, const BlockState* state,
                           boost::shared_ptr<Player>, InteractionHand::EInteractionHand,
                           const Direction*, float, float, float, bool)
{
    if (level->getBlockEntity(pos).get() == NULL)
        return false;
    level->removeBlockEntity(pos);
    return true;
}

Item* PistonMagicBlock::getResource(const BlockState*, Random*, int)
{
    return Items::air;
}

boost::shared_ptr<PistonMovingBlockEntity> PistonMagicBlock::getEntity(LevelSource* levelSource,
                                                                        const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> entity = levelSource->getBlockEntity(pos);
    return boost::dynamic_pointer_cast<PistonMovingBlockEntity>(entity);
}

void PistonMagicBlock::spawnResources(Level* level, const BlockPos& pos, const BlockState* state,
                                      float, int)
{
    if (level->mIsClientSide)
        return;
    boost::shared_ptr<PistonMovingBlockEntity> moving = getEntity(level, pos);
    if (moving)
    {
        const BlockState* movedState = moving->getBlockState();
        if (movedState != NULL)
            movedState->getBlock()->spawnResources(level, pos, movedState, 0);
    }
}

HitResult* PistonMagicBlock::clip(const BlockState*, Level*, const BlockPos&, Vec3*, Vec3*)
{
    return NULL;
}

void PistonMagicBlock::neighborChanged(const BlockState*, Level* level, const BlockPos& pos,
                                       Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
        level->getBlockEntity(pos);
}

const AABB* PistonMagicBlock::getClipAABB(const BlockState*, LevelSource* levelSource,
                                           const BlockPos& pos)
{
    boost::shared_ptr<PistonMovingBlockEntity> moving = getEntity(levelSource, pos);
    return moving ? moving->getAABB(levelSource, pos) : NULL;
}

void PistonMagicBlock::addCollisionAABBs(const BlockState*, Level* level, const BlockPos& pos,
                                         const AABB* entityBox, std::vector<AABB*>* collidingBoxes,
                                         boost::shared_ptr<Entity> entity, bool)
{
    boost::shared_ptr<PistonMovingBlockEntity> moving = getEntity(level, pos);
    if (moving)
        moving->addCollissionAABBs(level, pos, entityBox, collidingBoxes, entity);
}

const AABB* PistonMagicBlock::getShape(const BlockState*, LevelSource* levelSource,
                                        const BlockPos& pos)
{
    boost::shared_ptr<PistonMovingBlockEntity> moving = getEntity(levelSource, pos);
    return moving ? moving->getAABB(levelSource, pos) : NULL;
}

bool PistonMagicBlock::canContainLiquid(const BlockState*) const
{
    return true;
}

not_null_ptr<ItemInstance> PistonMagicBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>();
}

const BlockState* PistonMagicBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(PistonHeadBlock::getFacing(data));
    TypedBoxed<bool> extended((data & 8) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(PistonBaseBlock::FACING, &facing);
    return state->setBoxedValue(PistonBaseBlock::EXTENDED, &extended);
}

int PistonMagicBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(PistonBaseBlock::FACING)->get3DDataValue();
    if (state->getValue(PistonBaseBlock::EXTENDED))
        data |= 8;
    return data;
}

const BlockState* PistonMagicBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(PistonBaseBlock::FACING)));
    return state->setBoxedValue(PistonBaseBlock::FACING, &facing);
}

const BlockState* PistonMagicBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(PistonBaseBlock::FACING)));
}

BlockStateDefinition* PistonMagicBlock::createBlockStateDefinition()
{
    const Property* properties[] = { PistonBaseBlock::FACING, PistonBaseBlock::EXTENDED };
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape PistonMagicBlock::getBlockFaceShape(LevelSource*, const BlockState*,
                                                    const BlockPos&, const Direction*)
{
    return (EBlockFaceShape)8;
}

void PistonMagicBlock::registerIcons(IconRegister*)
{
}

EBlockRenderShape PistonMagicBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)-1;
}
