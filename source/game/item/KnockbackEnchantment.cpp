#include "game/item/KnockbackEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"

KnockbackEnchantment::KnockbackEnchantment(const Rarity* rarity, arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::WEAPON, slots)
{
    setDescriptionId(0xC3755A5B);
}

int KnockbackEnchantment::getMinCost(int level)
{
    return level * 20 - 15;
}

int KnockbackEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int KnockbackEnchantment::getMaxLevel()
{
    return 2;
}

KnockbackEnchantment::~KnockbackEnchantment()
{
}
