#include "game/world/block/WaterLilyBlockItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/HitResult.hpp"
#include "game/client/audio/SoundEvents.hpp"
#include "game/client/audio/SoundSource.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/LiquidBlock.hpp"
#include "game/world/block/Material.hpp"
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

    BlockPos pos = hitResult->getBlockPos();
    if (!level->mayInteract(player, pos, getBlock())
        || player->mayUseItemAt(pos.relative(hitResult->mDirection), hitResult->mDirection, itemInstance) != 1)
    {
        delete hitResult;
        return false;
    }
    delete hitResult;

    BlockPos above = pos.above();
    const BlockState* state = level->getBlockState(pos);
    return state->getBlock()->mMaterial == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0
        && level->isEmptyBlock(above);
}

InteractionResultHolder WaterLilyBlockItem::use(Level* level, boost::shared_ptr<Player> player,
                                                InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hitResult = Item::getPlayerPOVHitResult(level, player, true);
    if (hitResult == NULL)
    {
        return InteractionResultHolder(InteractionResult::eInteractionResult_Pass, itemInstance);
    }

    if (hitResult->mType == HitResult::BLOCK)
    {
        BlockPos pos = hitResult->getBlockPos();
        if (!level->mayInteract(player, pos, getBlock())
            || !player->mayUseItemAt(pos.relative(hitResult->mDirection), hitResult->mDirection, itemInstance))
        {
            delete hitResult;
            return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
        }
        delete hitResult;

        BlockPos above = pos.above();
        const BlockState* state = level->getBlockState(pos);
        if (state->getBlock()->mMaterial == Material::WATER && state->getValue(LiquidBlock::LEVEL) == 0
            && level->isEmptyBlock(above))
        {
            level->setBlock(above, Blocks::waterlily->defaultBlockState(), Block::UPDATE_ALL | Block::UPDATE_IMMEDIATE, false);
            if (!player->mAbilities.mInstabuild)
            {
                itemInstance->shrink(1);
            }
            level->playSound(player, pos, SoundEvents::WATERLILY_PLACE, SoundSource::eSoundSource_Blocks, 1.0f, 1.0f,
                             16.0f);
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
        }
    }
    else
    {
        delete hitResult;
    }

    return InteractionResultHolder(InteractionResult::eInteractionResult_Fail, itemInstance);
}

unsigned int WaterLilyBlockItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return TestUse((Level*)tooltipData.m_pLevel,
                   (boost::shared_ptr<Player>&)tooltipData.m_pPlayer,
                   InteractionHand::eInteractionHand_MainHand)
        ? IDS_TOOLTIPS_PLACE : IDS_NONE;
}
