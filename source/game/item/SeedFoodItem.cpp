#include "game/item/SeedFoodItem.hpp"

#include "game/entity/mob/LivingEntity.hpp"
#include "game/world/generation/Player.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/core/Direction.hpp"

SeedFoodItem::SeedFoodItem(int nutrition, float saturationModifier, Block* cropBlock, Block* soilBlock)
    : FoodItem(nutrition, saturationModifier, false), mCropBlock(cropBlock), mSoilBlock(soilBlock)
{
}

SeedFoodItem::~SeedFoodItem()
{
}

InteractionResult::EInteractionResult SeedFoodItem::useOn(boost::shared_ptr<Player> player,
                                                           Level* level,
                                                           const BlockPos& pos,
                                                           InteractionHand::EInteractionHand hand,
                                                           const Direction* direction,
                                                           float hitX,
                                                           float hitY,
                                                           float hitZ,
                                                           bool simulate)
{
    LivingEntity* livingPlayer = (LivingEntity*)(void*)player.get();
    not_null_ptr<ItemInstance> itemInstance = livingPlayer->getItemInHand(hand);
    if (direction == Direction::UP
        && player->mayUseItemAt(pos.relative(direction), direction, itemInstance)
        && level->getBlockState(pos)->getBlock() == mSoilBlock
        && level->isEmptyBlock(pos.above()))
    {
        if (!simulate)
        {
            level->setBlock(pos.above(), mCropBlock->defaultBlockState(), 11, false);
            itemInstance->shrink(1);
        }
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Pass;
}
