#include "game/core/WeaponRecipies.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void WeaponRecipies::_init()
{
    mIngredients.push_back(new Object(Blocks::planks, 0, 0, 0x4000206));
    mIngredients.push_back(new Object(Blocks::cobblestone, 0, 0, 0x4000206));
    mIngredients.push_back(new Object(Items::ironIngot, 0, 0, 0x4000202));
    mIngredients.push_back(new Object(Items::diamond, 0, 0, 0x4000202));
    mIngredients.push_back(new Object(Items::goldIngot, 0, 0, 0x4000202));

    mWeapons.push_back(new Object(Items::woodenSword, 0, 0, 0x4000202));
    mWeapons.push_back(new Object(Items::stoneSword, 0, 0, 0x4000202));
    mWeapons.push_back(new Object(Items::ironSword, 0, 0, 0x4000202));
    mWeapons.push_back(new Object(Items::diamondSword, 0, 0, 0x4000202));
    mWeapons.push_back(new Object(Items::goldenSword, 0, 0, 0x4000202));
}

void WeaponRecipies::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mIngredients.size(); ++i)
    {
        Object* ingredient = mIngredients.at(i);
        Object* weapon = mWeapons.at(i);
        const wchar_t* pattern = (ingredient->value == 0 && ingredient->flags == 0x4000206)
            ? L"wcictg" : L"wcicig";
        ItemInstance result(static_cast<Item*>(weapon->object));
        recipes->addShapedRecipe(&result, pattern, static_cast<const wchar_t*>(NULL), '#', Items::stick,
            'X', ingredient, 'T');
    }
}
