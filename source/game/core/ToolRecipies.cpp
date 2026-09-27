#include "game/core/ToolRecipies.hpp"

#include <string>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

enum
{
    eToolList_Ingredients = 0,
    eToolList_Pickaxes = 1,
    eToolList_Shovels = 2,
    eToolList_Axes = 3,
    eToolList_Hoes = 4,
    eToolList_Count = 5
};

enum
{
    eToolShape_Pickaxe = 0,
    eToolShape_Shovel = 1,
    eToolShape_Axe = 2,
    eToolShape_Hoe = 3,
    eToolShape_Count = 4
};

static std::wstring toolRecipeShapes[eToolShape_Count][4] =
{
    { L"XXX", L" # ", L" # " },
    { L"X", L"#", L"#" },
    { L"XX", L"X#", L" #" },
    { L"XX", L" #", L" #" }
};

void ToolRecipies::_init()
{
    mLists = new std::vector<Object*>[eToolList_Count];

    mLists[eToolList_Ingredients].push_back(new Object(Blocks::planks, 0, Object::eType_Block));
    mLists[eToolList_Ingredients].push_back(new Object(Blocks::cobblestone, 0, Object::eType_Block));
    mLists[eToolList_Ingredients].push_back(new Object(Items::ironIngot, 0, Object::eType_Item));
    mLists[eToolList_Ingredients].push_back(new Object(Items::diamond, 0, Object::eType_Item));
    mLists[eToolList_Ingredients].push_back(new Object(Items::goldIngot, 0, Object::eType_Item));

    mLists[eToolList_Pickaxes].push_back(new Object(Items::woodenPickaxe, 0, Object::eType_Item));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::stonePickaxe, 0, Object::eType_Item));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::ironPickaxe, 0, Object::eType_Item));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::diamondPickaxe, 0, Object::eType_Item));
    mLists[eToolList_Pickaxes].push_back(new Object(Items::goldenPickaxe, 0, Object::eType_Item));

    mLists[eToolList_Shovels].push_back(new Object(Items::woodenShovel, 0, Object::eType_Item));
    mLists[eToolList_Shovels].push_back(new Object(Items::stoneShovel, 0, Object::eType_Item));
    mLists[eToolList_Shovels].push_back(new Object(Items::ironShovel, 0, Object::eType_Item));
    mLists[eToolList_Shovels].push_back(new Object(Items::diamondShovel, 0, Object::eType_Item));
    mLists[eToolList_Shovels].push_back(new Object(Items::goldenShovel, 0, Object::eType_Item));

    mLists[eToolList_Axes].push_back(new Object(Items::woodenAxe, 0, Object::eType_Item));
    mLists[eToolList_Axes].push_back(new Object(Items::stoneAxe, 0, Object::eType_Item));
    mLists[eToolList_Axes].push_back(new Object(Items::ironAxe, 0, Object::eType_Item));
    mLists[eToolList_Axes].push_back(new Object(Items::diamondAxe, 0, Object::eType_Item));
    mLists[eToolList_Axes].push_back(new Object(Items::goldenAxe, 0, Object::eType_Item));

    mLists[eToolList_Hoes].push_back(new Object(Items::woodenHoe, 0, Object::eType_Item));
    mLists[eToolList_Hoes].push_back(new Object(Items::stoneHoe, 0, Object::eType_Item));
    mLists[eToolList_Hoes].push_back(new Object(Items::ironHoe, 0, Object::eType_Item));
    mLists[eToolList_Hoes].push_back(new Object(Items::diamondHoe, 0, Object::eType_Item));
    mLists[eToolList_Hoes].push_back(new Object(Items::goldenHoe, 0, Object::eType_Item));
}

void ToolRecipies::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mLists[eToolList_Ingredients].size(); ++i)
    {
        Object* ingredient = mLists[eToolList_Ingredients].at(i);

        Object* pickaxe = mLists[eToolList_Pickaxes].at(i);
        wchar_t pickaxeFormat[] = L"wcicig";
        if (ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block)
        {
            pickaxeFormat[4] = L't';
        }
        recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(pickaxe->object)), pickaxeFormat,
            toolRecipeShapes[eToolShape_Pickaxe], L'#', Items::stick, L'X', ingredient->object, L'T');

        Object* shovel = mLists[eToolList_Shovels].at(i);
        wchar_t shovelFormat[] = L"wcicig";
        if (ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block)
        {
            shovelFormat[4] = L't';
        }
        recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(shovel->object)), shovelFormat,
            toolRecipeShapes[eToolShape_Shovel], L'#', Items::stick, L'X', ingredient->object, L'T');

        Object* axe = mLists[eToolList_Axes].at(i);
        wchar_t axeFormat[] = L"wcicig";
        if (ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block)
        {
            axeFormat[4] = L't';
        }
        recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(axe->object)), axeFormat,
            toolRecipeShapes[eToolShape_Axe], L'#', Items::stick, L'X', ingredient->object, L'T');

        Object* hoe = mLists[eToolList_Hoes].at(i);
        wchar_t hoeFormat[] = L"wcicig";
        if (ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block)
        {
            hoeFormat[4] = L't';
        }
        recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(hoe->object)), hoeFormat,
            toolRecipeShapes[eToolShape_Hoe], L'#', Items::stick, L'X', ingredient->object, L'T');
    }

    recipes->addShapedRecipe(new ItemInstance(Items::shears), L"sscig", L" #", L"# ", L'#',
        Items::ironIngot, L'T');
}
