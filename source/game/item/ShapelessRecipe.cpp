#include "game/item/ShapelessRecipe.hpp"

#include <cstring>

#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

ShapelessRecipe::ShapelessRecipe(ItemInstance* result,
                                 std::vector<ItemInstance*>* ingredients,
                                 Recipe::_eGroupType group)
    : Recipe(),
      mGroup(group),
      mResultItem(result),
      mIngredients(ingredients)
{
}

ShapelessRecipe::~ShapelessRecipe()
{
}

ItemInstance* ShapelessRecipe::getResultItem()
{
    return mResultItem;
}

ItemInstance* ShapelessRecipe::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > ShapelessRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    unsigned int i;
    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->getItem()->hasCraftingRemainingItem())
        {
            result[i] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
    }
    return result;
}

Recipe::_eGroupType ShapelessRecipe::getGroup()
{
    return mGroup;
}

bool ShapelessRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    std::vector<ItemInstance*> remaining = *mIngredients;
    unsigned int row;
    unsigned int column;
    for (row = 0; row < craftingContainer->getHeight(); ++row)
    {
        for (column = 0; column < craftingContainer->getWidth(); ++column)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(column, row);
            if (!item->isEmpty())
            {
                std::vector<ItemInstance*>::iterator ingredient = remaining.begin();
                for (; ingredient != remaining.end(); ++ingredient)
                {
                    if (item->getItem() == (*ingredient)->getItem() &&
                        ((*ingredient)->getAuxValue() == -1 ||
                         item->getAuxValue() == (*ingredient)->getAuxValue() ||
                         ((*ingredient)->getAuxValue() == 0 && (*ingredient)->isDamageableItem())))
                    {
                        break;
                    }
                }
                if (ingredient == remaining.end())
                {
                    return false;
                }
                remaining.erase(ingredient);
            }
        }
    }
    return remaining.empty();
}

not_null_ptr<ItemInstance> ShapelessRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    return mResultItem->copy();
}

int ShapelessRecipe::size()
{
    return static_cast<int>(mIngredients->size());
}

bool ShapelessRecipe::recipeRequires(int itemId)
{
    std::vector<ItemInstance*>* ingredients = new std::vector<ItemInstance*>();
    *ingredients = *mIngredients;
    std::vector<ItemInstance*>::iterator ingredient;
    for (ingredient = mIngredients->begin(); ingredient != mIngredients->end(); ++ingredient)
    {
        ingredients->erase(ingredient);
    }
    delete ingredients;
    return false;
}

void ShapelessRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
    int* craftingGrid = new int[9]();
    int* itemIds = new int[9]();
    int* itemCounts = new int[9]();
    int* auxValues = new int[9];
    CompoundTag** itemTags = new CompoundTag*[9]();
    std::memset(auxValues, -1, sizeof(int) * 9);

    unsigned int i;
    unsigned int count = 0;
    unsigned int gridSlot = 0;
    for (i = 0; i < mIngredients->size(); ++i)
    {
        ItemInstance* ingredient = (*mIngredients)[i];
        if (!ingredient->isEmpty())
        {
            int id = Item::getId(ingredient->getItem());
            int aux = ingredient->getAuxValue();
            craftingGrid[gridSlot++] = id | (aux << 24);
            if (mIngredients->size() < 5 && gridSlot == 2)
            {
                gridSlot = 3;
            }

            unsigned int j;
            for (j = 0; j < count; ++j)
            {
                if (itemIds[j] == id && (aux == -1 || auxValues[j] == aux))
                {
                    ++itemCounts[j];
                    break;
                }
            }
            if (j == count)
            {
                itemIds[count] = id;
                itemCounts[count] = 1;
                auxValues[count] = aux;
                itemTags[count] = ingredient->getTag();
                ++count;
            }
        }
    }

    ingredientsRequired->numIngredients = count;
    ingredientsRequired->requiresCraftingTable = mIngredients->size() > 4;
    ingredientsRequired->itemIds = new int[count];
    ingredientsRequired->itemCounts = new int[count];
    ingredientsRequired->auxValues = new int[count];
    ingredientsRequired->itemTags = new CompoundTag*[count];
    ingredientsRequired->recipe = this;
    ingredientsRequired->craftingGrid = new int[9];
    for (i = 0; i < 4; ++i)
    {
        ingredientsRequired->available[i] = false;
    }
    if (count != 0)
    {
        std::memcpy(ingredientsRequired->itemIds, itemIds, sizeof(int) * count);
        std::memcpy(ingredientsRequired->itemCounts, itemCounts, sizeof(int) * count);
        std::memcpy(ingredientsRequired->auxValues, auxValues, sizeof(int) * count);
        std::memcpy(ingredientsRequired->itemTags, itemTags, sizeof(CompoundTag*) * count);
    }
    std::memcpy(ingredientsRequired->craftingGrid, craftingGrid, sizeof(int) * 9);

    delete[] itemIds;
    delete[] itemCounts;
    delete[] auxValues;
    delete[] itemTags;
    delete[] craftingGrid;
}

boost::shared_ptr<CraftingContainer> ShapelessRecipe::generateExpectedCraftSlots()
{
    boost::shared_ptr<CraftingContainer> container(new CraftingContainer(NULL, 3, 3));
    unsigned int slot = 0;
    std::vector<ItemInstance*>::iterator ingredient;
    for (ingredient = mIngredients->begin(); ingredient != mIngredients->end(); ++ingredient)
    {
        if (*ingredient != NULL && !(*ingredient)->isEmpty())
        {
            container->setItem(slot, (*ingredient)->copy());
            ++slot;
        }
    }
    return container;
}
