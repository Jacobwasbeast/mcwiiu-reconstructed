#include "game/item/WaterWalkerEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/Enchantments.hpp"

WaterWalkerEnchantment::WaterWalkerEnchantment(
    const Rarity* rarity,
    arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR, slots)
{
    setDescriptionId(0xd41baebf);
}

WaterWalkerEnchantment::~WaterWalkerEnchantment()
{
}

int WaterWalkerEnchantment::getMinCost(int level)
{
    return level * 10;
}

int WaterWalkerEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 15;
}

int WaterWalkerEnchantment::getMaxLevel()
{
    return 3;
}

bool WaterWalkerEnchantment::checkCompatibility(const Enchantment* other) const
{
    return Enchantment::checkCompatibility(other) && other != Enchantments::FROST_WALKER;
}
