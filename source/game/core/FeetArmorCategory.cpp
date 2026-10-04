#include "game/core/FeetArmorCategory.hpp"

#include "game/core/EquipmentSlot.hpp"
#include "game/item/ArmorItem.hpp"

bool FeetArmorCategory::canEnchant(Item* item) const
{
    ArmorItem* armorItem = dynamic_cast<ArmorItem*>(item);
    return armorItem != NULL && armorItem->mSlot == EquipmentSlot::FEET;
}
