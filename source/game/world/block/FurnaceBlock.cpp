#include "game/world/block/FurnaceBlock.hpp"

#include "game/core/Boxed.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/world/generation/Player.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/item/Containers.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/DirectionalBlock.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"

FurnaceBlock::FurnaceBlock(bool lit)
    : BaseEntityBlock(NULL), mLit(lit), mTopIcon(NULL), mFrontIcon(NULL)
{
    mBlockEntity = true;
    DerivedInit();
    registerDefaultState(mStateDefinition->any());
}

FurnaceBlock::~FurnaceBlock()
{
}

boost::shared_ptr<BlockEntity> FurnaceBlock::newBlockEntity(Level*, int)
{
    return boost::shared_ptr<BlockEntity>(new FurnaceBlockEntity());
}

void FurnaceBlock::blockStaticCtor()
{
    DirectionalBlock::blockStaticCtor();
}

Item* FurnaceBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(Blocks::furnace);
}

void FurnaceBlock::recalcLockDir(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (level->mIsClientSide) return;
    const BlockState* north = level->getBlockState(pos.north());
    const BlockState* south = level->getBlockState(pos.south());
    const BlockState* west = level->getBlockState(pos.west());
    const BlockState* east = level->getBlockState(pos.east());
    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    if (facing == Direction::NORTH && north->getBlock()->isSolidBlockingCube(north) && !south->getBlock()->isSolidBlockingCube(south)) facing = Direction::SOUTH;
    else if (facing == Direction::SOUTH && south->getBlock()->isSolidBlockingCube(south) && !north->getBlock()->isSolidBlockingCube(north)) facing = Direction::NORTH;
    else if (facing == Direction::WEST && west->getBlock()->isSolidBlockingCube(west) && !east->getBlock()->isSolidBlockingCube(east)) facing = Direction::EAST;
    else if (facing == Direction::EAST && east->getBlock()->isSolidBlockingCube(east) && !west->getBlock()->isSolidBlockingCube(west)) facing = Direction::WEST;
    TypedBoxed<const Direction*> boxed(facing);
    level->setBlock(pos, state->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &boxed), 2, false);
}

void FurnaceBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    recalcLockDir(level, pos, state);
}

void FurnaceBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!mLit) return;
    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    double x = pos.x + 0.5;
    double y = pos.y + 0.5;
    double z = pos.z + 0.5;
    double offset = 0.52;
    double side = random->nextDouble() * 0.6 - 0.3;
    if (random->nextDouble() < 0.1) level->levelEvent(1010, pos, 0);
    if (facing == Direction::NORTH) { z -= offset; x += side; }
    else if (facing == Direction::SOUTH) { z += offset; x += side; }
    else if (facing == Direction::WEST) { x -= offset; z += side; }
    else if (facing == Direction::EAST) { x += offset; z += side; }
    level->addParticle(ePARTICLE_SMOKE, false, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
    level->addParticle(ePARTICLE_FLAME, false, x, y, z, 0.0, 0.0, 0.0, arrayWithLength<int>());
}

bool FurnaceBlock::TestUse()
{
    return true;
}

bool FurnaceBlock::use(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    if (clientSide) return true;
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (blockEntity != NULL)
    {
        boost::shared_ptr<FurnaceBlockEntity> furnace = boost::dynamic_pointer_cast<FurnaceBlockEntity>(blockEntity);
        if (furnace != NULL) player->openContainer(boost::static_pointer_cast<Container>(furnace));
    }
    return true;
}

void FurnaceBlock::setLit(bool lit, Level* level, const BlockPos& pos)
{
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    const BlockState* state = level->getBlockState(pos);
    Block* block = lit ? Blocks::lit_furnace : Blocks::furnace;
    TypedBoxed<const Direction*> facing(state->getValue(DirectionalBlock::FACING_PROPERTY));
    const BlockState* replacement = block->defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &facing);
    level->setBlock(pos, replacement, 3, false);
    if (blockEntity != NULL) level->setBlockEntity(pos, blockEntity);
}

void FurnaceBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    const Direction* facing = state->getValue(DirectionalBlock::FACING_PROPERTY);
    TypedBoxed<const Direction*> opposite(facing->getOpposite());
    level->setBlock(pos, state->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &opposite), 2, false);
    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<FurnaceBlockEntity> furnace = boost::dynamic_pointer_cast<FurnaceBlockEntity>(level->getBlockEntity(pos));
        if (furnace != NULL) furnace->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(), itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
    }
}

void FurnaceBlock::onRemove(Level* level, const BlockPos& pos, const BlockState* state)
{
    if (!mLit)
    {
        boost::shared_ptr<FurnaceBlockEntity> furnace = boost::dynamic_pointer_cast<FurnaceBlockEntity>(level->getBlockEntity(pos));
        if (furnace != NULL) Containers::dropContents(level, pos, furnace);
        level->removeBlockEntity(pos);
    }
    BaseEntityBlock::onRemove(level, pos, state);
}

bool FurnaceBlock::hasAnalogOutputSignal(const BlockState*)
{
    return true;
}

int FurnaceBlock::getAnalogOutputSignal(const BlockState*, Level* level, const BlockPos& pos)
{
    return AbstractContainerMenu::getRedstoneSignalFromBlockEntity(level->getBlockEntity(pos));
}

not_null_ptr<ItemInstance> FurnaceBlock::getCloneItemInstance(Level*, const BlockPos&, const BlockState*)
{
    return not_null_ptr<ItemInstance>(new ItemInstance(Item::byBlock(Blocks::furnace)));
}

EBlockRenderShape FurnaceBlock::getRenderShape(const BlockState*)
{
    return BLOCK_RENDER_MODEL;
}

const BlockState* FurnaceBlock::getInventoryRenderState(const BlockState*)
{
    return Blocks::furnace->defaultBlockState();
}

const BlockState* FurnaceBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from3DDataValue(data);
    if (direction->getAxis() == Direction::Axis::Y) direction = Direction::NORTH;
    TypedBoxed<const Direction*> boxed(direction);
    return defaultBlockState()->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &boxed);
}

int FurnaceBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(DirectionalBlock::FACING_PROPERTY)->get3DDataValue();
}

unsigned int FurnaceBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bCanPlace || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside) return 0xea4e897e;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* FurnaceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> boxed(rotation->rotate(state->getValue(DirectionalBlock::FACING_PROPERTY)));
    return state->setBoxedValue(DirectionalBlock::FACING_PROPERTY, &boxed);
}

const BlockState* FurnaceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(DirectionalBlock::FACING_PROPERTY)));
}

BlockStateDefinition* FurnaceBlock::createBlockStateDefinition()
{
    const Property* properties[] = { DirectionalBlock::FACING_PROPERTY };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* FurnaceBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN) return mTopIcon;
    return direction == state->getValue(DirectionalBlock::FACING_PROPERTY) ? mFrontIcon : mIcon;
}

void FurnaceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"furnace_side");
    mFrontIcon = iconRegister->registerIcon(mLit ? L"furnace_front_on" : L"furnace_front");
    mTopIcon = iconRegister->registerIcon(L"furnace_top");
}
