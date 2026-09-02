#include "game/core/FishingRodCategory.hpp"

#include "game/item/FishingRodItem.hpp"
#include "game/item/Item.hpp"

bool FishingRodCategory::canEnchant(Item* item) const
{
    return dynamic_cast<FishingRodItem*>(item) != NULL;
}
