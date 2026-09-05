#include "game/item/Recipes.hpp"

#include <cwchar>
#include <string>
#include <utility>
#include <boost/unordered_map.hpp>

#include "game/core/FoodRecipies.hpp"
#include "game/core/OreRecipies.hpp"
#include "game/core/ToolRecipies.hpp"
#include "game/core/WeaponRecipies.hpp"
#include "game/entity/projectile/TippedArrowRecipe.hpp"
#include "game/item/ArmorRecipes.hpp"
#include "game/item/BannerRecipes.hpp"
#include "game/item/ClothDyeRecipes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/ShapedRecipe.hpp"
#include "game/item/ShapelessRecipe.hpp"
#include "game/item/ShulkerBoxRecipes.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/generation/StructureRecipies.hpp"

Recipes* Recipes::mInstance = NULL;

Recipes::Recipes()
{
    ToolRecipies toolRecipes;
    toolRecipes._init();
    toolRecipes.addRecipes(this);
    WeaponRecipies weaponRecipes;
    weaponRecipes._init();
    weaponRecipes.addRecipes(this);
    ArmorRecipes armorRecipes;
    armorRecipes._init();
    armorRecipes.addRecipes(this);
    FoodRecipies foodRecipes;
    foodRecipes.addRecipes(this);
    OreRecipies oreRecipes;
    oreRecipes._init();
    oreRecipes.addRecipes(this);
    StructureRecipies structureRecipes;
    structureRecipes.addRecipes(this);
    ClothDyeRecipes clothDyeRecipes;
    clothDyeRecipes.addRecipes(this);
    BannerRecipes bannerRecipes;
    bannerRecipes.addRecipes(this);
    ShulkerBoxRecipes shulkerBoxRecipes;
    shulkerBoxRecipes.addRecipes(this);
    TippedArrowRecipe tippedArrowRecipe;
    tippedArrowRecipe.addRecipes(this);
    buildRecipeIngredientsArray();
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, int category, _va_struct* arguments)
{
    std::wstring pattern;
    boost::unordered_map<wchar_t, ItemInstance*> ingredientMap;
    const wchar_t* format = va_arg(arguments->arguments, const wchar_t*);
    int width = 0;
    int height = 0;
    wchar_t ingredientKey = 0;
    int achievementId = -1;
    Recipe::_eGroupType group = Recipe::eGroupType_Miscellaneous;
    Item* resultItem = result->getItem();
    int itemId = Item::getId(resultItem);
    int baseItemType;

    if (static_cast<unsigned int>(itemId - 256) < 768)
    {
        baseItemType = resultItem->getBaseItemType();
    }
    else
    {
        baseItemType = Block::byItem(resultItem)->getBaseItemType();
    }

    for (const wchar_t* type = format; *type != 0; ++type)
    {
        switch (*type)
        {
        case L'a':
        case L's':
        {
            const wchar_t* row = va_arg(arguments->arguments, const wchar_t*);
            width = static_cast<int>(std::wcslen(row));
            ++height;
            pattern.append(row);
            break;
        }
        case L'b':
            baseItemType = va_arg(arguments->arguments, int);
            break;
        case L'c':
            ingredientKey = static_cast<wchar_t>(va_arg(arguments->arguments, int));
            break;
        case L'g':
        {
            wchar_t groupCode = static_cast<wchar_t>(va_arg(arguments->arguments, int));
            if (groupCode >= L'0' && groupCode <= L'5')
            {
                group = static_cast<Recipe::_eGroupType>(groupCode - L'0');
            }
            else
            {
                switch (groupCode)
                {
                case L'S': group = Recipe::eGroupType_Structures; break;
                case L'T': group = Recipe::eGroupType_Tools; break;
                case L'F': group = Recipe::eGroupType_Food; break;
                case L'A': group = Recipe::eGroupType_Armour; break;
                case L'M': group = Recipe::eGroupType_Mechanisms; break;
                case L'V': group = Recipe::eGroupType_Vehicles; break;
                case L'X': group = Recipe::eGroupType_Excluded; break;
                default: group = Recipe::eGroupType_Miscellaneous; break;
                }
            }
            break;
        }
        case L'i':
            ingredientMap.insert(std::make_pair(ingredientKey, new ItemInstance(va_arg(arguments->arguments, Item*))));
            break;
        case L't':
            ingredientMap.insert(std::make_pair(ingredientKey, new ItemInstance(va_arg(arguments->arguments, Block*), 1, -1)));
            break;
        case L'v':
            achievementId = va_arg(arguments->arguments, int);
            break;
        case L'w':
        {
            std::wstring* rows = va_arg(arguments->arguments, std::wstring*);
            while (!rows->empty())
            {
                width = static_cast<int>(rows->size());
                ++height;
                pattern.append(*rows);
                ++rows;
            }
            break;
        }
        case L'z':
            ingredientMap.insert(std::make_pair(ingredientKey, va_arg(arguments->arguments, ItemInstance*)));
            break;
        default:
            break;
        }
    }

    int ingredientCount = width * height;
    ItemInstance** ingredients = new ItemInstance*[ingredientCount];
    for (int index = 0; index < ingredientCount; ++index)
    {
        boost::unordered_map<wchar_t, ItemInstance*>::iterator ingredient = ingredientMap.find(pattern[index]);
        ingredients[index] = ingredient == ingredientMap.end() ? ItemInstance::EMPTY.get() : ingredient->second;
    }

    ShapedRecipe* recipe = new ShapedRecipe(width, height, ingredients, result, group);
    recipe->mAchievementId = achievementId;
    recipe->mBaseItemType = baseItemType;
    mRecipes[category].push_back(recipe);
    return recipe;
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, ...)
{
    _va_struct arguments;
    va_start(arguments.arguments, result);
    Recipe* recipe = addShapedRecipe(result, 0, &arguments);
    va_end(arguments.arguments);
    return recipe;
}

