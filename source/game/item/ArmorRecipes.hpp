#ifndef GAME_ITEM_ARMORRECIPES_HPP
#define GAME_ITEM_ARMORRECIPES_HPP

#include <vector>

#include "game/core/Object.hpp"

class Item;
class Recipes;

class ArmorRecipes
{
public:
    void _init();
    void addRecipes(Recipes* recipes);
    static int GetArmorType(Item* item);

private:
    std::vector<Object*>* mLists;
};

#endif
