#include "game/item/BannerRecipes.hpp"

#include <string>

#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"

class BannerBlockEntity
{
public:
    static int getPatternCount(const not_null_ptr<ItemInstance>& item);
};

BannerRecipes::BannerRecipes()
{
}

ItemInstance* BannerRecipes::createOutputItem(int color, const std::wstring& pattern)
{
    ItemInstance* result = new ItemInstance(Items::banner, 1, color);
    CompoundTag* tag = static_cast<CompoundTag*>(result->getOrCreateTagElement(L"BlockEntityTag"));
    ListTag* patterns;
    if (!tag->contains(L"Patterns", 9))
    {
        patterns = new ListTag(10);
        tag->put(L"Patterns", patterns);
    }
    else
    {
        patterns = tag->getList(L"Patterns", 10);
    }
    CompoundTag* entry = new CompoundTag();
    entry->putString(L"Pattern", pattern);
    entry->putInt(L"Color", color);
    patterns->add(entry);
    return result;
}

void BannerRecipes::addRecipes(Recipes* recipes)
{
    recipes->addCustomRecipe(new BannerDuplicateRecipe(), Recipes::eRecipeCategory_Crafting);
    recipes->addCustomRecipe(new BannerAddPatternRecipe(), Recipes::eRecipeCategory_Crafting);
}

BannerRecipes::BannerAddPatternRecipe::~BannerAddPatternRecipe()
{
}

unsigned long long BannerRecipes::BannerAddPatternRecipe::matchPatterns(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    unsigned int patterned = 0;
    unsigned int dyes = 0;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
            continue;
        if (item->getItem() == Items::banner)
        {
            if (BannerBlockEntity::getPatternCount(item) != 0)
                return 0;
            ++patterned;
        }
        else if (item->getItem() == Items::dye)
        {
            ++dyes;
        }
        else
        {
            return 0;
        }
    }
    return patterned == 1 && dyes == 1 ? 1 : 0;
}

bool BannerRecipes::BannerAddPatternRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    return matchPatterns(craftingContainer) != 0;
}

not_null_ptr<ItemInstance> BannerRecipes::BannerAddPatternRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    not_null_ptr<ItemInstance> result;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem() == Items::banner)
        {
            result = item->copy();
            result->setCount(1);
            break;
        }
    }
    return result;
}

int BannerRecipes::BannerAddPatternRecipe::size()
{
    return 10;
}

ItemInstance* BannerRecipes::BannerAddPatternRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* BannerRecipes::BannerAddPatternRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > BannerRecipes::BannerAddPatternRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem()->hasCraftingRemainingItem())
            result[i] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
    }
    return result;
}

Recipe::_eGroupType BannerRecipes::BannerAddPatternRecipe::getGroup()
{
    return eGroupType_Miscellaneous;
}

bool BannerRecipes::BannerAddPatternRecipe::recipeRequires(int itemId)
{
    return false;
}

void BannerRecipes::BannerAddPatternRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> BannerRecipes::BannerAddPatternRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}

BannerRecipes::BannerDuplicateRecipe::~BannerDuplicateRecipe()
{
}

bool BannerRecipes::BannerDuplicateRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    unsigned int i;
    unsigned int plain = 0;
    unsigned int patterned = 0;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
            continue;
        if (item->getItem() != Items::banner)
            return false;
        if (BannerBlockEntity::getPatternCount(item) == 0)
            ++plain;
        else
            ++patterned;
    }
    return plain == 1 && patterned == 1;
}

not_null_ptr<ItemInstance> BannerRecipes::BannerDuplicateRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    not_null_ptr<ItemInstance> result;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && BannerBlockEntity::getPatternCount(item) != 0)
        {
            result = item->copy();
            result->setCount(1);
            break;
        }
    }
    return result;
}

int BannerRecipes::BannerDuplicateRecipe::size()
{
    return 2;
}

ItemInstance* BannerRecipes::BannerDuplicateRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* BannerRecipes::BannerDuplicateRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > BannerRecipes::BannerDuplicateRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result = NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem() == Items::banner && BannerBlockEntity::getPatternCount(item) != 0)
        {
            not_null_ptr<ItemInstance> copy = item->copy();
            copy->setCount(1);
            result[i] = copy;
        }
    }
    return result;
}

Recipe::_eGroupType BannerRecipes::BannerDuplicateRecipe::getGroup()
{
    return eGroupType_Miscellaneous;
}

bool BannerRecipes::BannerDuplicateRecipe::recipeRequires(int itemId)
{
    return false;
}

void BannerRecipes::BannerDuplicateRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> BannerRecipes::BannerDuplicateRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
