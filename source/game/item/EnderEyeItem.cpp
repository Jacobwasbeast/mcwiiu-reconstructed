#include "game/item/EnderEyeItem.hpp"

#include <boost/shared_ptr.hpp>

#include "game/core/EyeOfEnderSignal.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/HitResult.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockPattern.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/EndPortalFrameBlock.hpp"
#include "game/world/generation/Player.hpp"

EnderEyeItem::EnderEyeItem() : Item()
{
}

EnderEyeItem::~EnderEyeItem()
{
}

InteractionResult::EInteractionResult EnderEyeItem::useOn(boost::shared_ptr<Player> player, Level* level,
                                                            const BlockPos& pos, InteractionHand::EInteractionHand hand,
                                                            const Direction* direction, float hitX, float hitY,
                                                            float hitZ, bool simulate)
{
    const BlockPos framePos = pos.relative(direction);
    const BlockState* state = level->getBlockState(framePos);
    if (state == NULL || state->getBlock() != Blocks::end_portal_frame
        || state->getValue(EndPortalFrameBlock::EYE))
    {
        return InteractionResult::eInteractionResult_Pass;
    }
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!simulate)
    {
        bool eye = true;
        level->setBlock(framePos, state->setBoxedValue(EndPortalFrameBlock::EYE, &eye), 2, false);
        level->levelEvent(1038, framePos, 0);
        itemInstance->shrink(1);
        BlockPattern* pattern = EndPortalFrameBlock::getOrCreatePortalShape();
        BlockPattern::BlockPatternMatch* match = pattern->find(level, framePos);
        if (match != NULL)
        {
            const BlockPos origin = match->getFrontTopLeft().offset(-2, 0, -2);
            for (int x = 0; x < 3; ++x)
            {
                for (int z = 0; z < 3; ++z)
                {
                    level->setBlock(origin.offset(x, 0, z), Blocks::end_portal->defaultBlockState(), 2, false);
                }
            }
            level->globalLevelEvent(1038, match->getFrontTopLeft(), 0);
        }
    }
    return InteractionResult::eInteractionResult_Success;
}

bool EnderEyeItem::TestUse(Level* level, boost::shared_ptr<Player> player, InteractionHand::EInteractionHand hand)
{
    HitResult* hit = getPlayerPOVHitResult(level, player, false);
    if (hit == NULL)
    {
        return true;
    }
    const BlockState* state = level->getBlockState(hit->getBlockPos());
    return state == NULL || state->getBlock() != Blocks::end_portal_frame
        || !state->getValue(EndPortalFrameBlock::EYE);
}

InteractionResultHolder EnderEyeItem::use(Level* level, boost::shared_ptr<Player> player,
                                           InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    HitResult* hit = getPlayerPOVHitResult(level, player, false);
    if (hit != NULL && hit->mType == HitResult::BLOCK)
    {
        const BlockState* state = level->getBlockState(hit->getBlockPos());
        if (state != NULL && state->getBlock() == Blocks::end_portal_frame
            && !state->getValue(EndPortalFrameBlock::EYE))
        {
            useOn(player, level, hit->getBlockPos(), hand, hit->mDirection, hit->mLocation->x,
                  hit->mLocation->y, hit->mLocation->z, false);
            return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
        }
    }
    if (!level->mIsClientSide)
    {
        boost::shared_ptr<EyeOfEnderSignal> signal(new EyeOfEnderSignal(level, player->mX, player->mY, player->mZ));
        signal->signalTo(hit == NULL ? BlockPos(player->mX, player->mY, player->mZ) : hit->getBlockPos());
        level->addEntity(signal);
    }
    player->swing(hand);
    if (!player->mAbilities.mInstabuild)
    {
        itemInstance->shrink(1);
    }
    player->awardStat(gStats->get_itemsUsed(VALIDgetId(this)),
                      GenericStats::param_itemsUsed(player, itemInstance));
    level->levelEvent(player, 1003, BlockPos(player->mX, player->mY, player->mZ), 0);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int EnderEyeItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return tooltipData.m_eInteractionResult == InteractionResult::eInteractionResult_Fail ? 0xffffffffu : 0x3c0788a3u;
}
