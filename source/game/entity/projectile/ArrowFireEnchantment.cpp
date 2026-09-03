#include "game/entity/projectile/ArrowFireEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"

ArrowFireEnchantment::ArrowFireEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(0x0EA75E04);
}

int ArrowFireEnchantment::getMinCost(int level)
{
    return 20;
}

int ArrowFireEnchantment::getMaxCost(int level)
{
    return 50;
}

int ArrowFireEnchantment::getMaxLevel()
{
    return 1;
}

ArrowFireEnchantment::~ArrowFireEnchantment()
{
}
