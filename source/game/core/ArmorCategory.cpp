#include "game/core/ArmorCategory.hpp"

#include "game/item/ArmorItem.hpp"
#include "game/item/Item.hpp"

bool ArmorCategory::canEnchant(Item* item) const
{
    return dynamic_cast<ArmorItem*>(item) != NULL;
}
