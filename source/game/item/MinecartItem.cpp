#include "game/item/MinecartItem.hpp"
#include "game/world/LevelEvent.hpp"
#include "game/core/EntityType.hpp"

#include "game/core/Direction.hpp"
#include "game/core/StringIds.hpp"
#include "game/core/Mth.hpp"
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
#include "game/world/generation/Player.hpp"

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

    if (!simulate)
    {
        not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
        if ((unsigned int)level->countInstanceOf(EntityType::MINECART, false, NULL, NULL) >= 250)
        {
            if (!level->mIsClientSide)
            {
                player->displayClientMessage(IDS_MAX_MINECARTS, true);
            }
            return InteractionResult::eInteractionResult_Fail;
        }

        if (!level->mIsClientSide)
        {
            Block* block = state->getBlock();
            BaseRailBlock::RailShape* shape = BaseRailBlock::RailShape::NORTH_SOUTH;
            if (block->instanceof(EBLOCK_TYPE_RAIL))
            {
                shape = state->getValue(static_cast<BaseRailBlock*>(block)->getShapeProperty());
            }

            double yOffset = shape->isAscending() ? 0.5 : 0.0;
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
        }

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
    Level* level = blockSource->getWorld();
    double x = blockSource->getX() + (float)direction->getStepX() * 1.125f;
    double y = Mth::floor(blockSource->getY() + (float)direction->getStepY() * 1.125f);
    double z = blockSource->getZ() + (float)direction->getStepZ() * 1.125f;
    BlockPos position = blockSource->getPos().relative(direction);
    const BlockState* state = level->getBlockState(position);

    BaseRailBlock::RailShape* shape = BaseRailBlock::RailShape::NORTH_SOUTH;
    if (state->getBlock()->instanceof(EBLOCK_TYPE_RAIL))
    {
        shape = state->getValue(static_cast<BaseRailBlock*>(state->getBlock())->getShapeProperty());
    }

    if (level->countInstanceOf(EntityType::MINECART, false, NULL, NULL) >= 250)
    {
        outcome = eOUTCOME_Default;
        return mDefaultDispenseItemBehavior.dispense(blockSource, itemInstance);
    }

    double railOffset;
    if (BaseRailBlock::isRail(state))
    {
        railOffset = shape->isAscending() ? 0.6 : 0.1;
    }
    else
    {
        if (state->getMaterial() != Material::AIR || !BaseRailBlock::isRail(level->getBlockState(position.below())))
        {
            outcome = eOUTCOME_Default;
            return mDefaultDispenseItemBehavior.dispense(blockSource, itemInstance);
        }

        const BlockState* belowState = level->getBlockState(position.below());
        BaseRailBlock::RailShape* belowShape = BaseRailBlock::RailShape::NORTH_SOUTH;
        if (belowState->getBlock()->instanceof(EBLOCK_TYPE_RAIL))
        {
            belowShape = belowState->getValue(static_cast<BaseRailBlock*>(belowState->getBlock())->getShapeProperty());
        }
        railOffset = direction != Direction::DOWN && belowShape->isAscending() ? -0.4 : -0.9;
    }

    outcome = eOUTCOME_Success;
    MinecartItem* minecartItem = static_cast<MinecartItem*>(itemInstance->getItem());
    boost::shared_ptr<Minecart> minecart =
        Minecart::createMinecart(level, x, y + railOffset, z, minecartItem->mMinecartType);

    if (itemInstance->hasCustomHoverName())
    {
        minecart->setCustomName(itemInstance->getHoverName(),
                                itemInstance->getOwnerUUID(),
                                itemInstance->wasCreatedByRestrictedPlayer());
    }

    level->addEntity(minecart);
    itemInstance->shrink(1);
    return itemInstance;
}

void MinecartItem::MinecartDispenseBehavior::playSound(BlockSource* blockSource, eOUTCOME outcome)
{
    blockSource->getWorld()->levelEvent(LevelEvent::SOUND_DISPENSER_DISPENSE, blockSource->getPos(), 0);
}
