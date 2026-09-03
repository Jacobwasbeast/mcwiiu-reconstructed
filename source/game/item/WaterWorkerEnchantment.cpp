#include "game/item/WaterWorkerEnchantment.hpp"
#include "game/item/EnchantmentCategory.hpp"

WaterWorkerEnchantment::WaterWorkerEnchantment(const Rarity* rarity,
    arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::ARMOR, slots)
{
    setDescriptionId(0x212cd6ae);
}

WaterWorkerEnchantment::~WaterWorkerEnchantment()
{
}

int WaterWorkerEnchantment::getMinCost(int level)
{
    return 1;
}

int WaterWorkerEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 40;
}

int WaterWorkerEnchantment::getMaxLevel()
{
    return 1;
}
