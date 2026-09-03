#include "game/item/UntouchingEnchantment.hpp"

#include "game/item/EnchantmentCategory.hpp"
#include "game/item/Enchantments.hpp"

UntouchingEnchantment::UntouchingEnchantment(const Rarity* rarity,
                                             arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::DIGGER, slots)
{
    setDescriptionId((int)0x68376dd4);
}

UntouchingEnchantment::~UntouchingEnchantment()
{
}

int UntouchingEnchantment::getMinCost(int level)
{
    return 15;
}

int UntouchingEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int UntouchingEnchantment::getMaxLevel()
{
    return 1;
}

bool UntouchingEnchantment::checkCompatibility(const Enchantment* other) const
{
    return Enchantment::checkCompatibility(other) && other != Enchantments::BLOCK_FORTUNE;
}
