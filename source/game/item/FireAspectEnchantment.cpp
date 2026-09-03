#include "game/item/FireAspectEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"

FireAspectEnchantment::FireAspectEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::WEAPON, slots)
{
    setDescriptionId(0x96021422);
}

int FireAspectEnchantment::getMinCost(int level)
{
    return level * 20 - 10;
}

int FireAspectEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int FireAspectEnchantment::getMaxLevel()
{
    return 2;
}

FireAspectEnchantment::~FireAspectEnchantment()
{
}
