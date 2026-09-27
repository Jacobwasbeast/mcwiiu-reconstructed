#include "game/core/WeaponRecipies.hpp"

#include <string>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

static const std::wstring SWORD_PATTERN[4] = { L"X", L"X", L"#", L"" };

void WeaponRecipies::_init()
{
    mLists = new std::vector<Object*>[2];

    mLists[0].push_back(new Object(Blocks::planks, 0, Object::eType_Block));
    mLists[0].push_back(new Object(Blocks::cobblestone, 0, Object::eType_Block));
    mLists[0].push_back(new Object(Items::ironIngot, 0, Object::eType_Item));
    mLists[0].push_back(new Object(Items::diamond, 0, Object::eType_Item));
    mLists[0].push_back(new Object(Items::goldIngot, 0, Object::eType_Item));

    mLists[1].push_back(new Object(Items::woodenSword, 0, Object::eType_Item));
    mLists[1].push_back(new Object(Items::stoneSword, 0, Object::eType_Item));
    mLists[1].push_back(new Object(Items::ironSword, 0, Object::eType_Item));
    mLists[1].push_back(new Object(Items::diamondSword, 0, Object::eType_Item));
    mLists[1].push_back(new Object(Items::goldenSword, 0, Object::eType_Item));
}

void WeaponRecipies::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mLists[0].size(); ++i)
    {
        Object* ingredient = mLists[0].at(i);
        Object* weapon = mLists[1].at(i);

        wchar_t format[] = L"wcicig";
        if (ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block)
        {
            format[4] = L't';
        }

        recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(weapon->object)), format,
            SWORD_PATTERN, L'#', Items::stick, L'X', ingredient->object, L'T');
    }
}
