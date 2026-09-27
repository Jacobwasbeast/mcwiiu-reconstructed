#include "game/core/OreRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void OreRecipies::_init()
{
    blockPairs[0].push_back(new Object(Blocks::gold_block, 0, Object::eType_Block));
    blockPairs[0].push_back(new Object(new ItemInstance(Items::goldIngot, 9), 0, Object::eType_ItemInstance));
    blockPairs[1].push_back(new Object(Blocks::iron_block, 0, Object::eType_Block));
    blockPairs[1].push_back(new Object(new ItemInstance(Items::ironIngot, 9), 0, Object::eType_ItemInstance));
    blockPairs[2].push_back(new Object(Blocks::diamond_block, 0, Object::eType_Block));
    blockPairs[2].push_back(new Object(new ItemInstance(Items::diamond, 9), 0, Object::eType_ItemInstance));
    blockPairs[3].push_back(new Object(Blocks::emerald_block, 0, Object::eType_Block));
    blockPairs[3].push_back(new Object(new ItemInstance(Items::emerald, 9), 0, Object::eType_ItemInstance));
    blockPairs[4].push_back(new Object(Blocks::lapis_block, 0, Object::eType_Block));
    blockPairs[4].push_back(new Object(new ItemInstance(Items::dye, 9, DyeColor::BLUE->getItemData()), 0, Object::eType_ItemInstance));
    blockPairs[5].push_back(new Object(Blocks::redstone_block, 0, Object::eType_Block));
    blockPairs[5].push_back(new Object(new ItemInstance(Items::redstone, 9), 0, Object::eType_ItemInstance));
    blockPairs[6].push_back(new Object(Blocks::coal_block, 0, Object::eType_Block));
    blockPairs[6].push_back(new Object(new ItemInstance(Items::coal, 9, 0), 0, Object::eType_ItemInstance));
    blockPairs[7].push_back(new Object(Blocks::hay_block, 0, Object::eType_Block));
    blockPairs[7].push_back(new Object(new ItemInstance(Items::wheat, 9), 0, Object::eType_ItemInstance));
    blockPairs[8].push_back(new Object(Blocks::slime, 0, Object::eType_Block));
    blockPairs[8].push_back(new Object(new ItemInstance(Items::slimeBall, 9), 0, Object::eType_ItemInstance));
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

            recipes->addShapedRecipe(new ItemInstance(block), L"sssczg", L"###", L"###", L"###", L'#', ingot, L'D');
            recipes->addShapedRecipe(ingot, L"sctg", L"#", L'#', block, L'D');
        }
    }

    Block* slime = static_cast<Block*>(blockPairs[8].at(0)->object);
    ItemInstance* slimeBall = static_cast<ItemInstance*>(blockPairs[8].at(1)->object);

    recipes->addShapedRecipe(new ItemInstance(slime), L"sssczg", L"###", L"###", L"###", L'#', slimeBall, L'D');
    recipes->addShapedRecipe(slimeBall, L"sctg", L"#", L'#', slime, L'D');

    recipes->addShapedRecipe(new ItemInstance(Items::string, 9), L"sctbg", L"#", L'#', Blocks::web, 0x1f, L'D');

    recipes->addShapedRecipe(new ItemInstance(Items::goldIngot), L"ssscig", L"###", L"###", L"###", L'#',
        Items::goldNugget, L'D');
    recipes->addShapedRecipe(new ItemInstance(Items::goldNugget, 9), L"scig", L"#", L'#', Items::goldIngot, L'D');

    recipes->addShapedRecipe(new ItemInstance(Items::ironIngot), L"ssscig", L"###", L"###", L"###", L'#',
        Items::ironNugget, L'D');
    recipes->addShapedRecipe(new ItemInstance(Items::ironNugget, 9), L"scig", L"#", L'#', Items::ironIngot, L'D');
}
