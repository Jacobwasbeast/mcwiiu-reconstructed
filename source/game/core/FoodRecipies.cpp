#include "game/core/FoodRecipies.hpp"

#include "game/core/DyeColor.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void FoodRecipies::addRecipes(Recipes* recipes)
{
    recipes->addShapedRecipe(new ItemInstance(Items::goldenApple), L"###", L"#X#", L"###", '#', Items::goldIngot, 'X', Items::apple, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::goldenApple, 1, 1), L"###", L"#X#", L"###", '#', Blocks::gold_block, 'X', Items::apple, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::speckledMelon, 1), L"###", L"#X#", L"###", '#', Items::goldNugget, 'X', Items::melon, 0x46);
    recipes->addShapelessRecipe(new ItemInstance(Items::mushroomStew), Blocks::brown_mushroom, Blocks::red_mushroom, Items::bowl, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::cookie, 8), L"#X#", 'X', new ItemInstance(Items::dye, 1, DyeColor::BROWN->getItemData()), '#', Items::wheat, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::rabbitStew), L" R ", L"CPM", L" B ", 'R', new ItemInstance(Items::cookedRabbit), 'C', Items::carrot, 'P', Items::bakedPotato, 'M', Blocks::brown_mushroom, 'B', Items::bowl, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::rabbitStew), L" R ", L"CPM", L" B ", 'R', new ItemInstance(Items::cookedRabbit), 'C', Items::carrot, 'P', Items::bakedPotato, 'M', Blocks::red_mushroom, 'B', Items::bowl, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Blocks::melon_block), L"MMM", L"MMM", L"MMM", 'M', Items::melon, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::beetrootSoup), L"OOO", L"OOO", L" B ", 'O', Items::beetroot, 'B', Items::bowl, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::melonSeeds), L"M", 'M', Items::melon, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::pumpkinSeeds, 4), L"M", 'M', Blocks::pumpkin, 0x46);
    recipes->addShapelessRecipe(new ItemInstance(Items::pumpkinPie), Blocks::pumpkin, Items::sugar, Items::egg, 0x46);
    recipes->addShapedRecipe(new ItemInstance(Items::goldenCarrot), L"###", L"#X#", L"###", '#', Items::goldNugget, 'X', Items::carrot, 0x46);
    recipes->addShapelessRecipe(new ItemInstance(Items::fermentedSpiderEye), Items::spiderEye, Blocks::brown_mushroom, Items::sugar, 0x46);
    recipes->addShapelessRecipe(new ItemInstance(Items::blazePowder, 2), Items::blazeRod, 0x46);
    recipes->addShapelessRecipe(new ItemInstance(Items::magmaCream), Items::blazePowder, Items::slimeBall, 0x46);
}
