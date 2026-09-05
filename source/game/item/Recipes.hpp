#ifndef GAME_ITEM_RECIPES_HPP
#define GAME_ITEM_RECIPES_HPP

#include <vector>
#include <cstdarg>
#include <boost/shared_ptr.hpp>

#include "game/item/Recipe.hpp"

class CraftingContainer;
class ItemInstance;
class Level;
struct _va_struct
{
    va_list arguments;
};

class Recipes
{
public:
    enum ERecipeCategory
    {
        eRecipeCategory_Crafting = 0,
        eRecipeCategory_Stonecutter = 1,
        eRecipeCategory_Smithing = 2,
        eRecipeCategory_Unknown = 3,
        eRecipeCategory_Custom = 4
    };

    Recipes();

    Recipe* addShapedRecipe(ItemInstance* result, int category, _va_struct* arguments);
    Recipe* addShapedRecipe(ItemInstance* result, ...);
    Recipe* addShapedRecipe(ItemInstance* result, int category, ...);
    Recipe* addShapelessRecipe(ItemInstance* result, int category, _va_struct* arguments);
    Recipe* addShapelessRecipe(ItemInstance* result, ...);
    Recipe* addShapelessRecipe(ItemInstance* result, int category, ...);
    void buildRecipeIngredientsArray();
    static void staticCtor();
    void addCustomRecipe(Recipe* recipe, ERecipeCategory category);
    not_null_ptr<ItemInstance> getItemFor(boost::shared_ptr<CraftingContainer> craftingContainer,
                                          Level* level,
                                          Recipe* recipe,
                                          ERecipeCategory category);
    bool hasAchievementAttachedToRecipe(boost::shared_ptr<CraftingContainer> craftingContainer,
                                        Level* level,
                                        Recipe* recipe,
                                        ERecipeCategory category);
    NonNullList<not_null_ptr<ItemInstance> > getRemainingItemsFor(boost::shared_ptr<CraftingContainer> craftingContainer,
                                                                  Level* level,
                                                                  ERecipeCategory category);
    std::vector<Recipe*>& getRecipes(ERecipeCategory category);
    not_null_ptr<ItemInstance> getItemForRecipe(Recipe* recipe);
    Recipe::INGREDIENTS_REQUIRED* getRecipeIngredientsArray(ERecipeCategory category);
    void _init();

    static Recipes* getInstance();

private:
    std::vector<Recipe*> mRecipes[5];
    Recipe::INGREDIENTS_REQUIRED* mRecipeIngredients[5];
};

#endif
