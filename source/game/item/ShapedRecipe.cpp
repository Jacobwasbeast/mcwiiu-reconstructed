#include "game/item/ShapedRecipe.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

ShapedRecipe::ShapedRecipe(int width, int height, ItemInstance** ingredients, ItemInstance* result, int group)
    : mWidth(width),
      mHeight(height),
      mGroup(static_cast<_eGroupType>(group)),
      mIngredients(ingredients),
      mResult(result),
      mKeepTag(false)
{
    int i;
    for (i = 0; i < mWidth * mHeight; ++i)
    {
        if (mIngredients[i] == NULL)
        {
            mIngredients[i] = ItemInstance::EMPTY.get();
        }
    }
}

ShapedRecipe::~ShapedRecipe()
{
}

ItemInstance* ShapedRecipe::getResultItem()
{
    return mResult;
}

ItemInstance* ShapedRecipe::getIngredientItem(int column, int row)
{
    return mIngredients[column + row * mWidth];
}

NonNullList<not_null_ptr<ItemInstance> > ShapedRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
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

Recipe::_eGroupType ShapedRecipe::getGroup()
{
    return mGroup;
}

bool ShapedRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, int column, int row, bool mirror)
{
    int gridColumn;
    int gridRow;
    for (gridColumn = 0; gridColumn < 3; ++gridColumn)
    {
        for (gridRow = 0; gridRow < 3; ++gridRow)
        {
            ItemInstance* ingredient = NULL;
            int recipeColumn = gridColumn - column;
            int recipeRow = gridRow - row;
            if (recipeColumn >= 0 && recipeRow >= 0 && recipeColumn < mWidth && recipeRow < mHeight)
            {
                if (mirror)
                {
                    recipeColumn = mWidth - recipeColumn - 1;
                }
                ingredient = mIngredients[recipeColumn + recipeRow * mWidth];
            }

            not_null_ptr<ItemInstance> item = craftingContainer->getItem(gridColumn, gridRow);
            if (item->isEmpty() != (ingredient == NULL || ingredient->isEmpty()))
            {
                return false;
            }
            if (item->isEmpty())
            {
                continue;
            }
            if (item->getItem() != ingredient->getItem())
            {
                return false;
            }
            int auxValue = ingredient->getAuxValue();
            if (auxValue != -1 && item->getAuxValue() != auxValue &&
                (auxValue != 0 || item->isDamageableItem()))
            {
                return false;
            }
        }
    }
    return true;
}

bool ShapedRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    int column;
    int row;
    for (column = 0; column <= 3 - mWidth; ++column)
    {
        for (row = 0; row <= 3 - mHeight; ++row)
        {
            if (matches(craftingContainer, column, row, true) ||
                matches(craftingContainer, column, row, false))
            {
                return true;
            }
        }
    }
    return false;
}

not_null_ptr<ItemInstance> ShapedRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> result = getResultItem()->copy();
    if (mKeepTag)
    {
        unsigned int i;
        for (i = 0; i < craftingContainer->getContainerSize(); ++i)
        {
            not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
            if (!item->isEmpty() && item->hasTag())
            {
                result->setTag(item->getTag()->copy());
            }
        }
    }
    return result;
}

int ShapedRecipe::size()
{
    return mWidth * mHeight;
}

bool ShapedRecipe::recipeRequires(int itemId)
{
    int column;
    int row;
    for (column = 0; column < mWidth; ++column)
    {
        for (row = 0; row < mHeight; ++row)
        {
            mIngredients[column + row * mWidth]->isEmpty();
        }
    }
    return false;
}

void ShapedRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
    int itemIds[9] = {0};
    int itemCounts[9] = {0};
    int auxValues[9];
    CompoundTag* itemTags[9] = {NULL};
    int craftingGrid[9] = {0};
    int ingredientCount = 0;
    int column;
    int row;

    for (column = 0; column < 3; ++column)
    {
        for (row = 0; row < 3; ++row)
        {
            int index = column + row * 3;
            if (column >= mWidth || row >= mHeight)
            {
                auxValues[index] = -1;
                continue;
            }
            ItemInstance* ingredient = mIngredients[column + row * mWidth];
            if (ingredient == NULL || ingredient->isEmpty())
            {
                auxValues[index] = -1;
                continue;
            }
            int itemId = Item::VALIDgetId(ingredient->getItem());
            int auxValue = ingredient->getAuxValue();
            craftingGrid[index] = itemId | (auxValue << 24);
            int found = -1;
            int i;
            for (i = 0; i < ingredientCount; ++i)
            {
                if (itemIds[i] == itemId && (auxValues[i] == -1 || auxValues[i] == auxValue))
                {
                    found = i;
                    break;
                }
            }
            if (found < 0)
            {
                found = ingredientCount++;
                itemIds[found] = itemId;
                auxValues[found] = auxValue;
                itemTags[found] = ingredient->getTag();
            }
            ++itemCounts[found];
        }
    }

    ingredientsRequired->numIngredients = ingredientCount;
    ingredientsRequired->requiresCraftingTable = mWidth < 3 && mHeight < 3 ? 0 : 1;
    ingredientsRequired->itemIds = new int[ingredientCount];
    ingredientsRequired->itemCounts = new int[ingredientCount];
    ingredientsRequired->auxValues = new int[ingredientCount];
    ingredientsRequired->itemTags = new CompoundTag*[ingredientCount];
    ingredientsRequired->recipe = this;
    ingredientsRequired->available[0] = false;
    ingredientsRequired->available[1] = false;
    ingredientsRequired->available[2] = false;
    ingredientsRequired->available[3] = false;
    ingredientsRequired->craftingGrid = new int[9];
    if (ingredientCount != 0)
    {
        for (int i = 0; i < ingredientCount; ++i)
        {
            ingredientsRequired->itemIds[i] = itemIds[i];
            ingredientsRequired->itemCounts[i] = itemCounts[i];
            ingredientsRequired->auxValues[i] = auxValues[i];
            ingredientsRequired->itemTags[i] = itemTags[i];
        }
    }
    for (int i = 0; i < 9; ++i)
    {
        ingredientsRequired->craftingGrid[i] = craftingGrid[i];
    }
}

boost::shared_ptr<CraftingContainer> ShapedRecipe::generateExpectedCraftSlots()
{
    boost::shared_ptr<CraftingContainer> result(new CraftingContainer(NULL, 3, 3));
    int column;
    int row;
    for (column = 0; column < 3; ++column)
    {
        for (row = 0; row < 3; ++row)
        {
            if (column < mWidth && row < mHeight)
            {
                ItemInstance* ingredient = mIngredients[column + row * mWidth];
                if (ingredient != NULL && !ingredient->isEmpty())
                {
                    result->setItem(static_cast<unsigned int>(column + row * 3), ingredient->copy());
                }
            }
        }
    }
    return result;
}
