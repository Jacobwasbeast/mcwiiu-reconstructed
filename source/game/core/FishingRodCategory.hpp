#ifndef GAME_CORE_FISHINGRODCATEGORY_HPP
#define GAME_CORE_FISHINGRODCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class FishingRodCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
