#ifndef GAME_CORE_FEETARMORCATEGORY_HPP
#define GAME_CORE_FEETARMORCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class FeetArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
