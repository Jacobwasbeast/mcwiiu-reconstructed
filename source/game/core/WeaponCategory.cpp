#include "game/core/WeaponCategory.hpp"

#include "game/item/Item.hpp"
#include "game/item/SwordItem.hpp"

bool WeaponCategory::canEnchant(Item* item) const
{
    return dynamic_cast<SwordItem*>(item) != NULL;
}
