#include "game/entity/projectile/ArrowInfiniteEnchantment.hpp"
#include "game/item/EnchantmentCategory.hpp"
#include "game/item/MendingEnchantment.hpp"

ArrowInfiniteEnchantment::ArrowInfiniteEnchantment(const Rarity* rarity,
                                                   arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, EnchantmentCategory::BOW, slots)
{
    setDescriptionId(0xd251ec6a);
}

int ArrowInfiniteEnchantment::getMinCost(int)
{
    return 20;
}

int ArrowInfiniteEnchantment::getMaxCost(int)
{
    return 50;
}

int ArrowInfiniteEnchantment::getMaxLevel()
{
    return 1;
}

bool ArrowInfiniteEnchantment::checkCompatibility(const Enchantment* other) const
{
    if (other != NULL && dynamic_cast<const MendingEnchantment*>(other) != NULL)
    {
        return false;
    }
    return Enchantment::checkCompatibility(other);
}

ArrowInfiniteEnchantment::~ArrowInfiniteEnchantment()
{
}
