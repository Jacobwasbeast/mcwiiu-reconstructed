#include "game/core/AllCategory.hpp"

#include "game/item/Item.hpp"

bool AllCategory::canEnchant(Item* item) const
{
    for (std::vector<const EnchantmentCategory*>::const_iterator category = CATEGORIES.begin();
         category != CATEGORIES.end();
         ++category)
    {
        if (*category != ALL && (*category)->canEnchant(item))
        {
            return true;
        }
    }

    return false;
}
