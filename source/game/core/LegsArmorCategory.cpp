#include "game/core/LegsArmorCategory.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"

bool LegsArmorCategory::canEnchant(Item* item) const
{
    ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
    return armorItem != NULL && armorItem->getSlot() == EquipmentSlot::LEGS;
}
