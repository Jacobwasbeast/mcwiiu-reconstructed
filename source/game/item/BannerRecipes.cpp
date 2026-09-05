#include "game/item/BannerRecipes.hpp"

#include <string>
#include <vector>

#include "game/core/BannerPattern.hpp"
#include "game/core/DyeColor.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Blocks.hpp"

struct BannerPatternDefinition
{
    const wchar_t* name;
    const wchar_t* rows[3];
};

static const BannerPatternDefinition bannerPatternDefinitions[] =
{
    { L"bo", { L"###", L"#*#", L"###" } },
    { L"cr", { L" # ", L"###", L" # " } },
    { L"rd", { L"#  ", L" ##", L"  #" } },
    { L"vh", { L"#*#", L"#*#", L"#*#" } },
    { L"mr", { L"#*#", L"###", L"#*#" } },
    { L"ss", { L"###", L"#* ", L"###" } },
    { L"bl", { L"#* ", L"#  ", L"###" } },
    { L"br", { L" *#", L"  #", L"###" } },
    { L"tl", { L"###", L"#  ", L"#* " } },
    { L"tr", { L"###", L"  #", L" *#" } },
    { L"sc", { L"###", L"#*#", L"# #" } },
    { L"bs", { L"###", L" *#", L"###" } },
    { L"cs", { L"# #", L"#*#", L"# #" } },
    { L"ls", { L"#  ", L"#*#", L"###" } },
    { L"ms", { L"###", L"#*#", L"###" } },
    { L"rs", { L"  #", L"#*#", L"###" } },
    { L"ts", { L"###", L" *#", L"  #" } },
    { L"bt", { L"# #", L"#*#", L"###" } },
    { L"tt", { L"###", L"#*#", L"# #" } },
    { L"fl", { L" # ", L"###", L" * " } },
    { L"cre", { L"###", L"#*#", L"###" } },
    { L"gra", { L"#  ", L" ##", L"  #" } },
    { L"bts", { L"###", L" # ", L" * " } },
    { L"tts", { L" * ", L" # ", L"###" } },
    { L"hhb", { L"###", L"###", L"##*" } },
    { L"vhr", { L"#*#", L"#*#", L"#*#" } },
    { L"dls", { L"#  ", L" ##", L" #*" } },
    { L"drs", { L"  #", L"## ", L"*# " } },
    { L"bts2", { L"###", L"# #", L"#*#" } },
    { L"tts2", { L"#*#", L"# #", L"###" } },
    { L"bo2", { L"###", L"#*#", L"###" } },
    { L"mc2", { L"# #", L" * ", L"# #" } }
};

static const BannerPatternDefinition* getPatternDefinition(const BannerPattern* pattern)
{
    std::wstring name = pattern->getHashname();
    unsigned int i;
    for (i = 0; i < sizeof(bannerPatternDefinitions) / sizeof(bannerPatternDefinitions[0]); ++i)
    {
        if (name == bannerPatternDefinitions[i].name)
            return &bannerPatternDefinitions[i];
    }
    return NULL;
}

static ItemInstance* makeBanner(const DyeColor* color, ListTag* patterns)
{
    ItemInstance* banner = new ItemInstance(Items::banner, 1, color->getItemData());
    if (patterns != NULL && !patterns->isEmpty())
    {
        CompoundTag* tag = static_cast<CompoundTag*>(banner->getOrCreateTagElement(L"BlockEntityTag"));
        tag->put(L"Patterns", patterns->copy());
    }
    return banner;
}

static void getBannerPatterns(std::vector<const BannerPattern*>& patterns)
{
    static const wchar_t* names[] =
    {
        L"b", L"bo", L"mc", L"cr", L"ld", L"rd", L"hh", L"vh", L"mr", L"ss",
        L"bl", L"br", L"tl", L"tr", L"sc", L"bs", L"cs", L"ls", L"ms", L"rs",
        L"ts", L"bt", L"tt", L"fl", L"cre", L"sku", L"moj", L"gra", L"bts", L"tts",
        L"hhb", L"vhr", L"dls", L"drs", L"bts2", L"tts2", L"bo2", L"mc2"
    };
    unsigned int i;
    for (i = 0; i < sizeof(names) / sizeof(names[0]); ++i)
    {
        const BannerPattern* pattern = BannerPattern::byHash(names[i]);
        if (pattern != NULL)
            patterns.push_back(pattern);
    }
}

BannerRecipes::BannerRecipes()
{
}

