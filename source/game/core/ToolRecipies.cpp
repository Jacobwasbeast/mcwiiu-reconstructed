#include "game/core/ToolRecipies.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void ToolRecipies::_init()
{
    mTools.resize(5);

    mTools[0].push_back(Item::byBlock(Blocks::planks));
    mTools[0].push_back(Item::byBlock(Blocks::cobblestone));
    mTools[0].push_back(Items::ironIngot);
    mTools[0].push_back(Items::goldIngot);
    mTools[0].push_back(Items::diamond);

    mTools[1].push_back(Items::woodenPickaxe);
    mTools[1].push_back(Items::stonePickaxe);
    mTools[1].push_back(Items::ironPickaxe);
    mTools[1].push_back(Items::goldenPickaxe);
    mTools[1].push_back(Items::diamondPickaxe);

    mTools[2].push_back(Items::woodenShovel);
    mTools[2].push_back(Items::stoneShovel);
    mTools[2].push_back(Items::ironShovel);
    mTools[2].push_back(Items::goldenShovel);
    mTools[2].push_back(Items::diamondShovel);

    mTools[3].push_back(Items::woodenAxe);
    mTools[3].push_back(Items::stoneAxe);
    mTools[3].push_back(Items::ironAxe);
    mTools[3].push_back(Items::goldenAxe);
    mTools[3].push_back(Items::diamondAxe);

    mTools[4].push_back(Items::woodenHoe);
    mTools[4].push_back(Items::stoneHoe);
    mTools[4].push_back(Items::ironHoe);
    mTools[4].push_back(Items::goldenHoe);
    mTools[4].push_back(Items::diamondHoe);
}

void ToolRecipies::addRecipes(Recipes* recipes)
{
    unsigned int i;
    for (i = 0; i < mTools[0].size(); ++i)
    {
        ItemInstance pickaxe(mTools[1][i]);
        recipes->addShapedRecipe(&pickaxe, L"###", L" X ", L" T ", '#', mTools[0][i], 'X', Items::stick, 'T', Items::stick);

        ItemInstance shovel(mTools[2][i]);
        recipes->addShapedRecipe(&shovel, L"#", L"X", L"T", '#', mTools[0][i], 'X', Items::stick, 'T', Items::stick);

        ItemInstance axe(mTools[3][i]);
        recipes->addShapedRecipe(&axe, L"##", L"#X", L" T", '#', mTools[0][i], 'X', Items::stick, 'T', Items::stick);

        ItemInstance hoe(mTools[4][i]);
        recipes->addShapedRecipe(&hoe, L"##", L" X", L" T", '#', mTools[0][i], 'X', Items::stick, 'T', Items::stick);
    }

    ItemInstance shears(Items::shears);
    recipes->addShapedRecipe(&shears, L" #", L"# ", '#', Items::ironIngot);
}
