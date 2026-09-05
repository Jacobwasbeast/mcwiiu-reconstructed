#include "game/core/ShulkerBoxColoring.hpp"

#include "game/data/CompoundTag.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Block.hpp"

ShulkerBoxColoring::~ShulkerBoxColoring()
{
}

bool ShulkerBoxColoring::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level* level)
{
    unsigned int shulkerBoxes = 0;
    unsigned int dyes = 0;
    unsigned int slot;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (item->isEmpty())
        {
            continue;
        }

        if (Block::byItem(item->getItem()) != NULL)
        {
            ++shulkerBoxes;
        }
        else if (item->getItem() == Items::dye)
        {
            ++dyes;
        }
        else
        {
            return false;
        }

        if (shulkerBoxes > 1 || dyes > 1)
        {
            return false;
        }
    }

    return shulkerBoxes == 1 && dyes == 1;
}

not_null_ptr<ItemInstance> ShulkerBoxColoring::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    not_null_ptr<ItemInstance> shulkerBox;
    int dye = 0;
    unsigned int slot;

    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (item->isEmpty())
        {
            continue;
        }
        if (item->getItem() == Items::dye)
        {
            dye = item->getAuxValue();
        }
        else if (Block::byItem(item->getItem()) != NULL)
        {
            shulkerBox = item;
        }
    }

    not_null_ptr<ItemInstance> result(new ItemInstance(shulkerBox->getItem(), 1, dye));
    if (shulkerBox->hasTag())
    {
        result->setTag(shulkerBox->getTag()->copy());
    }
    return result;
}

int ShulkerBoxColoring::size()
{
    return 10;
}

ItemInstance* ShulkerBoxColoring::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* ShulkerBoxColoring::getIngredientItem(int column, int row)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > ShulkerBoxColoring::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);
    unsigned int slot;
    for (slot = 0; slot < craftingContainer->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(slot);
        if (!item->isEmpty() && item->getItem()->hasCraftingRemainingItem())
        {
            result[slot] = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem()->getCraftingRemainingItem()));
        }
    }
    return result;
}

Recipe::_eGroupType ShulkerBoxColoring::getGroup()
{
    return Recipe::eGroupType_None;
}

bool ShulkerBoxColoring::recipeRequires(int itemId)
{
    return false;
}

void ShulkerBoxColoring::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
}

boost::shared_ptr<CraftingContainer> ShulkerBoxColoring::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
