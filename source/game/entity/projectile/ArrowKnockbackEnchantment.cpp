#include "game/entity/projectile/ArrowKnockbackEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"

ArrowKnockbackEnchantment::ArrowKnockbackEnchantment(const Rarity* rarity,
                                                     arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(0x86d14a8d);
}

int ArrowKnockbackEnchantment::getMinCost(int level)
{
    return level * 20 - 8;
}

int ArrowKnockbackEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 25;
}

int ArrowKnockbackEnchantment::getMaxLevel()
{
    return 2;
}

ArrowKnockbackEnchantment::~ArrowKnockbackEnchantment()
{
}
