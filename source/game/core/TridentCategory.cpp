#include "game/core/TridentCategory.hpp"

#include "game/item/Item.hpp"
#include "game/item/TridentItem.hpp"

bool TridentCategory::canEnchant(Item* item) const
{
    return dynamic_cast<TridentItem*>(item) != NULL;
}
