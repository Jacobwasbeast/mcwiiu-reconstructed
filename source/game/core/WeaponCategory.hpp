#ifndef GAME_CORE_WEAPONCATEGORY_HPP
#define GAME_CORE_WEAPONCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class WeaponCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
