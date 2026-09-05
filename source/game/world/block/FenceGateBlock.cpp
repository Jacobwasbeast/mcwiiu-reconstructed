#include "game/world/block/FenceGateBlock.hpp"

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/entity/mob/Entity.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/AABB.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/PlanksBlock.hpp"
#include "game/world/block/TypedBoxed.hpp"

AbstractProperty<bool>* FenceGateBlock::OPEN = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"open"));
AbstractProperty<bool>* FenceGateBlock::POWERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"powered"));
AbstractProperty<bool>* FenceGateBlock::IN_WALL = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"in_wall"));

FenceGateBlock::FenceGateBlock(const std::wstring& name, PlanksBlock::Variant* variant)
    : HorizontalDirectionalBlock(Material::WOOD, variant->getMaterialColor()), mTextureName(name)
{
    mBlockType = EBLOCK_TYPE_FENCE;
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> falseValue(false);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
    state = state->setBoxedValue(POWERED, &falseValue);
    state = state->setBoxedValue(IN_WALL, &falseValue);
    registerDefaultState(state->setBoxedValue(OPEN, &falseValue));
}

FenceGateBlock::~FenceGateBlock()
{
}

const AABB* FenceGateBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING);
    bool open = state->getValue(OPEN);
    if (open)
    {
        return facing->getAxis() == Direction::Axis::X ? AABB::newTemp(0.0, 0.0, 0.375, 1.0, 1.0, 0.625)
                                                       : AABB::newTemp(0.375, 0.0, 0.0, 0.625, 1.0, 1.0);
    }
    return facing->getAxis() == Direction::Axis::X ? AABB::newTemp(0.375, 0.0, 0.0, 0.625, 1.0, 1.0)
                                                   : AABB::newTemp(0.0, 0.0, 0.375, 1.0, 1.0, 0.625);
}

const BlockState* FenceGateBlock::fillVirtualBlockStateProperties(const BlockState* state, LevelSource* levelSource,
                                                                   const BlockPos& pos)
{
    const Direction::Axis* axis = state->getValue(HorizontalDirectionalBlock::FACING)->getAxis();
    bool inWall = false;
    if (axis == Direction::Axis::X)
        inWall = levelSource->getBlock(pos.west())->getRenderLayer() == 5 || levelSource->getBlock(pos.east())->getRenderLayer() == 5;
    else if (axis == Direction::Axis::Z)
        inWall = levelSource->getBlock(pos.north())->getRenderLayer() == 5 || levelSource->getBlock(pos.south())->getRenderLayer() == 5;
    TypedBoxed<bool> value(inWall);
    return state->setBoxedValue(IN_WALL, &value);
}

const BlockState* FenceGateBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
}

const BlockState* FenceGateBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING)));
}

bool FenceGateBlock::mayPlace(Level* level, const BlockPos& pos)
{
    return level->getBlockState(pos.below())->isSolid() && Block::mayPlace(level, pos);
}

const AABB* FenceGateBlock::getClipAABB(const BlockState* state, LevelSource*, const BlockPos&)
{
    if (state->getValue(OPEN)) return NULL;
    return getShape(state, NULL, BlockPos());
}

bool FenceGateBlock::isSolidRender(const BlockState*) const { return false; }
bool FenceGateBlock::isCubeShaped(const BlockState*) { return false; }
bool FenceGateBlock::isPathfindable(LevelSource* levelSource, const BlockPos& pos)
{
    return levelSource->getBlockState(pos)->getValue(OPEN);
}

const BlockState* FenceGateBlock::getStateForPlacement(Level* level, const BlockPos& pos, const Direction*, float, float,
                                                       float, int, boost::shared_ptr<LivingEntity> placer)
{
    Entity* entity = (Entity*)placer.get();
    const Direction* facing = Direction::fromYRot((double)entity->mYRot);
    bool powered = level->hasNeighborSignal(pos);
    TypedBoxed<const Direction*> facingValue(facing);
    TypedBoxed<bool> poweredValue(powered);
    TypedBoxed<bool> falseValue(false);
    const BlockState* state = defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facingValue);
    state = state->setBoxedValue(POWERED, &poweredValue);
    state = state->setBoxedValue(OPEN, &poweredValue);
    return state->setBoxedValue(IN_WALL, &falseValue);
}

bool FenceGateBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    bool open = state->getValue(OPEN);
    TypedBoxed<bool> value(!open);
    if (!clientSide)
    {
        level->setBlock(pos, state->setBoxedValue(OPEN, &value)->setBoxedValue(POWERED, &value), 11, false);
    }
    level->levelEvent(player, open ? 1011 : 1009, pos, 0);
    return true;
}

void FenceGateBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    bool powered = level->hasNeighborSignal(pos);
    if (powered != state->getValue(POWERED))
    {
        TypedBoxed<bool> value(powered);
        level->setBlock(pos, state->setBoxedValue(POWERED, &value)->setBoxedValue(OPEN, &value), 11, false);
        level->levelEvent(powered ? 1008 : 1010, pos, 0);
    }
}

bool FenceGateBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*) { return true; }

const BlockState* FenceGateBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from2DDataValue(data);
    TypedBoxed<const Direction*> facing(direction);
    TypedBoxed<bool> open((data & 4) != 0);
    TypedBoxed<bool> powered((data & 8) != 0);
    TypedBoxed<bool> inWall(false);
    const BlockState* state = defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING, &facing);
    state = state->setBoxedValue(OPEN, &open);
    state = state->setBoxedValue(POWERED, &powered);
    return state->setBoxedValue(IN_WALL, &inWall);
}

int FenceGateBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING)->get2DDataValue() | (state->getValue(OPEN) ? 8 : 0) | (state->getValue(POWERED) ? 4 : 0);
}

unsigned int FenceGateBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return (tooltipData.m_bUseItem || tooltipData.m_bCanPlace) && tooltipData.m_bCanDestroy ? 0xea4e897eu : Block::GetInteractTooltip(tooltipData);
}

bool FenceGateBlock::canContainLiquid(const BlockState*) const { return true; }

BlockStateDefinition* FenceGateBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING, POWERED, IN_WALL, OPEN };
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape FenceGateBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&, const Direction* direction)
{
    if (direction == Direction::UP || direction == Direction::DOWN) return BLOCK_FACE_UNDEFINED;
    return state->getValue(HorizontalDirectionalBlock::FACING)->getAxis() == direction->getClockWise()->getAxis() ? BLOCK_FACE_MIDDLE_POLE_THICK : BLOCK_FACE_UNDEFINED;
}

bool FenceGateBlock::isFenceGate(Block* block)
{
    return block != NULL && block->mBlockType == EBLOCK_TYPE_FENCE;
}

void FenceGateBlock::registerIcons(IconRegister* iconRegister) { mIcon = iconRegister->registerIcon(mTextureName); }
EBlockRenderShape FenceGateBlock::getRenderShape(const BlockState*) { return static_cast<EBlockRenderShape>(21); }
