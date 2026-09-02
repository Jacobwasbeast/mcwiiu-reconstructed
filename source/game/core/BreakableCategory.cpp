#include "game/core/BreakableCategory.hpp"

#include "game/item/Item.hpp"

bool BreakableCategory::canEnchant(Item* item) const
{
    return item->canBeDepleted();
}
