#include "game/item/AxeItem.hpp"

#include "game/core/Minecraft.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/EMiniGameId.hpp"
#include "game/world/Level.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/BlockPos.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

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
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);
    if (!player->mayUseItemAt(pos.relative(direction), direction, itemInstance))
    {
        return InteractionResult::eInteractionResult_Fail;
    }

    const BlockState* state = level->getBlockState(pos);
    Block* block = state->getBlock();
    if (block->isStrippable() && !Minecraft::InMiniGame(eMiniGameId_Any, true))
    {
        if (!simulate)
        {
            level->setBlock(pos, block->getStrippedBlock(state), 11, false);
            if (!level->mIsClientSide)
            {
                itemInstance->hurtAndBreak(1, player);
            }
        }
        return InteractionResult::eInteractionResult_Success;
    }
    return InteractionResult::eInteractionResult_Pass;
}
