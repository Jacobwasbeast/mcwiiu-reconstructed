#include "game/item/ShulkerBoxRecipes.hpp"

#include "game/core/ShulkerBoxColoring.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/Blocks.hpp"

void ShulkerBoxRecipes::addRecipes(Recipes* recipes)
{
    recipes->addShapedRecipe(new ItemInstance(Blocks::white_shulker_box), L"-", L"#", L"-",
        '-', Items::shulkerShell, '#', Blocks::chest, 'S');
    recipes->addCustomRecipe(new ShulkerBoxColoring(), Recipes::eRecipeCategory_Crafting);
}
