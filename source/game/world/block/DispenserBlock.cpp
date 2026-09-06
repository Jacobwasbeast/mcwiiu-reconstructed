#include "game/world/block/DispenserBlock.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Position.hpp"
#include "game/core/PositionImpl.hpp"
#include "game/core/Rotation.hpp"
#include "game/item/Container.hpp"
#include "game/item/Containers.hpp"
#include "game/item/DefaultDispenseItemBehavior.hpp"
#include "game/item/DispenseItemBehavior.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockSourceImpl.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/BooleanProperty.hpp"
#include "game/world/block/DispenserBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/client/render/TextureAtlasSprite.hpp"

AbstractProperty<bool>* DispenserBlock::TRIGGERED = NULL;

DispenserBlock::DispenserBlock()
    : BaseEntityBlock(Material::STONE), mRandom()
{
    Block::DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    TypedBoxed<bool> triggered(false);
    const BlockState* state = mStateDefinition->any();
    state = state->setBoxedValue(FACING, &facing);
    registerDefaultState(state->setBoxedValue(TRIGGERED, &triggered));
}

DispenserBlock::~DispenserBlock()
{
}

boost::shared_ptr<BlockEntity> DispenserBlock::newBlockEntity(Level* level, int data)
{
    boost::shared_ptr<DispenserBlockEntity> entity(new DispenserBlockEntity());
    entity->setLevel(level);
    entity->setData(data);
    return boost::static_pointer_cast<BlockEntity>(entity);
}

Position* DispenserBlock::getDispensePosition(BlockSource* blockSource)
{
    const Direction* direction = blockSource->getBlockState()->getValue(FACING);
    BlockPos pos = blockSource->getPos();
    return new PositionImpl(pos.x + 0.7 * direction->getNormal().x,
                            pos.y + 0.7 * direction->getNormal().y,
                            pos.z + 0.7 * direction->getNormal().z);
}

void DispenserBlock::blockStaticCtor()
{
    FACING = DirectionalBlock::FACING;
    TRIGGERED = static_cast<AbstractProperty<bool>*>(BooleanProperty::create(L"triggered"));
}

int DispenserBlock::getTickDelay(Level*)
{
    return 4;
}

void DispenserBlock::recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->mIsClientSide)
    {
        return;
    }
    const Direction* facing = state->getValue(FACING);
    const BlockState* north = level->getBlockState(pos.north());
    const BlockState* south = level->getBlockState(pos.south());
    const BlockState* west = level->getBlockState(pos.west());
    const BlockState* east = level->getBlockState(pos.east());
    const Direction* next = facing;
    if (facing == Direction::NORTH && north->isSolidRender() && !south->isSolidRender()) next = Direction::SOUTH;
    else if (facing == Direction::SOUTH && south->isSolidRender() && !north->isSolidRender()) next = Direction::NORTH;
    else if (facing == Direction::WEST && west->isSolidRender() && !east->isSolidRender()) next = Direction::EAST;
    else if (facing == Direction::EAST && east->isSolidRender() && !west->isSolidRender()) next = Direction::WEST;
    TypedBoxed<const Direction*> boxed(next);
    level->setBlock(pos, state->setBoxedValue(FACING, &boxed), 2, false);
}

void DispenserBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    recalcLockDir(level, pos, state);
}

bool DispenserBlock::TestUse()
{
    return true;
}

bool DispenserBlock::use(Level* level, const BlockPos& pos, const BlockState*,
                         boost::shared_ptr<Player> player, InteractionHand::EInteractionHand,
                         const Direction*, float, float, float, bool clientSide)
{
    if (clientSide)
    {
        return true;
    }
    boost::shared_ptr<BlockEntity> entity = level->getBlockEntity(pos);
    boost::shared_ptr<DispenserBlockEntity> dispenser = boost::dynamic_pointer_cast<DispenserBlockEntity>(entity);
    if (dispenser)
    {
        player->openContainer(boost::static_pointer_cast<Container>(dispenser));
    }
    return true;
}

