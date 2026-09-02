#ifndef GAME_CORE_CHESTARMORCATEGORY_HPP
#define GAME_CORE_CHESTARMORCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class ChestArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
