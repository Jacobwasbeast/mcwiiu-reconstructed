#include "game/item/LootBonusEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/Enchantments.hpp"

LootBonusEnchantment::LootBonusEnchantment(const Rarity* rarity,
                                           const EnchantmentCategory* category,
                                           arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, category, slots)
{
    if (category == EnchantmentCategory::WEAPON)
    {
        setDescriptionId((int)0xda6036a5);
    }
    else if (category == EnchantmentCategory::DIGGER)
    {
        setDescriptionId((int)0x797d9c28);
    }
    else
    {
        setDescriptionId((int)0x8e00390d);
    }
}

LootBonusEnchantment::~LootBonusEnchantment()
{
}

int LootBonusEnchantment::getMinCost(int level)
{
    return level * 9 + 6;
}

int LootBonusEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int LootBonusEnchantment::getMaxLevel()
{
    return 3;
}

bool LootBonusEnchantment::checkCompatibility(const Enchantment* other) const
{
    return Enchantment::checkCompatibility(other) && other != Enchantments::SILK_TOUCH;
}
