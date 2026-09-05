#include "game/item/TakeItemHint.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/minigame/Tutorial.hpp"

TakeItemHint::~TakeItemHint()
{
    delete[] itemIds;
    itemIds = 0;
}

unsigned int TakeItemHint::onTake(not_null_ptr<ItemInstance> item)
{
    if (!item->isEmpty() && itemCount != 0)
    {
        for (unsigned int i = 0; i < itemCount; ++i)
        {
            if (Item::VALIDgetId(item->getItem()) == itemIds[i])
            {
                Tutorial::PopupMessageDetails* details = new Tutorial::PopupMessageDetails();
                if (descriptionId == -1)
                {
                    details->messageId = item->getUseDescriptionId();
                }
                else
                {
                    details->messageId = descriptionId;
                }
                details->itemDescriptionId = item->getDescriptionId(-1);
                details->itemId = Item::VALIDgetId(item->getItem());
                details->isItem = true;
                return tutorial->setMessage(this, details);
            }
        }
    }
    return 0;
}
