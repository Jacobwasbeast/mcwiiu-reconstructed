#include "game/core/OreRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void OreRecipies::_init()
{
    blockPairs[0].push_back(new Object(Blocks::gold_block, 0, 0x4000206));
    blockPairs[0].push_back(new Object(new ItemInstance(Items::goldIngot, 9), 0, 0x4000203));
    blockPairs[1].push_back(new Object(Blocks::iron_block, 0, 0x4000206));
    blockPairs[1].push_back(new Object(new ItemInstance(Items::ironIngot, 9), 0, 0x4000203));
    blockPairs[2].push_back(new Object(Blocks::diamond_block, 0, 0x4000206));
    blockPairs[2].push_back(new Object(new ItemInstance(Items::diamond, 9), 0, 0x4000203));
    blockPairs[3].push_back(new Object(Blocks::emerald_block, 0, 0x4000206));
    blockPairs[3].push_back(new Object(new ItemInstance(Items::emerald, 9), 0, 0x4000203));
    blockPairs[4].push_back(new Object(Blocks::lapis_block, 0, 0x4000206));
    blockPairs[4].push_back(new Object(new ItemInstance(Items::dye, 9, DyeColor::BLUE->getItemData()), 0, 0x4000203));
    blockPairs[5].push_back(new Object(Blocks::redstone_block, 0, 0x4000206));
    blockPairs[5].push_back(new Object(new ItemInstance(Items::redstone, 9), 0, 0x4000203));
    blockPairs[6].push_back(new Object(Blocks::coal_block, 0, 0x4000206));
    blockPairs[6].push_back(new Object(new ItemInstance(Items::coal, 9, 0), 0, 0x4000203));
    blockPairs[7].push_back(new Object(Blocks::quartz_block, 0, 0x4000206));
    blockPairs[7].push_back(new Object(new ItemInstance(Items::quartz, 9), 0, 0x4000203));
    blockPairs[8].push_back(new Object(Blocks::slime, 0, 0x4000206));
    blockPairs[8].push_back(new Object(new ItemInstance(Items::slimeBall, 9), 0, 0x4000203));
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
            Block* block = static_cast<Block*>(blockPairs[index].at(0)->object);
            ItemInstance* ingot = static_cast<ItemInstance*>(blockPairs[index].at(1)->object);

            recipes->addShapedRecipe(new ItemInstance(block), L"sssczg", L"###", L"###", L"###", '#', ingot, 0x44);
            recipes->addShapedRecipe(ingot, L"sctg", L"#", '#', block, 0x44);
        }
    }

    Block* slime = static_cast<Block*>(blockPairs[8].at(0)->object);
    ItemInstance* slimeBall = static_cast<ItemInstance*>(blockPairs[8].at(1)->object);

    recipes->addShapedRecipe(new ItemInstance(slime), L"sssczg", L"###", L"###", L"###", '#', slimeBall, 0x44);
    recipes->addShapedRecipe(slimeBall, L"sctg", L"#", '#', slime, 0x44);

    recipes->addShapedRecipe(new ItemInstance(Items::string, 9), L"sctbg", L"#", '#', Blocks::web, 0x1f, 0x44);

    recipes->addShapedRecipe(new ItemInstance(Items::goldIngot), L"ssscig", L"###", L"###", L"###", '#',
        Items::goldNugget, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Items::goldNugget, 9), L"scig", L"#", '#', Items::goldIngot, 0x44);

    recipes->addShapedRecipe(new ItemInstance(Items::ironIngot), L"ssscig", L"###", L"###", L"###", '#',
        Items::ironNugget, 0x44);
    recipes->addShapedRecipe(new ItemInstance(Items::ironNugget, 9), L"scig", L"#", '#', Items::ironIngot, 0x44);
}
