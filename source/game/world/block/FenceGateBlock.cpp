#include "game/world/block/FenceGateBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/world/LevelEvent.hpp"

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
#include "game/world/block/Blocks.hpp"
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
    mIsFenceGate = true;
    mBlockType = EBLOCK_TYPE_FENCE_GATE;
    DerivedInit();
    TypedBoxed<bool> open(false);
    TypedBoxed<bool> powered(false);
    TypedBoxed<bool> inWall(false);
    registerDefaultState(mStateDefinition->any()
                             ->setBoxedValue(OPEN, &open)
                             ->setBoxedValue(POWERED, &powered)
                             ->setBoxedValue(IN_WALL, &inWall));
}

FenceGateBlock::~FenceGateBlock()
{
}

const AABB* FenceGateBlock::getShape(const BlockState* state, LevelSource*, const BlockPos&)
{
    const Direction* facing = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY);
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
    const Direction::Axis* axis = state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis();
    const bool inWall =
        (axis == Direction::Axis::Z
         && (levelSource->getBlockState(pos.west())->getBlock() == Blocks::cobblestone_wall
             || levelSource->getBlockState(pos.east())->getBlock() == Blocks::cobblestone_wall))
        || (axis == Direction::Axis::X
            && (levelSource->getBlockState(pos.north())->getBlock() == Blocks::cobblestone_wall
                || levelSource->getBlockState(pos.south())->getBlock() == Blocks::cobblestone_wall));
    if (!inWall)
        return state;
    TypedBoxed<bool> value(true);
    return state->setBoxedValue(IN_WALL, &value);
}

const BlockState* FenceGateBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
}

const BlockState* FenceGateBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)));
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
    const BlockState* state = defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facingValue);
    state = state->setBoxedValue(POWERED, &poweredValue);
    state = state->setBoxedValue(OPEN, &poweredValue);
    return state->setBoxedValue(IN_WALL, &falseValue);
}

bool FenceGateBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player,
                         InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    bool open = state->getValue(OPEN);
    if (!clientSide)
    {
        const BlockState* newState;
        if (!open)
        {
            Entity* entity = (Entity*)player.get();
            const Direction* facing = Direction::fromYRot((double)entity->mYRot);
            if (state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY) == facing->getOpposite())
            {
                TypedBoxed<const Direction*> facingValue(facing);
                state = state->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facingValue);
            }
            TypedBoxed<bool> value(true);
            newState = state->setBoxedValue(OPEN, &value);
        }
        else
        {
            TypedBoxed<bool> value(false);
            newState = state->setBoxedValue(OPEN, &value);
        }
        level->setBlock(pos, newState, Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        level->levelEvent(player, newState->getValue(OPEN) ? LevelEvent::SOUND_OPEN_FENCE_GATE : LevelEvent::SOUND_CLOSE_FENCE_GATE, pos, 0);
        return true;
    }
    level->levelEvent(player, open ? LevelEvent::SOUND_OPEN_FENCE_GATE : LevelEvent::SOUND_CLOSE_FENCE_GATE, pos, 0);
    return false;
}

void FenceGateBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos, Block*, const BlockPos&)
{
    if (level->mIsClientSide)
        return;
    const bool powered = level->hasNeighborSignal(pos);
    if (state->getValue(POWERED) != powered)
    {
        TypedBoxed<bool> value(powered);
        level->setBlock(pos, state->setBoxedValue(POWERED, &value)->setBoxedValue(OPEN, &value), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
        if (state->getValue(OPEN) != powered)
        {
            level->levelEvent(boost::shared_ptr<Player>(),
                              powered ? LevelEvent::SOUND_OPEN_FENCE_GATE : LevelEvent::SOUND_CLOSE_FENCE_GATE, pos, 0);
        }
    }
}

bool FenceGateBlock::shouldRenderFace(const BlockState*, LevelSource*, const BlockPos&, const Direction*) { return true; }

const BlockState* FenceGateBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from2DDataValue(data);
    TypedBoxed<const Direction*> facing(direction);
    TypedBoxed<bool> open((data & 4) != 0);
    TypedBoxed<bool> powered((data & 8) != 0);
    const BlockState* state = defaultBlockState()->setBoxedValue(HorizontalDirectionalBlock::FACING_PROPERTY, &facing);
    state = state->setBoxedValue(OPEN, &open);
    return state->setBoxedValue(POWERED, &powered);
}

int FenceGateBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->get2DDataValue() | (state->getValue(OPEN) ? 8 : 0) | (state->getValue(POWERED) ? 4 : 0);
}

unsigned int FenceGateBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    return (tooltipData.m_bUseItem || tooltipData.m_bCanPlace) && tooltipData.m_bCanDestroy ? IDS_TOOLTIPS_USE : Block::GetInteractTooltip(tooltipData);
}

bool FenceGateBlock::canContainLiquid(const BlockState*) const { return true; }

BlockStateDefinition* FenceGateBlock::createBlockStateDefinition()
{
    const Property* properties[] = { HorizontalDirectionalBlock::FACING_PROPERTY, POWERED, IN_WALL, OPEN };
    return new BlockStateDefinition(this, properties, 4);
}

EBlockFaceShape FenceGateBlock::getBlockFaceShape(LevelSource*, const BlockState* state, const BlockPos&, const Direction* direction)
{
    if (direction == Direction::UP || direction == Direction::DOWN) return BLOCK_FACE_UNDEFINED;
    return state->getValue(HorizontalDirectionalBlock::FACING_PROPERTY)->getAxis() == direction->getClockWise()->getAxis() ? BLOCK_FACE_MIDDLE_POLE : BLOCK_FACE_UNDEFINED;
}

bool FenceGateBlock::isFenceGate(Block* block)
{
    return block == Blocks::fence_gate || block == Blocks::spruce_fence_gate || block == Blocks::birch_fence_gate
        || block == Blocks::jungle_fence_gate || block == Blocks::acacia_fence_gate
        || block == Blocks::dark_oak_fence_gate;
}

void FenceGateBlock::registerIcons(IconRegister* iconRegister) { mIcon = iconRegister->registerIcon(mTextureName); }
EBlockRenderShape FenceGateBlock::getRenderShape(const BlockState*) { return BLOCK_RENDER_FENCE_GATE; }
