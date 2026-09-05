#include "game/item/RepairItemRecipe.hpp"

#include <vector>

#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

RepairItemRecipe::RepairItemRecipe()
    : Recipe()
{
}

RepairItemRecipe::~RepairItemRecipe()
{
}

bool RepairItemRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    std::vector<not_null_ptr<ItemInstance> > items;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty())
        {
            items.push_back(item);
        }
    }

    if (items.size() != 2)
    {
        return false;
    }
    if (items[0]->getItem() != items[1]->getItem())
    {
        return false;
    }
    if (items[0]->getCount() != 1 || items[1]->getCount() != 1)
    {
        return false;
    }
    return items[0]->getItem()->canBeDepleted();
}

not_null_ptr<ItemInstance> RepairItemRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    std::vector<not_null_ptr<ItemInstance> > items;
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (!item->isEmpty())
        {
            items.push_back(item);
        }
    }

    if (items.size() == 2 && items[0]->getItem() == items[1]->getItem() &&
        items[0]->getCount() == 1 && items[1]->getCount() == 1 &&
        items[0]->getItem()->canBeDepleted())
    {
        int maxDamage = items[0]->getItem()->getMaxDamage();
        int damage = maxDamage - items[0]->getDamageValue();
        damage += maxDamage - items[1]->getDamageValue();
        damage += maxDamage * 5 / 100;
        int resultDamage = maxDamage - damage;
        if (resultDamage < 0)
        {
            resultDamage = 0;
        }
        return not_null_ptr<ItemInstance>(new ItemInstance(items[0]->getItem(), 1, resultDamage));
    }
    return ItemInstance::EMPTY;
}

int RepairItemRecipe::size()
{
    return 4;
}

ItemInstance* RepairItemRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* RepairItemRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > RepairItemRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
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

Recipe::_eGroupType RepairItemRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool RepairItemRecipe::recipeRequires(int itemId)
{
    return false;
}

void RepairItemRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> RepairItemRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
