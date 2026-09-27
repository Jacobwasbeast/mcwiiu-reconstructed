#include "game/item/ArmorRecipes.hpp"

#include <string>

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"

enum
{
    eArmorList_Ingredients = 0,
    eArmorList_Helmets = 1,
    eArmorList_Chestplates = 2,
    eArmorList_Leggings = 3,
    eArmorList_Boots = 4,
    eArmorList_Count = 5
};

enum
{
    eArmorShape_Helmet = 0,
    eArmorShape_Chestplate = 1,
    eArmorShape_Leggings = 2,
    eArmorShape_Boots = 3,
    eArmorShape_Count = 4
};

static std::wstring armorRecipeShapes[eArmorShape_Count][4] =
{
    { L"XXX", L"X X", L"" },
    { L"X X", L"XXX", L"XXX", L"" },
    { L"XXX", L"X X", L"X X", L"" },
    { L"X X", L"X X", L"" }
};

void ArmorRecipes::_init()
{
    mLists = new std::vector<Object*>[eArmorList_Count];

    mLists[eArmorList_Ingredients].push_back(new Object(Items::leather, 0, Object::eType_Item));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::ironIngot, 0, Object::eType_Item));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::diamond, 0, Object::eType_Item));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::goldIngot, 0, Object::eType_Item));
    mLists[eArmorList_Ingredients].push_back(new Object(Items::turtleShellPiece, 0, Object::eType_Item));

    mLists[eArmorList_Helmets].push_back(new Object(Items::leatherHelmet, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Helmets].push_back(new Object(Items::ironHelmet, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Helmets].push_back(new Object(Items::diamondHelmet, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Helmets].push_back(new Object(Items::goldenHelmet, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Helmets].push_back(new Object(Items::turtleHelmet, 0, Object::eType_ArmorItem));

    mLists[eArmorList_Chestplates].push_back(new Object(Items::leatherChestplate, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Chestplates].push_back(new Object(Items::ironChestplate, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Chestplates].push_back(new Object(Items::diamondChestplate, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Chestplates].push_back(new Object(Items::goldenChestplate, 0, Object::eType_ArmorItem));

    mLists[eArmorList_Leggings].push_back(new Object(Items::leatherLeggings, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Leggings].push_back(new Object(Items::ironLeggings, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Leggings].push_back(new Object(Items::diamondLeggings, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Leggings].push_back(new Object(Items::goldenLeggings, 0, Object::eType_ArmorItem));

    mLists[eArmorList_Boots].push_back(new Object(Items::leatherBoots, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Boots].push_back(new Object(Items::ironBoots, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Boots].push_back(new Object(Items::diamondBoots, 0, Object::eType_ArmorItem));
    mLists[eArmorList_Boots].push_back(new Object(Items::goldenBoots, 0, Object::eType_ArmorItem));
}

void ArmorRecipes::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mLists[eArmorList_Ingredients].size(); ++i)
    {
        Object* ingredient = mLists[eArmorList_Ingredients].at(i);

        if (i < mLists[eArmorList_Helmets].size())
        {
            Object* helmet = mLists[eArmorList_Helmets].at(i);
            wchar_t types[5];
            types[0] = L'w';
            types[1] = L'c';
            types[2] = ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block ? L't' : L'i';
            types[3] = L'g';
            types[4] = 0;
            recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(helmet->object)), types,
                armorRecipeShapes[eArmorShape_Helmet], L'X', ingredient->object, L'A');
        }

        if (i < mLists[eArmorList_Chestplates].size())
        {
            Object* chestplate = mLists[eArmorList_Chestplates].at(i);
            wchar_t types[5];
            types[0] = L'w';
            types[1] = L'c';
            types[2] = ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block ? L't' : L'i';
            types[3] = L'g';
            types[4] = 0;
            recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(chestplate->object)), types,
                armorRecipeShapes[eArmorShape_Chestplate], L'X', ingredient->object, L'A');
        }

        if (i < mLists[eArmorList_Leggings].size())
        {
            Object* leggings = mLists[eArmorList_Leggings].at(i);
            wchar_t types[5];
            types[0] = L'w';
            types[1] = L'c';
            types[2] = ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block ? L't' : L'i';
            types[3] = L'g';
            types[4] = 0;
            recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(leggings->object)), types,
                armorRecipeShapes[eArmorShape_Leggings], L'X', ingredient->object, L'A');
        }

        if (i < mLists[eArmorList_Boots].size())
        {
            Object* boots = mLists[eArmorList_Boots].at(i);
            wchar_t types[5];
            types[0] = L'w';
            types[1] = L'c';
            types[2] = ingredient->auxValue == 0 && ingredient->flags == Object::eType_Block ? L't' : L'i';
            types[3] = L'g';
            types[4] = 0;
            recipes->addShapedRecipe(new ItemInstance(static_cast<Item*>(boots->object)), types,
                armorRecipeShapes[eArmorShape_Boots], L'X', ingredient->object, L'A');
        }
    }

    recipes->addShapedRecipe(new ItemInstance(Items::leatherHorseArmor), L"ssscig", L"~ ~", L"~~~",
        L"~ ~", L'~', Items::leather, L'A');
}

int ArmorRecipes::GetArmorType(Item* item)
{
    return item->GetArmorType();
}
