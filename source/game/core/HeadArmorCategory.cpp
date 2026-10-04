#include "game/core/HeadArmorCategory.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"

bool HeadArmorCategory::canEnchant(Item* item) const
{
    ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
    return armorItem != NULL && armorItem->mSlot == EquipmentSlot::HEAD;
}
