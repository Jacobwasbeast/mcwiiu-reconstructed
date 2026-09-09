#include "game/item/MinecartItem.hpp"

#include "game/core/Direction.hpp"
#include "game/core/BehaviorRegistry.hpp"
#include "game/entity/mob/LivingEntity.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BaseRailBlock.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockSource.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Material.hpp"
#include "game/world/block/TypedBoxed.hpp"
#include "game/world/block/DirectionProperty.hpp"
#include "game/world/block/DispenserBlock.hpp"

MinecartItem::MinecartItem(const Minecart::Type* type)
    : mMinecartType(type)
{
    mMaxStackSize = 1;
    DispenserBlock::DISPENSER_REGISTRY.add(this, new MinecartDispenseBehavior());
}

MinecartItem::~MinecartItem()
{
}

InteractionResult::EInteractionResult MinecartItem::useOn(boost::shared_ptr<Player> player,
                                                           Level* level,
                                                           const BlockPos& pos,
                                                           InteractionHand::EInteractionHand hand,
                                                           const Direction* direction,
                                                           float hitX,
                                                           float hitY,
                                                           float hitZ,
                                                           bool simulate)
{
    const BlockState* state = level->getBlockState(pos);
    if (!BaseRailBlock::isRail(state))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    not_null_ptr<ItemInstance> itemInstance = ((LivingEntity*)player.get())->getItemInHand(hand);
    if (!simulate && !level->mIsClientSide)
    {
        if (level->countInstanceOf(0xA800000ULL, false, NULL, NULL) > 249)
        {
            return InteractionResult::eInteractionResult_Fail;
        }

        Block* block = state->getBlock();
        BaseRailBlock::RailShape* shape = NULL;
        if (block->instanceof(EBLOCK_TYPE_RAIL))
        {
            shape = static_cast<TypedBoxed<BaseRailBlock::RailShape*>*>(
                state->getBoxedValue(static_cast<BaseRailBlock*>(block)->getShapeProperty()))->getValue();
        }

        double yOffset = shape != NULL && shape->isAscending() ? 1.0 : 0.0;
        boost::shared_ptr<Minecart> minecart = Minecart::createMinecart(
            level, (double)pos.x + 0.5, (double)pos.y + 0.0625 + yOffset,
            (double)pos.z + 0.5, mMinecartType);

        if (itemInstance->hasCustomHoverName())
        {
            minecart->setCustomName(itemInstance->getHoverName(),
                                    itemInstance->getOwnerUUID(),
                                    itemInstance->wasCreatedByRestrictedPlayer());
        }

        level->addEntity(minecart);
        itemInstance->shrink(1);
    }

    return InteractionResult::eInteractionResult_Success;
}

MinecartItem::MinecartDispenseBehavior::~MinecartDispenseBehavior()
{
}

not_null_ptr<ItemInstance> MinecartItem::MinecartDispenseBehavior::execute(
    BlockSource* blockSource, not_null_ptr<ItemInstance> itemInstance, eOUTCOME& outcome)
{
    const Direction* direction = blockSource->getBlockState()->getValue(DispenserBlock::FACING);
    BlockPos position = blockSource->getPos().relative(direction);
    Level* level = blockSource->getWorld();
    const BlockState* state = level->getBlockState(position);

    if (level->countInstanceOf(0xA800000ULL, false, NULL, NULL) > 249)
    {
        outcome = eOUTCOME_Failure;
        return DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome);
    }

    if (!BaseRailBlock::isRail(state))
    {
        const BlockPos below = position.below();
        const BlockState* belowState = level->getBlockState(below);
        if (state->getBlock()->getCraftingMaterial() != Material::AIR
            || !BaseRailBlock::isRail(belowState))
        {
            outcome = eOUTCOME_Failure;
            return DefaultDispenseItemBehavior::execute(blockSource, itemInstance, outcome);
        }
        position = below;
        state = belowState;
    }

    BaseRailBlock::RailShape* shape = NULL;
    Block* block = state->getBlock();
    if (block->instanceof(EBLOCK_TYPE_RAIL))
    {
        shape = static_cast<TypedBoxed<BaseRailBlock::RailShape*>*>(
            state->getBoxedValue(static_cast<BaseRailBlock*>(block)->getShapeProperty()))->getValue();
    }

    double y = (double)position.y + 0.0625;
    if (shape != NULL && shape->isAscending())
    {
        y += 1.0;
    }

    MinecartItem* minecartItem = static_cast<MinecartItem*>(itemInstance->getItem());
    boost::shared_ptr<Minecart> minecart = Minecart::createMinecart(
        level, (double)position.x + 0.5, y, (double)position.z + 0.5,
        minecartItem->mMinecartType);

    if (itemInstance->hasCustomHoverName())
    {
        minecart->setCustomName(itemInstance->getHoverName(),
                                itemInstance->getOwnerUUID(),
                                itemInstance->wasCreatedByRestrictedPlayer());
    }

    level->addEntity(minecart);
    itemInstance->shrink(1);
    outcome = eOUTCOME_Success;
    return itemInstance;
}

void MinecartItem::MinecartDispenseBehavior::playSound(BlockSource* blockSource, eOUTCOME outcome)
{
    blockSource->getWorld()->levelEvent(1000, blockSource->getPos(), 0);
}
