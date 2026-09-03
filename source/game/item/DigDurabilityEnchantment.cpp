#include "game/item/DigDurabilityEnchantment.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

DigDurabilityEnchantment::DigDurabilityEnchantment(const Rarity* rarity,
                                                   arrayWithLength<const EquipmentSlot*> slots)
    : Enchantment(rarity, 0, slots)
{
}

DigDurabilityEnchantment::~DigDurabilityEnchantment()
{
}

int DigDurabilityEnchantment::getMinCost(int level)
{
    return level * 8 - 3;
}

int DigDurabilityEnchantment::getMaxCost(int level)
{
    return Enchantment::getMinCost(level) + 50;
}

int DigDurabilityEnchantment::getMaxLevel()
{
    return 3;
}

bool DigDurabilityEnchantment::canEnchant(not_null_ptr<ItemInstance> item)
{
    return item->isDamageableItem();
}

bool DigDurabilityEnchantment::shouldIgnoreDurabilityDrop(not_null_ptr<ItemInstance> item,
                                                          int level, Random* random)
{
    if (item->getItem() != 0 && random->nextFloat() < 0.6f)
        return false;
    return random->nextInt(level + 1) == 0;
}
