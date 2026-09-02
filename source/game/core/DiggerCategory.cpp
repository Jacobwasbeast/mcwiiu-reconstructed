#include "game/core/DiggerCategory.hpp"

#include "game/item/DiggerItem.hpp"
#include "game/item/Item.hpp"

bool DiggerCategory::canEnchant(Item* item) const
{
    return dynamic_cast<DiggerItem*>(item) != NULL;
}
