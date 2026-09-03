#include "game/item/SnowballItem.hpp"

#include "game/ui/ItemTooltipDataHolder.hpp"

SnowballItem::SnowballItem() : Item()
{
    setMaxStackSize(16);
}

SnowballItem::~SnowballItem()
{
}

InteractionResultHolder SnowballItem::use(Level*, boost::shared_ptr<Player>, InteractionHand::EInteractionHand)
{
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, not_null_ptr<ItemInstance>());
}

unsigned int SnowballItem::GetUseTooltip(const ItemTooltipDataHolder&)
{
    return 0x3c0788a3u;
}
