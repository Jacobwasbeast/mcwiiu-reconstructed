#include "game/item/BindingCurseEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"

BindingCurseEnchantment::BindingCurseEnchantment(const Rarity* rarity,
                                                 arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR, slots)
{
    setDescriptionId((int)0x7b5dbcd8);
}

BindingCurseEnchantment::~BindingCurseEnchantment()
{
}

int BindingCurseEnchantment::getMinCost(int level)
{
    return 25;
}

int BindingCurseEnchantment::getMaxCost(int level)
{
    return 50;
}

int BindingCurseEnchantment::getMaxLevel()
{
    return 1;
}

bool BindingCurseEnchantment::isTreasureOnly()
{
    return true;
}

bool BindingCurseEnchantment::isCurse()
{
    return true;
}
