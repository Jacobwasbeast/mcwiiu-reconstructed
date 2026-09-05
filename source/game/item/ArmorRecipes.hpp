#ifndef GAME_ITEM_ARMORRECIPES_HPP
#define GAME_ITEM_ARMORRECIPES_HPP

#include <vector>

#include "game/item/Item.hpp"

class Recipes;

class ArmorRecipes
{
private:
    struct Object
    {
        Object(Item* objectItem, int objectValue, int objectAuxValue, int objectFlags)
            : item(objectItem), value(objectValue), auxValue(objectAuxValue), flags(objectFlags)
        {
        }

        Item* item;
        int value;
        int auxValue;
        int flags;
    };

    std::vector<Object*> mIngredients;
    std::vector<Object*> mHelmets;
    std::vector<Object*> mChestplates;
    std::vector<Object*> mLeggings;
    std::vector<Object*> mBoots;

public:
    void _init();
    void addRecipes(Recipes* recipes);
    static void GetArmorType(Item* item);
};

#endif
