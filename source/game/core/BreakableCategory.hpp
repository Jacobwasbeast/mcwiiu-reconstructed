#ifndef GAME_CORE_BREAKABLECATEGORY_HPP
#define GAME_CORE_BREAKABLECATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class BreakableCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
