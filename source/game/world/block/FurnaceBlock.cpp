#include "game/world/block/FurnaceBlock.hpp"
#include "game/core/StringIds.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/client/render/ParticleType.hpp"

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
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"

DirectionProperty* FurnaceBlock::FACING = NULL;

bool FurnaceBlock::sKeepInventory = false;

FurnaceBlock::FurnaceBlock(bool lit)
    : BaseEntityBlock(Material::STONE), mLit(lit), mTopIcon(NULL), mFrontIcon(NULL)
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
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
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
    const Direction* facing = state->getValue(FACING);
    if (facing == Direction::NORTH && north->getBlock()->isSolidBlockingCube(north) && !south->getBlock()->isSolidBlockingCube(south)) facing = Direction::SOUTH;
    else if (facing == Direction::SOUTH && south->getBlock()->isSolidBlockingCube(south) && !north->getBlock()->isSolidBlockingCube(north)) facing = Direction::NORTH;
    else if (facing == Direction::WEST && west->getBlock()->isSolidBlockingCube(west) && !east->getBlock()->isSolidBlockingCube(east)) facing = Direction::EAST;
    else if (facing == Direction::EAST && east->getBlock()->isSolidBlockingCube(east) && !west->getBlock()->isSolidBlockingCube(west)) facing = Direction::WEST;
    TypedBoxed<const Direction*> boxed(facing);
    level->setBlock(pos, state->setBoxedValue(FACING, &boxed), Block::UPDATE_CLIENTS, false);
}

void FurnaceBlock::onPlace(Level* level, const BlockPos& pos, const BlockState* state)
{
    Block::onPlace(level, pos, state);
    recalcLockDir(level, pos, state);
}

void FurnaceBlock::animateTick(const BlockState* state, Level* level, const BlockPos& pos, Random* random)
{
    if (!mLit)
        return;
    const Direction* facing = state->getValue(FACING);
    const double x = pos.x + 0.5;
    const double y = pos.y + random->nextDouble() * 6.0 / 16.0;
    const double z = pos.z + 0.5;
    const double side = random->nextDouble() * 0.6 - 0.3;
    if (random->nextDouble() < 0.1)
    {
        level->playLocalSound(pos.x + 0.5, pos.y, pos.z + 0.5, SoundEvents::FURNACE_FIRE_CRACKLE,
                              SoundSource::eSoundSource_Blocks, 1.0f, 1.0f, false, 16.0f);
    }
    switch (facing->get3DDataValue())
    {
    case Direction::eDirection_North:
        level->addParticle(PARTICLE_SMOKE, x + side, y, z - 0.52, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + side, y, z - 0.52, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    case Direction::eDirection_South:
        level->addParticle(PARTICLE_SMOKE, x + side, y, z + 0.52, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + side, y, z + 0.52, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    case Direction::eDirection_West:
        level->addParticle(PARTICLE_SMOKE, x - 0.52, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x - 0.52, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    case Direction::eDirection_East:
        level->addParticle(PARTICLE_SMOKE, x + 0.52, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        level->addParticle(PARTICLE_FLAME, x + 0.52, y, z + side, 0.0, 0.0, 0.0, arrayWithLength<int>());
        break;
    default:
        break;
    }
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
    const BlockState* state = level->getBlockState(pos);
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    Block* block = lit ? Blocks::lit_furnace : Blocks::furnace;
    sKeepInventory = true;
    TypedBoxed<const Direction*> facing1(state->getValue(FACING));
    level->setBlock(pos, block->defaultBlockState()->setBoxedValue(FACING, &facing1), Block::UPDATE_ALL, false);
    TypedBoxed<const Direction*> facing2(state->getValue(FACING));
    level->setBlock(pos, block->defaultBlockState()->setBoxedValue(FACING, &facing2), Block::UPDATE_ALL, false);
    sKeepInventory = false;
    if (blockEntity != NULL)
    {
        sKeepInventory = false;
        blockEntity->clearRemoved();
        level->setBlockEntity(pos, blockEntity);
    }
}

void FurnaceBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state, boost::shared_ptr<LivingEntity> placer, not_null_ptr<ItemInstance> itemInstance)
{
    Block::setPlacedBy(level, pos, state, placer, itemInstance);
    const Direction* facing = state->getValue(FACING);
    TypedBoxed<const Direction*> opposite(facing->getOpposite());
    level->setBlock(pos, state->setBoxedValue(FACING, &opposite), Block::UPDATE_CLIENTS, false);
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
    TypedBoxed<const Direction*> facing(Direction::SOUTH);
    return defaultBlockState()->setBoxedValue(FACING, &facing);
}

const BlockState* FurnaceBlock::getBlockState(int data)
{
    const Direction* direction = Direction::from3DDataValue(data);
    if (direction->getAxis() == Direction::Axis::Y) direction = Direction::NORTH;
    TypedBoxed<const Direction*> boxed(direction);
    return defaultBlockState()->setBoxedValue(FACING, &boxed);
}

int FurnaceBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

unsigned int FurnaceBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bCanPlace || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside) return IDS_TOOLTIPS_USE;
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* FurnaceBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> boxed(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &boxed);
}

const BlockState* FurnaceBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* FurnaceBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING };
    return new BlockStateDefinition(this, properties, 1);
}

TextureAtlasSprite* FurnaceBlock::getTexture(const Direction* direction, const BlockState* state)
{
    if (direction == Direction::UP || direction == Direction::DOWN) return mTopIcon;
    return direction == state->getValue(FACING) ? mFrontIcon : mIcon;
}

void FurnaceBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"furnace_side");
    mFrontIcon = iconRegister->registerIcon(mLit ? L"furnace_front_lit" : L"furnace_front");
    mTopIcon = iconRegister->registerIcon(L"furnace_top");
}
