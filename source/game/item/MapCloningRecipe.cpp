#include "game/item/MapCloningRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

MapCloningRecipe::~MapCloningRecipe()
{
}

bool MapCloningRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    int filledMaps = 0;
    int emptyMaps = 0;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::filledMap)
        {
            ++filledMaps;
        }
        else if (item->getItem() == Items::map)
        {
            ++emptyMaps;
        }
        else
        {
            return false;
        }
    }
    return filledMaps == 1 && emptyMaps > 0;
}

not_null_ptr<ItemInstance> MapCloningRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> result;
    int emptyMaps = 0;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::filledMap)
        {
            result = not_null_ptr<ItemInstance>(new ItemInstance(Items::filledMap,
                                                                  emptyMaps + 1,
                                                                  item->getAuxValue()));
            if (item->hasTag())
            {
                result->setTag(item->getTag()->copy());
            }
        }
        else if (item->getItem() == Items::map)
        {
            ++emptyMaps;
        }
    }
    return result;
}

int MapCloningRecipe::size()
{
    return 9;
}

ItemInstance* MapCloningRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* MapCloningRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > MapCloningRecipe::getRemainingItems(
    boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(),
                                                           ItemInstance::EMPTY);
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty() && item->getItem()->hasCraftingRemainingItem())
        {
            result[i] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
    }
    return result;
}

Recipe::_eGroupType MapCloningRecipe::getGroup()
{
    return eGroupType_None;
}

bool MapCloningRecipe::recipeRequires(int itemId)
{
    return false;
}

void MapCloningRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> MapCloningRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