Recipe* Recipes::addShapedRecipe(ItemInstance* result, int category, ...)
{
    _va_struct arguments;
    va_start(arguments.arguments, category);
    Recipe* recipe = addShapedRecipe(result, category, &arguments);
    va_end(arguments.arguments);
    return recipe;
}

void Recipes::addShapelessRecipe(ItemInstance* result, int category, _va_struct* arguments)
{
    std::vector<ItemInstance*>* ingredients = new std::vector<ItemInstance*>();
    const wchar_t* format = va_arg(arguments->arguments, const wchar_t*);
    int achievementId = -1;
    Recipe::_eGroupType group = Recipe::eGroupType_Miscellaneous;
    Item* resultItem = result->getItem();
    int itemId = Item::getId(resultItem);
    int baseItemType;

    if (itemId < 256)
    {
        baseItemType = Block::byItem(resultItem)->getBaseItemType();
    }
    else
    {
        baseItemType = resultItem->getBaseItemType();
    }

    for (const wchar_t* type = format; *type != 0; ++type)
    {
        switch (*type)
        {
        case L'b':
            baseItemType = va_arg(arguments->arguments, int);
            break;
        case L'g':
        {
            wchar_t groupCode = static_cast<wchar_t>(va_arg(arguments->arguments, int));
            if (groupCode >= L'0' && groupCode <= L'5')
            {
                group = static_cast<Recipe::_eGroupType>(groupCode - L'0');
            }
            else
            {
                switch (groupCode)
                {
                case L'S': group = Recipe::eGroupType_Structures; break;
                case L'T': group = Recipe::eGroupType_Tools; break;
                case L'F': group = Recipe::eGroupType_Food; break;
                case L'A': group = Recipe::eGroupType_Armour; break;
                case L'M': group = Recipe::eGroupType_Mechanisms; break;
                case L'V': group = Recipe::eGroupType_Vehicles; break;
                case L'X': group = Recipe::eGroupType_Excluded; break;
                default: group = Recipe::eGroupType_Miscellaneous; break;
                }
            }
            break;
        }
        case L'i':
            ingredients->push_back(new ItemInstance(va_arg(arguments->arguments, Item*)));
            break;
        case L't':
            ingredients->push_back(new ItemInstance(va_arg(arguments->arguments, Block*), 1, -1));
            break;
        case L'v':
            achievementId = va_arg(arguments->arguments, int);
            break;
        case L'z':
            ingredients->push_back(va_arg(arguments->arguments, ItemInstance*)->copy_not_shared());
            break;
        default:
            break;
        }
    }

    ShapelessRecipe* recipe = new ShapelessRecipe(result, ingredients, group);
    recipe->mAchievementId = achievementId;
    recipe->mBaseItemType = baseItemType;
    mRecipes[category].push_back(recipe);
}

void Recipes::addShapelessRecipe(ItemInstance* result, ...)
{
    _va_struct arguments;
    va_start(arguments.arguments, result);
    addShapelessRecipe(result, 0, &arguments);
    va_end(arguments.arguments);
}

void Recipes::addShapelessRecipe(ItemInstance* result, int category, ...)
{
    _va_struct arguments;
    va_start(arguments.arguments, category);
    addShapelessRecipe(result, category, &arguments);
    va_end(arguments.arguments);
}

