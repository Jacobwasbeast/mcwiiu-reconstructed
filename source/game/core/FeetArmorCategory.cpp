#include "game/core/FeetArmorCategory.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/Item.hpp"

bool FeetArmorCategory::canEnchant(Item* item) const
{
    ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
    return armorItem != NULL && armorItem->getSlot() == EquipmentSlot::FEET;
}
