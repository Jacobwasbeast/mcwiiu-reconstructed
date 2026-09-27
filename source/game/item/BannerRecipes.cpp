#include "game/item/BannerRecipes.hpp"

#include <string>
#include <vector>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"

BannerRecipes::BannerDuplicateRecipe* BannerRecipes::mDuplicateRecipe = NULL;
BannerRecipes::BannerAddPatternRecipe* BannerRecipes::mAddPatternRecipe = NULL;

BannerRecipes::BannerRecipes()
{
}

ItemInstance* BannerRecipes::createOutputItem(int color, const std::wstring& pattern)
{
    ItemInstance* result = new ItemInstance(Items::banner, 1, DyeColor::WHITE->getItemData());
    CompoundTag* tag = static_cast<CompoundTag*>(result->getOrCreateTagElement(L"BlockEntityTag"));
    ListTag* patterns;
    if (!tag->contains(L"Patterns", Tag::TAG_LIST))
    {
        patterns = new ListTag(10);
        tag->put(L"Patterns", patterns);
    }
    else
    {
        patterns = tag->getList(L"Patterns", Tag::TAG_COMPOUND);
    }
    CompoundTag* entry = new CompoundTag();
    entry->putString(L"Pattern", pattern);
    entry->putInt(L"Color", color);
    patterns->add(entry);
    return result;
}

void BannerRecipes::addRecipes(Recipes* recipes)
{
    for (std::vector<const DyeColor*>::const_iterator colorIt = DyeColor::COLORS.begin(); colorIt != DyeColor::COLORS.end();
         ++colorIt)
    {
        const DyeColor* color = *colorIt;
        ItemInstance* result = BannerItem::makeBanner(color, NULL);
        BannerBlockEntity::addTags(result, color->getItemData(), NULL);
        Recipe* recipe = recipes->addShapedRecipe(result, L"sssczcig", L"###", L"###", L" | ", L'#',
                                                  new ItemInstance(Blocks::wool, 1, color->getBlockData()), L'|',
                                                  Items::stick, L'D');
        recipe->mDisplayInSimpleCrafting = false;

        result = BannerItem::makeBanner(color, NULL);
        BannerBlockEntity::addTags(result, color->getItemData(), NULL);
        recipes->addShapedRecipe(result, Recipes::eRecipeCategory_Banners, L"sssczcibg", L"###", L"###", L" | ", L'#',
                                 new ItemInstance(Blocks::wool, 1, color->getBlockData()), L'|', Items::stick,
                                 BannerPattern::BASE->getAtlasIndex(), BannerPattern::BASE->getCraftingMenuTab());

        result = BannerItem::makeBanner(color, NULL);
        BannerBlockEntity::addTags(result, color->getItemData(), NULL);
        recipes->addShapedRecipe(result, Recipes::eRecipeCategory_Banners, L"sczbg", L"##", L'#',
                                 BannerItem::makeBanner(color, NULL), 999, BannerPattern::BASE->getCraftingMenuTab());
    }

    mDuplicateRecipe = new BannerDuplicateRecipe();
    mDuplicateRecipe->mDisplayInSimpleCrafting = false;
    recipes->addCustomRecipe(mDuplicateRecipe, Recipes::eRecipeCategory_Crafting);

    mAddPatternRecipe = new BannerAddPatternRecipe();
    mAddPatternRecipe->mDisplayInSimpleCrafting = false;
    recipes->addCustomRecipe(mAddPatternRecipe, Recipes::eRecipeCategory_Crafting);

    for (std::vector<const BannerPattern*>::const_iterator patternIt = BannerPattern::PATTERNS.begin();
         patternIt != BannerPattern::PATTERNS.end(); ++patternIt)
    {
        const BannerPattern* pattern = *patternIt;
        if (!pattern->hasPattern())
            continue;

        std::wstring hashname = pattern->getHashname();
        if (!pattern->hasPatternItem())
        {
            for (int dye = 0; dye < 16; ++dye)
            {
                arrayWithLength<std::wstring> rows = pattern->getPatterns();
                recipes->addShapedRecipe(createOutputItem(dye, hashname), Recipes::eRecipeCategory_Banners, L"sssczczbg",
                                         rows.data[0].c_str(), rows.data[1].c_str(), rows.data[2].c_str(), L'#',
                                         new ItemInstance(Items::dye, 1, dye), L'*',
                                         BannerItem::makeBanner(DyeColor::WHITE, NULL), pattern->getAtlasIndex(),
                                         pattern->getCraftingMenuTab());
            }
        }
        else
        {
            for (int dye = 0; dye < 16; ++dye)
            {
                recipes->addShapelessRecipe(createOutputItem(dye, hashname), Recipes::eRecipeCategory_Banners, L"zzzbg",
                                            BannerItem::makeBanner(DyeColor::WHITE, NULL), pattern->getPatternItem(),
                                            new ItemInstance(Items::dye, 1, dye), pattern->getAtlasIndex(),
                                            pattern->getCraftingMenuTab());
            }
        }
    }
}

BannerRecipes::BannerAddPatternRecipe::~BannerAddPatternRecipe()
{
}

