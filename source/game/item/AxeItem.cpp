#include "game/item/AxeItem.hpp"

#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"

static arrayWithLength<Block*> BLOCKS(22, false);

void AxeItem::staticCtor()
{
    BLOCKS.data[0] = Blocks::log;
    BLOCKS.data[1] = Blocks::log2;
    BLOCKS.data[2] = Blocks::planks;
    BLOCKS.data[3] = Blocks::bookshelf;
    BLOCKS.data[4] = Blocks::chest;
    BLOCKS.data[5] = Blocks::trapped_chest;
    BLOCKS.data[6] = Blocks::pumpkin;
    BLOCKS.data[7] = Blocks::lit_pumpkin;
    BLOCKS.data[8] = Blocks::melon_block;
    BLOCKS.data[9] = Blocks::ladder;
    BLOCKS.data[10] = Blocks::wooden_button;
    BLOCKS.data[11] = Blocks::wooden_pressure_plate;
    BLOCKS.data[12] = Blocks::jukebox;
    BLOCKS.data[13] = Blocks::noteblock;
    BLOCKS.data[14] = Blocks::oak_stairs;
    BLOCKS.data[15] = Blocks::spruce_stairs;
    BLOCKS.data[16] = Blocks::birch_stairs;
    BLOCKS.data[17] = Blocks::jungle_stairs;
    BLOCKS.data[18] = Blocks::acacia_stairs;
    BLOCKS.data[19] = Blocks::dark_oak_stairs;
    BLOCKS.data[20] = Blocks::fence;
    BLOCKS.data[21] = Blocks::fence_gate;
}

AxeItem::AxeItem(const Item::Tier* tier)
    : DiggerItem(tier, BLOCKS)
{
    mAttackDamage = 3.0f + tier->getAttackDamageBonus();
}

AxeItem::~AxeItem()
{
}

float AxeItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    Block* block = state->getBlock();
    if (block != Blocks::log && block != Blocks::log2 && block != Blocks::planks && block != Blocks::bookshelf)
    {
        return DiggerItem::getDestroySpeed(itemInstance, state);
    }
    return mSpeed;
}

InteractionResult::EInteractionResult AxeItem::useOn(boost::shared_ptr<Player> player, Level* level,
    const BlockPos& pos, InteractionHand::EInteractionHand hand, const Direction* direction,
    float hitX, float hitY, float hitZ, bool simulate)
{
    LivingEntity* livingPlayer = (LivingEntity*)(void*)player.get();
    not_null_ptr<ItemInstance> itemInstance = livingPlayer->getItemInHand(hand);
    BlockPos targetPos = pos.relative(direction);
    const BlockState* targetState = level->getBlockState(targetPos);
    bool blockUsed = targetState->getBlock()->use(level, targetPos, targetState, player, hand, direction,
                                                   hitX, hitY, hitZ, simulate);
    if (!blockUsed)
    {
        return InteractionResult::eInteractionResult_Fail;
    }
    if (Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!simulate)
        {
            livingPlayer->swing(hand);
            level->levelEvent(player, 11, pos, 0);
            if (!level->mIsClientSide)
            {
                boost::shared_ptr<LivingEntity> entity(player, livingPlayer);
                itemInstance->hurtAndBreak(1, entity);
            }
        }
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Pass;
}
