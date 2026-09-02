#ifndef GAME_CORE_ARMORCATEGORY_HPP
#define GAME_CORE_ARMORCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class ArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
