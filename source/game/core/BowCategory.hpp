#ifndef GAME_CORE_BOWCATEGORY_HPP
#define GAME_CORE_BOWCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class BowCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
