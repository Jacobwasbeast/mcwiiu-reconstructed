#include <algorithm>
#include <vector>

#include "game/item/ArmorDyeRecipe.hpp"

#include "game/core/DyeColor.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"

ArmorDyeRecipe::~ArmorDyeRecipe()
{
}

bool ArmorDyeRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level*)
{
    unsigned int i;
    not_null_ptr<ItemInstance> armor = ItemInstance::EMPTY;
    std::vector<not_null_ptr<ItemInstance> > dyes;

    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }

        Item* itemType = item->getItem();
        ArmorItem* armorItem = dynamic_cast<ArmorItem*>(itemType);
        HorseArmorItem* horseArmorItem = dynamic_cast<HorseArmorItem*>(itemType);
        if (armorItem != NULL)
        {
            if (armorItem->getMaterial() != ArmorItem::ArmorMaterial::LEATHER || !armor->isEmpty())
            {
                return false;
            }
            armor = item;
        }
        else if (horseArmorItem != NULL && horseArmorItem->isDyeable())
        {
            if (!armor->isEmpty())
            {
                return false;
            }
            armor = item;
        }
        else if (itemType != Items::dye)
        {
            return false;
        }
        else
        {
            dyes.push_back(item);
        }
    }

    return !armor->isEmpty() && !dyes.empty();
}

not_null_ptr<ItemInstance> ArmorDyeRecipe::assembleDyedArmor(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    not_null_ptr<ItemInstance> armor = ItemInstance::EMPTY;
    ArmorItem* armorItem = NULL;
    HorseArmorItem* horseArmorItem = NULL;
    int red = 0;
    int green = 0;
    int blue = 0;
    int totalBrightness = 0;
    int colorCount = 0;

    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }

        Item* itemType = item->getItem();
        ArmorItem* currentArmorItem = dynamic_cast<ArmorItem*>(itemType);
        HorseArmorItem* currentHorseArmorItem = dynamic_cast<HorseArmorItem*>(itemType);
        if ((currentArmorItem != NULL && currentArmorItem->getMaterial() == ArmorItem::ArmorMaterial::LEATHER) ||
            (currentHorseArmorItem != NULL && currentHorseArmorItem->isDyeable()))
        {
            if (!armor->isEmpty())
            {
                return ItemInstance::EMPTY;
            }

            armorItem = currentArmorItem;
            horseArmorItem = currentHorseArmorItem;
            armor = item->copy();
            armor->setCount(1);

            bool hasCustomColor;
            if (armorItem != NULL)
            {
                hasCustomColor = armorItem->hasCustomColor(item);
            }
            else
            {
                hasCustomColor = horseArmorItem->hasCustomColor(item);
            }

            if (hasCustomColor)
            {
                int color = itemType->getColor(armor, 0);
                float colorRed = (float)((color >> 16) & 255) / 255.0f;
                float colorGreen = (float)((color >> 8) & 255) / 255.0f;
                float colorBlue = (float)(color & 255) / 255.0f;
                red += (int)(colorRed * 255.0f);
                green += (int)(colorGreen * 255.0f);
                blue += (int)(colorBlue * 255.0f);
                totalBrightness += (int)(std::max(colorRed, std::max(colorGreen, colorBlue)) * 255.0f);
                ++colorCount;
            }
        }
        else if (itemType == Items::dye)
        {
            arrayWithLength<const float> colors = DyeColor::byItemData(item->getAuxValue())->getTextureDiffuseColors();
            int colorRed = (int)(colors.data[0] * 255.0f);
            int colorGreen = (int)(colors.data[1] * 255.0f);
            int colorBlue = (int)(colors.data[2] * 255.0f);
            int brightest = colorRed;
            if (colorGreen > brightest)
            {
                brightest = colorGreen;
            }
            if (colorBlue > brightest)
            {
                brightest = colorBlue;
            }
            red += colorRed;
            green += colorGreen;
            blue += colorBlue;
            totalBrightness += brightest;
            ++colorCount;
        }
        else
        {
            return ItemInstance::EMPTY;
        }
    }

    if (armorItem == NULL && horseArmorItem == NULL)
    {
        return ItemInstance::EMPTY;
    }

    red /= colorCount;
    green /= colorCount;
    blue /= colorCount;
    float averageBrightness = (float)totalBrightness / (float)colorCount;
    float brightest = (float)std::max(red, std::max(green, blue));
    red = (int)((float)red * averageBrightness / brightest);
    green = (int)((float)green * averageBrightness / brightest);
    blue = (int)((float)blue * averageBrightness / brightest);
    int color = (red << 16) | (green << 8) | blue;

    if (armorItem != NULL)
    {
        armorItem->setColor(armor, color);
    }
    else
    {
        horseArmorItem->setColor(armor, color);
    }

    return armor;
}

not_null_ptr<ItemInstance> ArmorDyeRecipe::assemble(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    return assembleDyedArmor(craftingContainer);
}

int ArmorDyeRecipe::size()
{
    return 10;
}

ItemInstance* ArmorDyeRecipe::getResultItem()
{
    return ItemInstance::EMPTY.get();
}

ItemInstance* ArmorDyeRecipe::getIngredientItem(int, int)
{
    return ItemInstance::EMPTY.get();
}

NonNullList<not_null_ptr<ItemInstance> > ArmorDyeRecipe::getRemainingItems(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    NonNullList<not_null_ptr<ItemInstance> > result =
        NonNullList<not_null_ptr<ItemInstance> >::withSize(craftingContainer->getContainerSize(), ItemInstance::EMPTY);

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

Recipe::_eGroupType ArmorDyeRecipe::getGroup()
{
    return Recipe::eGroupType_None;
}

bool ArmorDyeRecipe::recipeRequires(int)
{
    return false;
}

void ArmorDyeRecipe::recipeRequires(INGREDIENTS_REQUIRED* ingredientsRequired)
{
    ingredientsRequired->numIngredients = 0;
    ingredientsRequired->requiresCraftingTable = 0;
    ingredientsRequired->itemIds = new int[0];
    ingredientsRequired->itemCounts = new int[0];
    ingredientsRequired->auxValues = new int[0];
    ingredientsRequired->itemTags = new CompoundTag*[0];
    ingredientsRequired->recipe = this;
    ingredientsRequired->available[0] = false;
    ingredientsRequired->available[1] = false;
    ingredientsRequired->available[2] = false;
    ingredientsRequired->available[3] = false;
    ingredientsRequired->craftingGrid = new int[9]();
}

boost::shared_ptr<CraftingContainer> ArmorDyeRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
