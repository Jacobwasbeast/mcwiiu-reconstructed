#include "game/item/ShearsItem.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

ShearsItem::ShearsItem()
    : Item()
{
    setMaxStackSize(1);
    setMaxDamage(238);
}

ShearsItem::~ShearsItem()
{
}

bool ShearsItem::mineBlock(not_null_ptr<ItemInstance> itemInstance, Level* level, const BlockState* state, const BlockPos& pos, boost::shared_ptr<LivingEntity> entity)
{
    if (!level->mIsClientSide)
    {
        itemInstance->hurtAndBreak(1, entity);
    }
    return Item::mineBlock(itemInstance, level, state, pos, entity);
}

bool ShearsItem::canDestroySpecial(const BlockState* state)
{
    Block* block = state->getBlock();
    return block == Blocks::web || block == Blocks::leaves || block == Blocks::wool;
}

float ShearsItem::getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state)
{
    Block* block = state->getBlock();
    if (block == Blocks::web || block == Blocks::leaves || block == Blocks::wool)
    {
        return 15.0f;
    }
    return Item::getDestroySpeed(itemInstance, state);
}

InteractionResult::EInteractionResult ShearsItem::useOn(boost::shared_ptr<Player> player, Level* level, const BlockPos& pos, InteractionHand::EInteractionHand hand, const Direction* direction, float hitX, float hitY, float hitZ, bool simulate)
{
    return Item::useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
}