ItemInstance* BannerRecipes::createOutputItem(int color, const std::wstring& pattern)
{
    ItemInstance* result = new ItemInstance(Items::banner, 1, DyeColor::WHITE->getItemData());
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
    const DyeColor* colors[] =
    {
        DyeColor::WHITE, DyeColor::ORANGE, DyeColor::MAGENTA, DyeColor::LIGHT_BLUE,
        DyeColor::YELLOW, DyeColor::LIME, DyeColor::PINK, DyeColor::GRAY,
        DyeColor::SILVER, DyeColor::CYAN, DyeColor::PURPLE, DyeColor::BLUE,
        DyeColor::BROWN, DyeColor::GREEN, DyeColor::RED, DyeColor::BLACK
    };
    unsigned int i;
    for (i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
    {
        ItemInstance* result = makeBanner(colors[i], NULL);
        BannerBlockEntity::addTags(result, colors[i]->getItemData(), NULL);
        ItemInstance* wool = new ItemInstance(Blocks::wool, 1, colors[i]->getBlockData());
        Recipe* recipe = recipes->addShapedRecipe(result, L"sssczcig", L"###", L"###", L" # ", L'#', wool, L'|', Items::stick, 0x44);
        recipe->mDisplayInSimpleCrafting = false;

        result = makeBanner(colors[i], NULL);
        BannerBlockEntity::addTags(result, colors[i]->getItemData(), NULL);
        wool = new ItemInstance(Blocks::wool, 1, colors[i]->getBlockData());
        const BannerPattern* base = BannerPattern::byHash(L"b");
        recipes->addShapedRecipe(result, 1, L"sssczcibg", L"###", L"###", L" # ", L'#', wool, L'|', Items::stick, base->getAtlasIndex(), base->getCraftingMenuTab());

        result = makeBanner(colors[i], NULL);
        BannerBlockEntity::addTags(result, colors[i]->getItemData(), NULL);
        recipes->addShapedRecipe(result, 1, L"sczbg", L"##", L'#', makeBanner(colors[i], NULL), 999, base->getCraftingMenuTab());
    }

    BannerDuplicateRecipe* duplicateRecipe = new BannerDuplicateRecipe();
    duplicateRecipe->mDisplayInSimpleCrafting = false;
    recipes->addCustomRecipe(duplicateRecipe, Recipes::eRecipeCategory_Crafting);

    BannerAddPatternRecipe* addPatternRecipe = new BannerAddPatternRecipe();
    addPatternRecipe->mDisplayInSimpleCrafting = false;
    recipes->addCustomRecipe(addPatternRecipe, Recipes::eRecipeCategory_Crafting);

    std::vector<const BannerPattern*> patterns;
    getBannerPatterns(patterns);
    std::vector<const BannerPattern*>::const_iterator patternIt;
    for (patternIt = patterns.begin(); patternIt != patterns.end(); ++patternIt)
    {
        const BannerPattern* pattern = *patternIt;
        if (!pattern->hasPattern())
            continue;
        std::wstring hashname = pattern->getHashname();
        if (pattern->hasPatternItem())
        {
            for (i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
            {
                ItemInstance* result = createOutputItem(colors[i]->getItemData(), hashname);
                recipes->addShapelessRecipe(result, 1, L"zzzbg", makeBanner(DyeColor::WHITE, NULL), pattern->getPatternItem(), new ItemInstance(Items::dye, 1, colors[i]->getItemData()), pattern->getAtlasIndex(), pattern->getCraftingMenuTab());
            }
        }
        else
        {
            const BannerPatternDefinition* definition = getPatternDefinition(pattern);
            if (definition == NULL)
                continue;
            for (i = 0; i < sizeof(colors) / sizeof(colors[0]); ++i)
            {
                ItemInstance* result = createOutputItem(colors[i]->getItemData(), hashname);
                recipes->addShapedRecipe(result, 1, L"sssczczbg", definition->rows[0], definition->rows[1], definition->rows[2], L'#', new ItemInstance(Items::dye, 1, colors[i]->getItemData()), L'*', makeBanner(DyeColor::WHITE, NULL), pattern->getAtlasIndex(), pattern->getCraftingMenuTab());
            }
        }
    }
}

BannerRecipes::BannerAddPatternRecipe::~BannerAddPatternRecipe()
{
}

const BannerPattern* BannerRecipes::BannerAddPatternRecipe::matchPatterns(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    std::vector<const BannerPattern*> patterns;
    getBannerPatterns(patterns);
    std::vector<const BannerPattern*>::const_iterator patternIt;
    for (patternIt = patterns.begin(); patternIt != patterns.end(); ++patternIt)
    {
        const BannerPattern* pattern = *patternIt;
        if (!pattern->hasPattern())
            continue;
        bool matches = true;
        if (pattern->hasPatternItem())
        {
            bool foundPatternItem = false;
            bool foundDye = false;
            unsigned int i;
            for (i = 0; i < craftingContainer->getContainerSize(); ++i)
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
            if (matches && foundPatternItem && foundDye)
                return pattern;
        }
        else
        {
            const BannerPatternDefinition* definition = getPatternDefinition(pattern);
            if (definition == NULL || craftingContainer->getContainerSize() != 9)
                continue;
            int dyeColor = -1;
            unsigned int i;
            for (i = 0; i < craftingContainer->getContainerSize() && matches; ++i)
            {
                not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
                wchar_t expected = definition->rows[i / 3][i % 3];
                if (item->isEmpty() || item->getItem() == Items::banner)
                {
                    if (expected != L' ' && expected != L'*')
                        matches = false;
                }
                else
                {
                    if (item->getItem() != Items::dye || expected == L' ' || expected == L'*')
                    {
                        matches = false;
                    }
                    else if (dyeColor != -1 && dyeColor != item->getAuxValue())
                    {
                        matches = false;
                    }
                    else
                    {
                        dyeColor = item->getAuxValue();
                    }
                }
            }
            if (matches)
                return pattern;
        }
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
