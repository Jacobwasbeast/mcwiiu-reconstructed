#ifndef GAME_CORE_TRIDENTCATEGORY_HPP
#define GAME_CORE_TRIDENTCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class TridentCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
