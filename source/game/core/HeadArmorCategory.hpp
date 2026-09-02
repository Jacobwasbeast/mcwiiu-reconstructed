#ifndef GAME_CORE_HEADARMORCATEGORY_HPP
#define GAME_CORE_HEADARMORCATEGORY_HPP

#include "game/item/EnchantmentCategory.hpp"

class Item;

class HeadArmorCategory : public EnchantmentCategory
{
public:
    virtual bool canEnchant(Item* item) const;
};

#endif
