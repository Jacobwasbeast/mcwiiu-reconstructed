#include "game/world/block/EnderChestBlock.hpp"

#include "game/client/render/PlayerEnderChestContainer.hpp"
#include "game/core/Direction.hpp"
#include "game/core/IconRegister.hpp"
#include "game/core/Mirror.hpp"
#include "game/core/Rotation.hpp"
#include "game/client/render/ParticleType.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/BlockTooltipDataHolder.hpp"
#include "game/util/Random.hpp"
#include "game/world/Level.hpp"
#include "game/world/LevelSource.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockEntity.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/BlockStateDefinition.hpp"
#include "game/world/block/ChestBlock.hpp"
#include "game/world/block/EnderChestBlockEntity.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/HorizontalDirectionalBlock.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/Property.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/generation/Player.hpp"

DirectionProperty* EnderChestBlock::FACING = NULL;

EnderChestBlock::EnderChestBlock()
    : BaseEntityBlock(Material::STONE)
{
    DerivedInit();
    TypedBoxed<const Direction*> facing(Direction::NORTH);
    registerDefaultState(defaultBlockState()->setBoxedValue(FACING, &facing));
}

EnderChestBlock::~EnderChestBlock()
{
}

boost::shared_ptr<BlockEntity> EnderChestBlock::newBlockEntity(Level*, int data)
{
    boost::shared_ptr<EnderChestBlockEntity> entity(new EnderChestBlockEntity());
    entity->setData(data);
    return entity;
}

void EnderChestBlock::blockStaticCtor()
{
    FACING = HorizontalDirectionalBlock::FACING_PROPERTY;
}

const AABB* EnderChestBlock::getShape(const BlockState*, LevelSource*, const BlockPos&)
{
    return NULL;
}

bool EnderChestBlock::isSolidRender(const BlockState*) const
{
    return false;
}

bool EnderChestBlock::isCubeShaped(const BlockState*)
{
    return false;
}

bool EnderChestBlock::hasCustomBreakingProgress(const BlockState*)
{
    return true;
}

Item* EnderChestBlock::getResource(const BlockState*, Random*, int)
{
    return Item::byBlock(this);
}

int EnderChestBlock::getResourceCount(Random*)
{
    return 8;
}

bool EnderChestBlock::isSilkTouchable()
{
    return true;
}

const BlockState* EnderChestBlock::getStateForPlacement(Level*, const BlockPos&, const Direction*, float, float,
                                                         float, int, boost::shared_ptr<LivingEntity> placer)
{
    const Direction* facing = placer->getDirection()->getOpposite();
    TypedBoxed<const Direction*> boxedFacing(facing);
    return defaultBlockState()->setBoxedValue(FACING, &boxedFacing);
}

void EnderChestBlock::setPlacedBy(Level* level, const BlockPos& pos, const BlockState* state,
                                  boost::shared_ptr<LivingEntity> placer,
                                  not_null_ptr<ItemInstance> itemInstance)
{
    const Direction* facing = placer->getDirection()->getOpposite();
    TypedBoxed<const Direction*> boxedFacing(facing);
    level->setBlock(pos, state->setBoxedValue(FACING, &boxedFacing), 2, false);

    if (itemInstance->hasCustomHoverName())
    {
        boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
        boost::shared_ptr<EnderChestBlockEntity> enderChest =
            boost::dynamic_pointer_cast<EnderChestBlockEntity>(blockEntity);
        if (enderChest)
        {
            enderChest->setCustomName(itemInstance->getHoverNameUnfiltered(), itemInstance->getOwnerUUID(),
                                      itemInstance->wasCreatedByRestrictedPlayer(), itemInstance->wasCreatedOnHost());
        }
    }
}

bool EnderChestBlock::use(Level* level, const BlockPos& pos, const BlockState*, boost::shared_ptr<Player> player,
                          InteractionHand::EInteractionHand, const Direction*, float, float, float, bool clientSide)
{
    boost::shared_ptr<PlayerEnderChestContainer> inventory = player->getEnderChestInventory();
    boost::shared_ptr<BlockEntity> blockEntity = level->getBlockEntity(pos);
    if (inventory)
    {
        if (blockEntity->GetType() == 0x1200208ULL && !ChestBlock::isChestBlockedAt(level, pos) && !clientSide)
        {
            boost::shared_ptr<EnderChestBlockEntity> chest =
                boost::dynamic_pointer_cast<EnderChestBlockEntity>(blockEntity);
            inventory->setActiveChest(chest);
            player->openContainer(inventory);
        }
    }
    return true;
}

void EnderChestBlock::animateTick(const BlockState*, Level* level, const BlockPos& pos, Random* random)
{
    for (int i = 0; i < 3; ++i)
    {
        double xSign = (double)(random->nextInt(2) * 2 - 1);
        double zSign = (double)(random->nextInt(2) * 2 - 1);
        double x = (double)pos.x + 0.5 + xSign * 0.25;
        double y = (double)pos.y + (double)random->nextFloat();
        double z = (double)pos.z + 0.5 + zSign * 0.25;
        double xa = (double)random->nextFloat() * xSign;
        double ya = ((double)random->nextFloat() - 0.5) * 0.125;
        double za = (double)random->nextFloat() * zSign;
        level->addParticle(PARTICLE_PORTAL, x, y, z, xa, ya, za, arrayWithLength<int>());
    }
}

const BlockState* EnderChestBlock::getBlockState(int data)
{
    const Direction* facing = Direction::from3DDataValue(data);
    if (facing->getAxis() == Direction::Axis::Y)
    {
        facing = Direction::NORTH;
    }
    TypedBoxed<const Direction*> boxedFacing(facing);
    return defaultBlockState()->setBoxedValue(FACING, &boxedFacing);
}

int EnderChestBlock::convertBlockStateToLegacyData(const BlockState* state)
{
    return state->getValue(FACING)->get3DDataValue();
}

unsigned int EnderChestBlock::GetInteractTooltip(const BlockTooltipDataHolder& tooltipData)
{
    if ((tooltipData.m_bUseItem || tooltipData.m_bCanDestroy) && tooltipData.m_bIsInside)
    {
        return 0x41590ad1;
    }
    return Block::GetInteractTooltip(tooltipData);
}

const BlockState* EnderChestBlock::rotate(const BlockState* state, Rotation* rotation)
{
    TypedBoxed<const Direction*> facing(rotation->rotate(state->getValue(FACING)));
    return state->setBoxedValue(FACING, &facing);
}

const BlockState* EnderChestBlock::mirror(const BlockState* state, Mirror* mirror)
{
    return rotate(state, mirror->getRotation(state->getValue(FACING)));
}

BlockStateDefinition* EnderChestBlock::createBlockStateDefinition()
{
    const Property* properties[] = { FACING };
    return new BlockStateDefinition(this, properties, 1);
}

EBlockFaceShape EnderChestBlock::getBlockFaceShape(LevelSource*, const BlockState*, const BlockPos&, const Direction*)
{
    return static_cast<EBlockFaceShape>(8);
}

void EnderChestBlock::registerIcons(IconRegister* iconRegister)
{
    mIcon = iconRegister->registerIcon(L"ender_chest_top");
}

EBlockRenderShape EnderChestBlock::getRenderShape(const BlockState*)
{
    return static_cast<EBlockRenderShape>(0x16);
}
