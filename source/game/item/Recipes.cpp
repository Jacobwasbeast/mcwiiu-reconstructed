#include "game/item/Recipes.hpp"

#include <cstdarg>

#include "game/item/CraftingContainer.hpp"
#include "game/item/ItemInstance.hpp"

class ShapedRecipe;
class ShapelessRecipe;

extern Recipe* makeShapedRecipe(int width, int height, ItemInstance** ingredients, ItemInstance* result, Recipe::_eGroupType group);
extern Recipe* makeShapelessRecipe(ItemInstance* result, std::vector<ItemInstance*>* ingredients, Recipe::_eGroupType group);

static Recipes* sRecipes = NULL;

Recipes::Recipes()
    : mRecipes(),
      mRecipeIngredients()
{
    _init();
}

void Recipes::_init()
{
    int i;
    for (i = 0; i < 5; ++i)
    {
        mRecipeIngredients[i] = NULL;
    }
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, int category, _va_struct* arguments)
{
    std::wstring shape;
    std::vector<ItemInstance*> ingredients;
    ItemInstance* ingredient[64];
    int width = 0;
    int height = 0;
    int count = 0;
    Recipe::_eGroupType group = Recipe::eGroupType_None;
    wchar_t* row;
    int i;

    row = va_arg(arguments->arguments, wchar_t*);
    while (row != NULL)
    {
        shape.append(row);
        ++height;
        row = va_arg(arguments->arguments, wchar_t*);
        if (row == NULL)
        {
            break;
        }
    }
    if (height > 0)
    {
        width = static_cast<int>(shape.size()) / height;
    }
    for (i = 0; i < width * height && i < 64; ++i)
    {
        ingredient[i] = va_arg(arguments->arguments, ItemInstance*);
        if (ingredient[i] != NULL)
        {
            ingredients.push_back(ingredient[i]);
            ++count;
        }
    }
    if (count == 0)
    {
        return NULL;
    }
    for (i = 0; i < count; ++i)
    {
        ingredient[i] = ingredients[i];
    }
    return makeShapedRecipe(width, height, ingredient, result, group);
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, ...)
{
    _va_struct arguments;
    Recipe* recipe;
    va_start(arguments.arguments, result);
    recipe = addShapedRecipe(result, static_cast<int>(eRecipeCategory_Crafting), &arguments);
    va_end(arguments.arguments);
    if (recipe != NULL)
    {
        addCustomRecipe(recipe, eRecipeCategory_Crafting);
    }
    return recipe;
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, int category, ...)
{
    _va_struct arguments;
    Recipe* recipe;
    va_start(arguments.arguments, category);
    recipe = addShapedRecipe(result, category, &arguments);
    va_end(arguments.arguments);
    if (recipe != NULL)
    {
        addCustomRecipe(recipe, static_cast<ERecipeCategory>(category));
    }
    return recipe;
}

Recipe* Recipes::addShapelessRecipe(ItemInstance* result, int category, _va_struct* arguments)
{
    std::vector<ItemInstance*> ingredients;
    Recipe::_eGroupType group = Recipe::eGroupType_None;
    unsigned short type;
    ItemInstance* item;

    type = va_arg(arguments->arguments, unsigned short);
    while (type != 0)
    {
        item = va_arg(arguments->arguments, ItemInstance*);
        if (item != NULL)
        {
            ingredients.push_back(item);
        }
        type = va_arg(arguments->arguments, unsigned short);
    }
    if (!ingredients.empty())
    {
        Recipe* recipe = makeShapelessRecipe(result, &ingredients, group);
        addCustomRecipe(recipe, static_cast<ERecipeCategory>(category));
        return recipe;
    }
    return NULL;
}

Recipe* Recipes::addShapelessRecipe(ItemInstance* result, ...)
{
    _va_struct arguments;
    Recipe* recipe;
    va_start(arguments.arguments, result);
    recipe = addShapelessRecipe(result, static_cast<int>(eRecipeCategory_Crafting), &arguments);
    va_end(arguments.arguments);
    return recipe;
}

Recipe* Recipes::addShapelessRecipe(ItemInstance* result, int category, ...)
{
    _va_struct arguments;
    Recipe* recipe;
    va_start(arguments.arguments, category);
    recipe = addShapelessRecipe(result, category, &arguments);
    va_end(arguments.arguments);
    return recipe;
}

void Recipes::buildRecipeIngredientsArray()
{
    int category;
    unsigned int count;
    unsigned int i;

    for (category = 0; category < 5; ++category)
    {
        count = static_cast<unsigned int>(mRecipes[category].size());
        mRecipeIngredients[category] = count == 0 ? NULL : new Recipe::INGREDIENTS_REQUIRED[count];
        for (i = 0; i < count; ++i)
        {
            mRecipes[category][i]->recipeRequires(&mRecipeIngredients[category][i]);
        }
    }
}

void Recipes::staticCtor()
{
    sRecipes = new Recipes();
}

Recipes* Recipes::getInstance()
{
    return sRecipes;
}

void Recipes::addCustomRecipe(Recipe* recipe, ERecipeCategory category)
{
    mRecipes[category].push_back(recipe);
}

not_null_ptr<ItemInstance> Recipes::getItemFor(boost::shared_ptr<CraftingContainer> craftingContainer,
                                               Level* level,
                                               Recipe* recipe,
                                               ERecipeCategory category)
{
    unsigned int i;
    if (recipe != NULL)
    {
        if (recipe->matches(craftingContainer, level))
        {
            return recipe->assemble(craftingContainer);
        }
        return not_null_ptr<ItemInstance>();
    }
    for (i = 0; i < mRecipes[category].size(); ++i)
    {
        if (mRecipes[category][i]->matches(craftingContainer, level))
        {
            return mRecipes[category][i]->assemble(craftingContainer);
        }
    }
    return not_null_ptr<ItemInstance>();
}

bool Recipes::hasAchievementAttachedToRecipe(boost::shared_ptr<CraftingContainer> craftingContainer,
                                             Level* level,
                                             Recipe* recipe,
                                             ERecipeCategory category)
{
    unsigned int i;
    if (recipe != NULL)
    {
        return recipe->matches(craftingContainer, level) && recipe->displayInSimpleCrafting();
    }
    for (i = 0; i < mRecipes[category].size(); ++i)
    {
        if (mRecipes[category][i]->matches(craftingContainer, level) && mRecipes[category][i]->displayInSimpleCrafting())
        {
            return true;
        }
    }
    return false;
}

NonNullList<not_null_ptr<ItemInstance> > Recipes::getRemainingItemsFor(boost::shared_ptr<CraftingContainer> craftingContainer,
                                                                       Level* level,
                                                                       ERecipeCategory category)
{
    unsigned int i;
    for (i = 0; i < mRecipes[category].size(); ++i)
    {
        if (mRecipes[category][i]->matches(craftingContainer, level))
        {
            return mRecipes[category][i]->getRemainingItems(craftingContainer);
        }
    }
    return NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
}

std::vector<Recipe*>& Recipes::getRecipes(ERecipeCategory category)
{
    return mRecipes[category];
}

not_null_ptr<ItemInstance> Recipes::getItemForRecipe(Recipe* recipe)
{
    if (recipe == NULL)
    {
        return not_null_ptr<ItemInstance>();
    }
    return recipe->getResultItem();
}

Recipe::INGREDIENTS_REQUIRED* Recipes::getRecipeIngredientsArray(ERecipeCategory category)
{
    return mRecipeIngredients[category];
}
