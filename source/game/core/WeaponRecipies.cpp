#include "game/core/WeaponRecipies.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void WeaponRecipies::_init()
{
    mIngredients.push_back(Blocks::planks);
    mIngredients.push_back(Blocks::cobblestone);
    mIngredients.push_back(Items::ironIngot);
    mIngredients.push_back(Items::diamond);
    mIngredients.push_back(Items::goldIngot);

    mWeapons.push_back(Items::woodenSword);
    mWeapons.push_back(Items::stoneSword);
    mWeapons.push_back(Items::ironSword);
    mWeapons.push_back(Items::diamondSword);
    mWeapons.push_back(Items::goldenSword);
}

void WeaponRecipies::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mIngredients.size(); ++i)
    {
        void* ingredient = mIngredients.at(i);
        const wchar_t* pattern = (ingredient == Blocks::planks || ingredient == Blocks::cobblestone)
            ? L"wcictg" : L"wcicig";
        ItemInstance result(static_cast<Item*>(mWeapons.at(i)));
        recipes->addShapedRecipe(&result, pattern, static_cast<const wchar_t*>(NULL), '#', Items::stick,
            'X', ingredient, 'T');
    }
}
