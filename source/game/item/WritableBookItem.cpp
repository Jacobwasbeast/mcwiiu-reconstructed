#include "game/item/WritableBookItem.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"

WritableBookItem::WritableBookItem()
    : Item()
{
    setMaxStackSize(1);
}

WritableBookItem::~WritableBookItem()
{
}

InteractionResultHolder WritableBookItem::use(Level* level,
                                              boost::shared_ptr<Player> player,
                                              InteractionHand::EInteractionHand hand)
{
    InteractionResultHolder item = Item::use(level, player, hand);
    return InteractionResultHolder(InteractionResult::eInteractionResult_Success, item.getObject());
}

bool WritableBookItem::TestUse(Level* level,
                               boost::shared_ptr<Player> player,
                               InteractionHand::EInteractionHand hand)
{
    return true;
}

bool WritableBookItem::makeSureTagIsValid(CompoundTag* tag)
{
    if (tag == NULL || !tag->contains(L"pages", 9))
    {
        return false;
    }

    ListTag* pages = tag->getList(L"pages", 8);
    for (int index = 0; index < pages->size(); ++index)
    {
        if (pages->getString(index).size() > 256)
        {
            return false;
        }
    }
    return true;
}

unsigned int WritableBookItem::GetUseTooltip(const ItemTooltipDataHolder& tooltipData)
{
    return 0x41590ad1u;
}
