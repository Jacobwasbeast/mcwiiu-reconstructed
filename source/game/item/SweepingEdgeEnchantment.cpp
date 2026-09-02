#include "game/item/SweepingEdgeEnchantment.hpp"

int SweepingEdgeEnchantment::getMaxLevel()
{
    return 3;
}

int SweepingEdgeEnchantment::getMinCost(int level)
{
    return level * 9 - 4;
}

int SweepingEdgeEnchantment::getMaxCost(int level)
{
    return getMinCost(level) + 15;
}

int SweepingEdgeEnchantment::getDescriptionId()
{
    return 0x7C8A12EF;
}

SweepingEdgeEnchantment::~SweepingEdgeEnchantment()
{
}
