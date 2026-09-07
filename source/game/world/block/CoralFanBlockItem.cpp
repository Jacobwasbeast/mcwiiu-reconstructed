#include "game/world/block/CoralFanBlockItem.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"

CoralFanBlockItem::CoralFanBlockItem(Block* block)
    : BlockItem(block), mIsDead(block == Blocks::coral_fan_dead)
{
    setStackedByData(true);
    setMaxDamage(0);
}

CoralFanBlockItem::~CoralFanBlockItem()
{
}

int CoralFanBlockItem::getDescriptionId(not_null_ptr<ItemInstance> itemInstance)
{
    if (mIsDead)
    {
        return Blocks::coral_fan_dead->getDescriptionId(-1);
    }
    return Blocks::coral_fan->getDescriptionId(itemInstance->getAuxValue());
}

int CoralFanBlockItem::getUseDescriptionId(not_null_ptr<ItemInstance>)
{
    if (mIsDead)
    {
        return Blocks::coral_fan_dead->getUseDescriptionId(-1);
    }
    return Blocks::coral_fan->getUseDescriptionId(-1);
}

InteractionResult::EInteractionResult CoralFanBlockItem::useOn(boost::shared_ptr<Player> player,
                                                                Level* level,
                                                                const BlockPos& pos,
                                                                InteractionHand::EInteractionHand hand,
                                                                const Direction* direction,
                                                                float hitX,
                                                                float hitY,
                                                                float hitZ,
                                                                bool simulate)
{
    return BlockItem::useOn(player, level, pos, hand, direction, hitX, hitY, hitZ, simulate);
}

int CoralFanBlockItem::getLevelDataForAuxValue(int auxValue)
{
    return auxValue;
}