void Recipes::buildRecipeIngredientsArray()
{
    for (int category = 0; category < 5; ++category)
    {
        unsigned int recipeCount = static_cast<unsigned int>(mRecipes[category].size());
        mRecipeIngredients[category] = new Recipe::INGREDIENTS_REQUIRED[recipeCount];
        for (unsigned int recipeIndex = 0; recipeIndex < recipeCount; ++recipeIndex)
        {
            mRecipes[category][recipeIndex]->recipeRequires(&mRecipeIngredients[category][recipeIndex]);
        }
    }
}

void Recipes::staticCtor()
{
    mInstance = new Recipes();
}

void Recipes::addCustomRecipe(Recipe* recipe, ERecipeCategory category)
{
    mRecipes[category].push_back(recipe);
}

not_null_ptr<ItemInstance> Recipes::getItemFor(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level, Recipe* recipe, ERecipeCategory category)
{
    not_null_ptr<ItemInstance> first = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> second = ItemInstance::EMPTY;
    int itemCount = 0;
    for (unsigned int slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (!item->isEmpty())
        {
            if (itemCount == 0) first = item;
            if (itemCount == 1) second = item;
            ++itemCount;
        }
    }

    if (itemCount == 2 && first->getItem() == second->getItem() && first->getCount() == 1 &&
        second->getCount() == 1 && first->getItem()->canBeDepleted())
    {
        Item* item = first->getItem();
        int damage = item->getMaxDamage() - ((item->getMaxDamage() - first->getDamageValue()) +
                     (item->getMaxDamage() - second->getDamageValue()) + item->getMaxDamage() * 5 / 100);
        if (damage < 0) damage = 0;
        return not_null_ptr<ItemInstance>(new ItemInstance(item, 1, damage));
    }

    if (recipe == NULL)
    {
        std::vector<Recipe*>& recipes = mRecipes[category];
        for (std::vector<Recipe*>::iterator current = recipes.begin(); current != recipes.end(); ++current)
        {
            if ((*current)->matches(craftingContainer, level)) return (*current)->assemble(craftingContainer);
        }
    }
    else if (recipe->matches(craftingContainer, level))
    {
        return recipe->assemble(craftingContainer);
    }
    return ItemInstance::EMPTY;
}

eAward Recipes::hasAchievementAttachedToRecipe(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level, Recipe* recipe, ERecipeCategory category)
{
    not_null_ptr<ItemInstance> first = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> second = ItemInstance::EMPTY;
    int itemCount = 0;
    for (unsigned int slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (!item->isEmpty())
        {
            if (itemCount == 0) first = item;
            if (itemCount == 1) second = item;
            ++itemCount;
        }
    }

    if (itemCount == 2 && first->getItem() == second->getItem() && first->getCount() == 1 &&
        second->getCount() == 1 && first->getItem()->canBeDepleted()) return eAward_None;

    if (recipe == NULL)
    {
        std::vector<Recipe*>& recipes = mRecipes[category];
        for (std::vector<Recipe*>::iterator current = recipes.begin(); current != recipes.end(); ++current)
        {
            if ((*current)->matches(craftingContainer, level)) return static_cast<eAward>((*current)->mAchievementId);
        }
    }
    else if (recipe->matches(craftingContainer, level))
    {
        return static_cast<eAward>(recipe->mAchievementId);
    }
    return eAward_None;
}

NonNullList<not_null_ptr<ItemInstance> > Recipes::getRemainingItemsFor(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level, ERecipeCategory category)
{
    std::vector<Recipe*>& recipes = mRecipes[category];
    for (std::vector<Recipe*>::iterator current = recipes.begin(); current != recipes.end(); ++current)
    {
        if ((*current)->matches(craftingContainer, level)) return (*current)->getRemainingItems(craftingContainer);
    }

    NonNullList<not_null_ptr<ItemInstance> > remaining = NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (unsigned int slot = 0; slot < remaining.size(); ++slot)
    {
        remaining[slot] = craftingContainer->getItem(slot);
    }
    return remaining;
}

std::vector<Recipe*>* Recipes::getRecipes(ERecipeCategory category)
{
    return &mRecipes[category];
}

not_null_ptr<ItemInstance> Recipes::getItemForRecipe(Recipe* recipe)
{
    return recipe->assemble(boost::shared_ptr<CraftingContainer>());
}

Recipe::INGREDIENTS_REQUIRED* Recipes::getRecipeIngredientsArray(ERecipeCategory category)
{
    return mRecipeIngredients[category];
}
