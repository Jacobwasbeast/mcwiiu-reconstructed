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
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PistonBaseBlock.hpp"
#include "game/world/block/PistonHeadBlock.hpp"
#include "game/world/block/PistonMovingBlockEntity.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<const Direction*>* PistonMagicBlock::FACING = NULL;
AbstractProperty<PistonHeadBlock::PistonType*>* PistonMagicBlock::TYPE = NULL;

PistonMagicBlock::PistonMagicBlock()
    : BaseEntityBlock(Material::PISTON), EntityBlock()
{
    DerivedInit();
    const BlockState* state = mStateDefinition->any();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    state = state->setBoxedValue(FACING, &facing);
    TypedBoxed<PistonHeadBlock::PistonType*> type(pistonTypeNormal);
    registerDefaultState(state->setBoxedValue(TYPE, &type));
    setDestroyTime(-1.0f);
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
    FACING = DirectionalBlock::FACING;
    TYPE = PistonHeadBlock::TYPE;
}

void PistonMagicBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    if (boost::dynamic_pointer_cast<PistonMovingBlockEntity>(entity))
    {
        boost::shared_ptr<PistonMovingBlockEntity> moving = boost::dynamic_pointer_cast<PistonMovingBlockEntity>(entity);
        moving->finalTick();
    }
    else
    {
        BaseEntityBlock::onRemove(level, pos, state);
    }
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
    const Direction* direction = state->getValue(FACING);
    BlockPos adjacent = pos.relative(direction->getOpposite());
    const BlockState* adjacentState = level->getBlockState(adjacent);
    Block* adjacentBlock = adjacentState->getBlock();
    if (adjacentBlock != NULL && dynamic_cast<PistonBaseBlock*>(adjacentBlock) != NULL)
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
    if (boost::dynamic_pointer_cast<PistonMovingBlockEntity>(entity))
    {
        return boost::dynamic_pointer_cast<PistonMovingBlockEntity>(entity);
    }
    return boost::shared_ptr<PistonMovingBlockEntity>();
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
    return ItemInstance::EMPTY;
}

const BlockState* PistonMagicBlock::getBlockState(int data)
{
    const BlockState* state = defaultBlockState();
    TypedBoxed<const Direction*> facing(PistonHeadBlock::getFacing(data));
    state = state->setBoxedValue(FACING, &facing);
    if ((data & 8) == 0)
    {
        TypedBoxed<PistonHeadBlock::PistonType*> normal(pistonTypeNormal);
        return state->setBoxedValue(TYPE, &normal);
    }

    TypedBoxed<PistonHeadBlock::PistonType*> sticky(pistonTypeSticky);
    return state->setBoxedValue(TYPE, &sticky);
}

int PistonMagicBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(TYPE) == pistonTypeSticky)
        data |= 8;
    return data;
}

const BlockState* PistonMagicBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* PistonMagicBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* PistonMagicBlock::createBlockStateDefinition()
{
    static const Property* properties[] = { FACING, TYPE };
    return new BlockStateDefinition(this, properties, 2);
}

EBlockFaceShape PistonMagicBlock::getBlockFaceShape(LevelSource*, const BlockState*,
                                                    const BlockPos&, const Direction*)
{
    return BLOCK_FACE_UNDEFINED;
}

void PistonMagicBlock::registerIcons(IconRegister*)
{
}

EBlockRenderShape PistonMagicBlock::getRenderShape(const BlockState*)
{
    return (EBlockRenderShape)-1;
}