void DispenserBlock::dispenseFrom(Level* level, const BlockPos& pos)
{
    BlockSourceImpl source(level, pos);
    boost::shared_ptr<DispenserBlockEntity> dispenser =
        boost::dynamic_pointer_cast<DispenserBlockEntity>(source.getEntity());
    if (!dispenser)
    {
        level->levelEvent(1001, pos, 0);
        return;
    }
    int slot = dispenser->getRandomSlot();
    if (slot < 0)
    {
        level->levelEvent(1001, pos, 0);
        return;
    }
    not_null_ptr<ItemInstance> item = dispenser->getItem(static_cast<unsigned int>(slot));
    DispenseItemBehavior* behavior = getDispenseMethod(item);
    if (behavior != NULL)
    {
        not_null_ptr<ItemInstance> result = behavior->dispense(&source, item);
        dispenser->setItem(static_cast<unsigned int>(slot), result);
    }
}

DispenseItemBehavior* DispenserBlock::getDispenseMethod(not_null_ptr<ItemInstance> itemInstance)
{
    return DISPENSER_REGISTRY.get(itemInstance->getItem());
}

void DispenserBlock::neighborChanged(const BlockState* state, Level* level, const BlockPos& pos,
                                     Block*, const BlockPos&)
{
    if (!level->mIsClientSide)
    {
        bool signal = level->hasNeighborSignal(pos);
        bool triggered = state->getValue(TRIGGERED);
        if (signal && !triggered)
        {
            TypedBoxed<bool> value(true);
            level->setBlock(pos, state->setBoxedValue(TRIGGERED, &value), 2, false);
            level->addToTickNextTick(pos, this, getTickDelay(level));
        }
        else if (!signal && triggered)
        {
            TypedBoxed<bool> value(false);
            level->setBlock(pos, state->setBoxedValue(TRIGGERED, &value), 2, false);
        }
    }
}

void DispenserBlock::tick(Level* level, const BlockPos& pos, const BlockState* state, Random*)
{
    if (state->getValue(TRIGGERED))
    {
        dispenseFrom(level, pos);
        TypedBoxed<bool> value(false);
        level->setBlock(pos, state->setBoxedValue(TRIGGERED, &value), 2, false);
    }
}

const BlockState* DispenserBlock::getStateForPlacement(Level*, const BlockPos& pos, const Direction*,
                                                       float, float, float, int,
                                                       boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = Direction::getDirectionFromLivingEntity(pos, placer)->getOpposite();
    TypedBoxed<const Direction*> boxed(facing);
    return defaultBlockState()->setBoxedValue(FACING, &boxed);
}

void DispenserBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                 boost::shared_ptr<LivingEntity> placer,
                                 not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
}

void DispenserBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    boost::shared_ptr<DispenserBlockEntity> dispenser =
        boost::dynamic_pointer_cast<DispenserBlockEntity>(level->getBlockEntity(pos));
    if (dispenser)
    {
        Containers::dropContents(level, pos, boost::static_pointer_cast<Container>(dispenser));
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool DispenserBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int DispenserBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

EBlockRenderShape DispenserBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

const BlockState* DispenserBlock::getBlockState(int data)
{
    TypedBoxed<const Direction*> facing(Direction::from3DDataValue(data & 7));
    TypedBoxed<bool> triggered((data & 8) != 0);
    return defaultBlockState()->setBoxedValue(FACING, &facing)->setBoxedValue(TRIGGERED, &triggered);
}

int DispenserBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    int data = state->getValue(FACING)->get3DDataValue();
    if (state->getValue(TRIGGERED)) data |= 8;
    return data;
}

unsigned int DispenserBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bCanPlace || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside)
    {
        return 0xea4e897eU;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* DispenserBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* DispenserBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* DispenserBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING, TRIGGERED };
    return new BlockStateDefinition(this, properties, 2);
}

TextureAtlasSprite* DispenserBlock::getTexture(const Direction* direction, const BlockState* state)
{
    const Direction* facing = state->getValue(FACING);
    if (direction == facing)
    {
        return facing == Direction::UP || facing == Direction::DOWN ? mFrontVerticalIcon : mFrontIcon;
    }
    if (facing != Direction::UP && facing != Direction::DOWN &&
        direction != Direction::UP && direction != Direction::DOWN)
    {
        return mIcon;
    }
    return mTopIcon;
}

void DispenserBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"furnace_side");
    mTopIcon = iconRegister->registerIcon(L"furnace_top");
    mFrontIcon = iconRegister->registerIcon(L"dispenser_front");
    mFrontVerticalIcon = iconRegister->registerIcon(L"dispenser_front_vertical");
}
