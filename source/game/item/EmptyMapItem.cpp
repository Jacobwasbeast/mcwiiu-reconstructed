#include "game/item/EmptyMapItem.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/GenericStats.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/MapItem.hpp"
#include "game/ui/ItemTooltipDataHolder.hpp"
#include "game/world/generation/Player.hpp"

EmptyMapItem::EmptyMapItem()
    : ComplexItem()
{
}

EmptyMapItem::~EmptyMapItem()
{
}

InteractionResultHolder EmptyMapItem::use(Level* level,
                                          boost::shared_ptr<Player> player,
                                          InteractionHand::EInteractionHand hand)
{
    not_null_ptr<ItemInstance> itemInstance = player->getItemInHand(hand);

    MapItem::setupNewMap(player->mXuid, level, player->mDimension,
                         player->mX, player->mZ, 3, true, false);
    itemInstance->shrink(1);

    if (!itemInstance->isEmpty())
    {
        not_null_ptr<ItemInstance> itemCopy = itemInstance->copy();
        if (!player->mInventory->add(itemCopy))
        {
            player->drop(itemCopy, false);
        }
    }

    player->awardStat(gStats->get_itemsUsed(VALIDgetId(this)),
                      GenericStats::param_itemsUsed(player, itemInstance));

    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, itemInstance);
}

unsigned int EmptyMapItem::GetUseTooltip(const ItemTooltipDataHolder&)
{
    return IDS_TOOLTIPS_USE;
}
