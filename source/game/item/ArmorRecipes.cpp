#include "game/item/ArmorRecipes.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"

void ArmorRecipes::_init()
{
    mIngredients.push_back(new Object(Items::leather, 0, 0, 0x4000202));
    mIngredients.push_back(new Object(Items::ironIngot, 0, 0, 0x4000202));
    mIngredients.push_back(new Object(Items::diamond, 0, 0, 0x4000202));
    mIngredients.push_back(new Object(Items::goldIngot, 0, 0, 0x4000202));
    mIngredients.push_back(new Object(Items::turtleShellPiece, 0, 0, 0x4000202));

    mHelmets.push_back(new Object(Items::leatherHelmet, 0, 0, 0x4000205));
    mHelmets.push_back(new Object(Items::ironHelmet, 0, 0, 0x4000205));
    mHelmets.push_back(new Object(Items::diamondHelmet, 0, 0, 0x4000205));
    mHelmets.push_back(new Object(Items::goldenHelmet, 0, 0, 0x4000205));
    mHelmets.push_back(new Object(Items::turtleHelmet, 0, 0, 0x4000205));

    mChestplates.push_back(new Object(Items::leatherChestplate, 0, 0, 0x4000205));
    mChestplates.push_back(new Object(Items::ironChestplate, 0, 0, 0x4000205));
    mChestplates.push_back(new Object(Items::diamondChestplate, 0, 0, 0x4000205));
    mChestplates.push_back(new Object(Items::goldenChestplate, 0, 0, 0x4000205));

    mLeggings.push_back(new Object(Items::leatherLeggings, 0, 0, 0x4000205));
    mLeggings.push_back(new Object(Items::ironLeggings, 0, 0, 0x4000205));
    mLeggings.push_back(new Object(Items::diamondLeggings, 0, 0, 0x4000205));
    mLeggings.push_back(new Object(Items::goldenLeggings, 0, 0, 0x4000205));

    mBoots.push_back(new Object(Items::leatherBoots, 0, 0, 0x4000205));
    mBoots.push_back(new Object(Items::ironBoots, 0, 0, 0x4000205));
    mBoots.push_back(new Object(Items::diamondBoots, 0, 0, 0x4000205));
    mBoots.push_back(new Object(Items::goldenBoots, 0, 0, 0x4000205));
}

void ArmorRecipes::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mIngredients.size(); ++i)
    {
        Object* ingredient = mIngredients.at(i);

        if (i < mHelmets.size())
        {
            wchar_t key = ingredient->value == 0 && ingredient->flags == 0x4000206 ? L't' : L'w';
            recipes->addShapedRecipe(new ItemInstance(mHelmets.at(i)->item), L"www", L"w w",
                key, ingredient->item, 0x41);
        }
        if (i < mChestplates.size())
        {
            wchar_t key = ingredient->value == 0 && ingredient->flags == 0x4000206 ? L't' : L'w';
            recipes->addShapedRecipe(new ItemInstance(mChestplates.at(i)->item), L"w w", L"www", L"www",
                key, ingredient->item, 0x41);
        }
        if (i < mLeggings.size())
        {
            wchar_t key = ingredient->value == 0 && ingredient->flags == 0x4000206 ? L't' : L'w';
            recipes->addShapedRecipe(new ItemInstance(mLeggings.at(i)->item), L"www", L"w w", L"w w",
                key, ingredient->item, 0x41);
        }
        if (i < mBoots.size())
        {
            wchar_t key = ingredient->value == 0 && ingredient->flags == 0x4000206 ? L't' : L'w';
            recipes->addShapedRecipe(new ItemInstance(mBoots.at(i)->item), L"w w", L"w w",
                key, ingredient->item, 0x41);
        }
    }

    recipes->addShapedRecipe(new ItemInstance(Items::leatherHorseArmor), L"sss", L"s s",
        L'~', Items::turtleShellPiece, 0x41);
}

void ArmorRecipes::GetArmorType(Item* item)
{
    item->GetArmorType();
}
