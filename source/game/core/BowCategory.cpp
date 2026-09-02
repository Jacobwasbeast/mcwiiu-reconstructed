#include "game/core/BowCategory.hpp"

#include "game/item/BowItem.hpp"
#include "game/item/Item.hpp"

bool BowCategory::canEnchant(Item* item) const
{
    return dynamic_cast<BowItem*>(item) != NULL;
}
