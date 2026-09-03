#include "game/item/OxygenEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"

OxygenEnchantment::OxygenEnchantment(const Rarity* rarity,
                                     arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR_HEAD, slots)
{
    setDescriptionId(0x3349d040);
}

OxygenEnchantment::~OxygenEnchantment()
{
}

int OxygenEnchantment::getMinCost(int level)
{
    return level * 10;
}

int OxygenEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 30;
}

int OxygenEnchantment::getMaxLevel()
{
    return 3;
}
