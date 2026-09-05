#include "game/core/OreRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void OreRecipies::_init()
{
    blockPairs[0].push_back(Blocks::gold_block);
    blockPairs[0].push_back(new ItemInstance(Items::goldIngot, 9));
    blockPairs[1].push_back(Blocks::iron_block);
    blockPairs[1].push_back(new ItemInstance(Items::ironIngot, 9));
    blockPairs[2].push_back(Blocks::diamond_block);
    blockPairs[2].push_back(new ItemInstance(Items::diamond, 9));
    blockPairs[3].push_back(Blocks::emerald_block);
    blockPairs[3].push_back(new ItemInstance(Items::emerald, 9));
    blockPairs[4].push_back(Blocks::lapis_block);
    blockPairs[4].push_back(new ItemInstance(Items::dye, 9, DyeColor::BLUE->getItemData()));
    blockPairs[5].push_back(Blocks::redstone_block);
    blockPairs[5].push_back(new ItemInstance(Items::redstone, 9));
    blockPairs[6].push_back(Blocks::coal_block);
    blockPairs[6].push_back(new ItemInstance(Items::coal, 9, 0));
    blockPairs[7].push_back(Blocks::quartz_block);
    blockPairs[7].push_back(new ItemInstance(Items::quartz, 9));
    blockPairs[8].push_back(Blocks::slime);
    blockPairs[8].push_back(new ItemInstance(Items::slimeBall, 9));
}

void OreRecipies::addRecipes(Recipes* recipes)
{
    int row;
    int column;

    for (row = 0; row < 2; ++row)
    {
        for (column = 0; column < 4; ++column)
        {
            int index = row * 4 + column;
            ItemInstance result(static_cast<Block*>(blockPairs[index].at(0)));
            recipes->addShapedRecipe(&result, L"###", L"###", L"###", '#',
                static_cast<ItemInstance*>(blockPairs[index].at(1)), 0x44);
            recipes->addShapedRecipe(static_cast<ItemInstance*>(blockPairs[index].at(1)), L"#", '#',
                static_cast<Block*>(blockPairs[index].at(0)), 0x44);
        }
    }

    recipes->addShapedRecipe(static_cast<ItemInstance*>(blockPairs[8].at(1)), L"#", '#',
        static_cast<Block*>(blockPairs[8].at(0)), 0x44);

    ItemInstance string(Items::string, 9);
    recipes->addShapedRecipe(&string, L"#", '#', Blocks::web, 0x1f, 0x44);

    ItemInstance goldIngot(Items::goldIngot);
    recipes->addShapedRecipe(&goldIngot, L"###", L"###", L"###", '#', Items::goldNugget, 0x44);
    ItemInstance goldNugget(Items::goldNugget, 9);
    recipes->addShapedRecipe(&goldNugget, L"#", '#', Items::goldIngot, 0x44);

    ItemInstance ironIngot(Items::ironIngot);
    recipes->addShapedRecipe(&ironIngot, L"###", L"###", L"###", '#', Items::ironNugget, 0x44);
    ItemInstance ironNugget(Items::ironNugget, 9);
    recipes->addShapedRecipe(&ironNugget, L"#", '#', Items::ironIngot, 0x44);
}
