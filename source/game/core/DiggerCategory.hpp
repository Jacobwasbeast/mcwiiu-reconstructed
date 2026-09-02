#ifndef GAME_CORE_DIGGERCATEGORY_HPP
#define GAME_CORE_DIGGERCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class DiggerCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
