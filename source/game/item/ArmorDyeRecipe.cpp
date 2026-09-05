#include "game/item/ArmorDyeRecipe.hpp"

#include "game/core/DyeColor.hpp"
#include "game/entity/mob/HorseArmorItem.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/ItemInstance.hpp"

ArmorDyeRecipe::~ArmorDyeRecipe()
{
}

bool ArmorDyeRecipe::matches(boost::shared_ptr<CraftingContainer> craftingContainer, Level*)
{
    unsigned int i;
    unsigned int dyeCount = 0;
    not_null_ptr<ItemInstance> armor = ItemInstance::EMPTY;

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
        else if (dynamic_cast<DyeItem*>(itemType) == NULL)
        {
            return false;
        }
        else
        {
            ++dyeCount;
        }
    }

    return !armor->isEmpty() && dyeCount > 0;
}

not_null_ptr<ItemInstance> ArmorDyeRecipe::assembleDyedArmor(boost::shared_ptr<CraftingContainer> craftingContainer)
{
    unsigned int i;
    unsigned int dyeCount = 0;
    not_null_ptr<ItemInstance> armor = ItemInstance::EMPTY;
    ArmorItem* armorItem = NULL;
    HorseArmorItem* horseArmorItem = NULL;
    int red = 0;
    int green = 0;
    int blue = 0;
    int maxColor = 0;

    for (i = 0; i < craftingContainer->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> item = craftingContainer->getItem(i);
        if (item->isEmpty())
        {
            continue;
        }

        Item* itemType = item->getItem();
        armorItem = dynamic_cast<ArmorItem*>(itemType);
        horseArmorItem = dynamic_cast<HorseArmorItem*>(itemType);
        if (armorItem != NULL && armorItem->getMaterial() == ArmorItem::ArmorMaterial::LEATHER)
        {
            if (armor->isEmpty())
            {
                armor = item->copy();
                armor->setCount(1);
            }
            if (armorItem->hasCustomColor(item))
            {
                int color = armorItem->getColor(item, 0);
                red += (color >> 16) & 255;
                green += (color >> 8) & 255;
                blue += color & 255;
                maxColor += 1;
            }
        }
        else if (horseArmorItem != NULL && horseArmorItem->isDyeable())
        {
            if (armor->isEmpty())
            {
                armor = item->copy();
                armor->setCount(1);
            }
            if (horseArmorItem->hasCustomColor(item))
            {
                int color = horseArmorItem->getColor(item, 0);
                red += (color >> 16) & 255;
                green += (color >> 8) & 255;
                blue += color & 255;
                maxColor += 1;
            }
        }
        else if (dynamic_cast<DyeItem*>(itemType) != NULL)
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
            maxColor += brightest;
            ++dyeCount;
        }
    }

    if (armor->isEmpty() || dyeCount == 0)
    {
        return ItemInstance::EMPTY;
    }

    if (armorItem != NULL && armorItem->getMaterial() == ArmorItem::ArmorMaterial::LEATHER)
    {
        if (maxColor == 0)
        {
            return armor;
        }
        red /= dyeCount;
        green /= dyeCount;
        blue /= dyeCount;
        int brightest = red;
        if (green > brightest)
        {
            brightest = green;
        }
        if (blue > brightest)
        {
            brightest = blue;
        }
        int scale = maxColor / dyeCount;
        if (brightest < scale)
        {
            scale = brightest;
        }
        red = (int)(red * scale / (brightest == 0 ? 1 : brightest));
        green = (int)(green * scale / (brightest == 0 ? 1 : brightest));
        blue = (int)(blue * scale / (brightest == 0 ? 1 : brightest));
        armorItem->setColor(armor, (red << 16) | (green << 8) | blue);
    }
    else if (horseArmorItem != NULL && horseArmorItem->isDyeable())
    {
        int color = ((red / dyeCount) << 16) | ((green / dyeCount) << 8) | (blue / dyeCount);
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
        if (!item->isEmpty() && item->getItem()->hasCraftingRemainingItem())
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
    ingredientsRequired->missingSlotMask[0] = 0;
    ingredientsRequired->missingSlotMask[1] = 0;
    ingredientsRequired->missingSlotMask[2] = 0;
    ingredientsRequired->missingSlotMask[3] = 0;
}

boost::shared_ptr<CraftingContainer> ArmorDyeRecipe::generateExpectedCraftSlots()
{
    return boost::shared_ptr<CraftingContainer>();
}
