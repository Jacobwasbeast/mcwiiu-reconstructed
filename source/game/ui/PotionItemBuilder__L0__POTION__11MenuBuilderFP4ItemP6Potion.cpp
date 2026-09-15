#include "game/ui/PotionItemBuilder__L0__POTION__11MenuBuilderFP4ItemP6Potion.hpp"

not_null_ptr<ItemInstance>
PotionItemBuilder__L0__POTION__11MenuBuilderFP4ItemP6Potion::build() const
{
    not_null_ptr<ItemInstance> result = item.copy();
    return PotionUtils::setPotion(result, potion);
}
