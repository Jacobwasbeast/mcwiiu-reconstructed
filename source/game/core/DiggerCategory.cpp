#include "game/core/DiggerCategory.hpp"

#include "game/item/DiggerItem.hpp"

bool DiggerCategory::canEnchant(Item* item) const
{
    return dynamic_cast<DiggerItem*>(item) != NULL;
}