const BannerPattern* BannerRecipes::BannerAddPatternRecipe::matchPatterns(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    for (std::vector<const BannerPattern*>::const_iterator patternIt = BannerPattern::PATTERNS.begin();
         patternIt != BannerPattern::PATTERNS.end(); ++patternIt)
    {
        const BannerPattern* pattern = *patternIt;
        if (!pattern->hasPattern())
            continue;

        bool matches = true;
        if (pattern->hasPatternItem())
        {
            bool foundPatternItem = false;
            bool foundDye = false;
            for (unsigned int i = 0; i < craftingContainer->getContainerSize() && matches; ++i)
            {
                not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
                if (item->isEmpty() || item->getItem() == Items::banner)
                    continue;

                if (item->getItem() == Items::dye)
                {
                    if (foundDye)
                    {
                        matches = false;
                        break;
                    }
                    foundDye = true;
                }
                else
                {
                    if (foundPatternItem || !item->sameItem_not_shared(pattern->getPatternItem()))
                    {
                        matches = false;
                        break;
                    }
                    foundPatternItem = true;
                }
            }
            if (!foundPatternItem || !foundDye)
                continue;
        }
        else
        {
            arrayWithLength<std::wstring> rows = pattern->getPatterns();
            if (craftingContainer->getContainerSize() != rows.length * pattern->getPatterns().data[0].size())
                continue;

            int color = -1;
            bool rejected = false;
            for (unsigned int i = 0; i < craftingContainer->getContainerSize() && matches; ++i)
            {
                int row = i / 3;
                int column = i % 3;
                not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
                if (!item->isEmpty() && item->getItem() != Items::banner)
                {
                    if (item->getItem() != Items::dye)
                        matches = false;
                    if (color != -1 && color != item->getAuxValue())
                    {
                        rejected = true;
                        break;
                    }
                    wchar_t cell = pattern->getPatterns().data[row].at(column);
                    if (cell == L' ' || cell == L'*')
                    {
                        rejected = true;
                        break;
                    }
                    color = item->getAuxValue();
                }
                else
                {
                    wchar_t cell = pattern->getPatterns().data[row].at(column);
                    if (cell != L' ' && cell != L'*')
                        matches = false;
                }
            }
            if (rejected)
                continue;
        }

        if (matches)
            return pattern;
    }
    return NULL;
}

bool BannerRecipes::BannerAddPatternRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    bool foundBanner = false;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem() == Items::banner)
        {
            if (foundBanner || BannerBlockEntity::getPatternCount(item) > 5)
                return false;
            foundBanner = true;
        }
    }
    return foundBanner && matchPatterns(craftingContainer) != NULL;
}

not_null_ptr<ItemInstance> BannerRecipes::BannerAddPatternRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    const BannerPattern* pattern = matchPatterns(craftingContainer);
    if (pattern == NULL)
        return ItemInstance::EMPTY;
    unsigned int i;
    int color = 0;
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem() == Items::banner)
        {
            result = item->copy();
            result->setCount(1);
        }
        else if (!item->isEmpty() && item->getItem() == Items::dye)
            color = item->getAuxValue();
    }
    CompoundTag* tag = static_cast<CompoundTag*>(result->getOrCreateTagElement(L"BlockEntityTag"));
    ListTag* patterns;
    if (!tag->contains(L"Patterns", Tag::TAG_LIST))
    {
        patterns = new ListTag(10);
        tag->put(L"Patterns", patterns);
    }
    else
    {
        patterns = tag->getList(L"Patterns", Tag::TAG_COMPOUND);
    }
    CompoundTag* entry = new CompoundTag();
    entry->putString(L"Pattern", pattern->getHashname());
    entry->putInt(L"Color", color);
    patterns->add(entry);
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
    not_null_ptr<ItemInstance> patternedBanner = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> plainBanner = ItemInstance::EMPTY;
    for (unsigned int i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
            continue;

        if (item->getItem() != Items::banner)
            return false;
        if (!patternedBanner->isEmpty() && !plainBanner->isEmpty())
            return false;

        const DyeColor* baseColor = BannerBlockEntity::getBaseColor(item);
        bool hasPatterns = BannerBlockEntity::getPatternCount(item) > 0;
        if (!patternedBanner->isEmpty())
        {
            if (hasPatterns)
                return false;
            if (baseColor != BannerBlockEntity::getBaseColor(patternedBanner))
                return false;
            plainBanner = item;
        }
        else if (!plainBanner->isEmpty())
        {
            if (!hasPatterns)
                return false;
            if (baseColor != BannerBlockEntity::getBaseColor(plainBanner))
                return false;
            patternedBanner = item;
        }
        else if (hasPatterns)
        {
            patternedBanner = item;
        }
        else
        {
            plainBanner = item;
        }
    }
    return !patternedBanner->isEmpty() && !plainBanner->isEmpty();
}

not_null_ptr<ItemInstance> BannerRecipes::BannerDuplicateRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    for (unsigned int i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && BannerBlockEntity::getPatternCount(item) > 0)
        {
            not_null_ptr<ItemInstance> result = item->copy();
            result->setCount(1);
            return result;
        }
    }
    return ItemInstance::EMPTY;
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
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    for (unsigned int i = 0; i < result.size(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
            continue;

        if (item->getItem()->hasCraftingRemainingItem())
        {
            result[i] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
        else if (item->hasTag() && BannerBlockEntity::getPatternCount(item) > 0)
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
