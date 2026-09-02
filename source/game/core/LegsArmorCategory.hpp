#ifndef GAME_CORE_LEGSARMORCATEGORY_HPP
#define GAME_CORE_LEGSARMORCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class LegsArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
