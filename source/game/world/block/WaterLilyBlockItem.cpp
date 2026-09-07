#include "game/world/block/WaterLilyBlockItem.hpp"

#include "game/core/HitResult.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

WaterLilyBlockItem::WaterLilyBlockItem(Block* block)
    : TintedBlockItem(block, false)
{
}

WaterLilyBlockItem::~WaterLilyBlockItem()
{
}

bool WaterLilyBlockItem::TestUse(Level* level, boost::shared_ptr<Player> player,
                                 InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
    if (hitResult == NULL)
    {
        return false;
    }

    if (hitResult->mType != HitResult::BLOCK)
    {
        delete hitResult;
        return false;
    }

    BlockPos placePos = hitResult->getBlockPos().above();
    if (player->mayUseItemAt(hitResult->getBlockPos(), hitResult->mDirection, itemInstance) != 1)
    {
        delete hitResult;
        return false;
    }

    const BlockState* waterState = level->getBlockState(hitResult->getBlockPos());
    bool canPlace = waterState != NULL && waterState->getBlock() == Blocks::WATER
        && level->isEmptyBlock(placePos)
        && getBlock()->mayPlace(level, placePos, hitResult->mDirection);
    delete hitResult;
    return canPlace;
}

InteractionResultHolder WaterLilyBlockItem::use(Level* level, boost::shared_ptr<Player> player,
                                                InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!TestUse(level, player, hand))
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    if (!level->mIsClientSide)
    {
        HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
        if (hitResult != NULL && hitResult->mType == HitResult::BLOCK)
        {
            BlockPos placePos = hitResult->getBlockPos().above();
            if (level->setBlock(placePos, getBlock()->defaultBlockState(), 11, false)
                && !player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
        }
        delete hitResult;
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int WaterLilyBlockItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return TestUse((Level*)tooltipData.m_pLevel,
                   (boost::shared_ptr<Player>&)tooltipData.m_pPlayer,
                   InteractionHand::eInteractionHand_MainHand)
        ? 0x4c36cd96U : 0xffffffffU;
}
