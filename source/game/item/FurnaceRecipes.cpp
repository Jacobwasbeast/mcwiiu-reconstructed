#include "game/item/FurnaceRecipes.hpp"

#include "game/item/FishFoodItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/Items.hpp"
#include "game/world/block/Blocks.hpp"

FurnaceRecipes* gFurnaceRecipes = 0;

FurnaceRecipes::FurnaceRecipes()
{
    addFurnaceRecipe(Blocks::iron_ore, new ItemInstance(Items::ironIngot), 0.7f);
    addFurnaceRecipe(Blocks::gold_ore, new ItemInstance(Items::goldIngot), 1.0f);
    addFurnaceRecipe(Blocks::sand, new ItemInstance(Blocks::glass), 0.1f);
    addFurnaceRecipe(Blocks::cobblestone, new ItemInstance(Blocks::stone), 0.1f);
    addFurnaceRecipe(Blocks::clay, new ItemInstance(Items::brick), 0.3f);
    addFurnaceRecipe(Blocks::log, new ItemInstance(Items::coal), 0.15f);
    addFurnaceRecipe(Blocks::log2, new ItemInstance(Items::coal), 0.15f);
    addFurnaceRecipe(Blocks::netherrack, new ItemInstance(Items::netherbrick), 0.1f);
    addFurnaceRecipe(Blocks::cactus, new ItemInstance(Items::dye, 1, 2), 0.2f);
    addFurnaceRecipe(Blocks::sponge, new ItemInstance(Blocks::sponge), 0.15f);
    addFurnaceRecipe(Items::beef, new ItemInstance(Items::cookedBeef), 0.35f);
    addFurnaceRecipe(Items::porkchop, new ItemInstance(Items::cookedPorkchop), 0.35f);
    addFurnaceRecipe(Items::chicken, new ItemInstance(Items::cookedChicken), 0.35f);
    addFurnaceRecipe(Items::mutton, new ItemInstance(Items::cookedMutton), 0.35f);
    addFurnaceRecipe(Items::rabbit, new ItemInstance(Items::cookedRabbit), 0.35f);
    addFurnaceRecipe(Items::potato, new ItemInstance(Items::bakedPotato), 0.35f);
    addFurnaceRecipe(Items::kelp, new ItemInstance(Items::driedKelp), 0.1f);
    addFurnaceRecipe(Blocks::chorus_flower, new ItemInstance(Items::chorusFruitPopped), 0.1f);

    FishFoodItem::Type* type = gFishFoodTypes[0];
    if (type->canBeCooked())
    {
        addFurnaceRecipe(new ItemInstance(Items::fish, 1, type->getId()),
                         new ItemInstance(Items::cookedFish, 1, type->getId()), 0.35f);
    }
    type = gFishFoodTypes[1];
    if (type->canBeCooked())
    {
        addFurnaceRecipe(new ItemInstance(Items::fish, 1, type->getId()),
                         new ItemInstance(Items::cookedFish, 1, type->getId()), 0.35f);
    }
    type = gFishFoodTypes[2];
    if (type->canBeCooked())
    {
        addFurnaceRecipe(new ItemInstance(Items::fish, 1, type->getId()),
                         new ItemInstance(Items::cookedFish, 1, type->getId()), 0.35f);
    }
    type = gFishFoodTypes[3];
    if (type->canBeCooked())
    {
        addFurnaceRecipe(new ItemInstance(Items::fish, 1, type->getId()),
                         new ItemInstance(Items::cookedFish, 1, type->getId()), 0.35f);
    }
}

bool FurnaceRecipes::isSameItem(not_null_ptr<ItemInstance> item, ItemInstance* recipeItem)
{
    if (item->getItem() != recipeItem->getItem())
    {
        return false;
    }
    if (recipeItem->getAuxValue() != -1 && item->getAuxValue() != recipeItem->getAuxValue())
    {
        return false;
    }
    return true;
}

ItemInstance* FurnaceRecipes::getResult(not_null_ptr<ItemInstance> item)
{
    iterator it;
    for (it = begin(); it != end(); ++it)
    {
        if (isSameItem(item, it->first))
        {
            return it->second;
        }
    }
    return ItemInstance::EMPTY.get();
}

FurnaceRecipes* FurnaceRecipes::getInstance()
{
    return gFurnaceRecipes;
}

double FurnaceRecipes::getRecipeValue(not_null_ptr<ItemInstance> item)
{
    boost::unordered_map<ItemInstance*, float>::iterator it;
    for (it = mRecipeExperience.begin(); it != mRecipeExperience.end(); ++it)
    {
        if (isSameItem(item, it->first))
        {
            return (double)it->second;
        }
    }
    return 0.0;
}

void FurnaceRecipes::addFurnaceRecipe(ItemInstance* item, ItemInstance* result, float experience)
{
    (*this)[item] = result;
    mRecipeExperience[item] = experience;
}

void FurnaceRecipes::addFurnaceRecipe(Item* item, ItemInstance* result, float experience)
{
    addFurnaceRecipe(new ItemInstance(item, 1, -1), result, experience);
}

void FurnaceRecipes::addFurnaceRecipe(Block* block, ItemInstance* result, float experience)
{
    addFurnaceRecipe(Item::byBlock(block), result, experience);
}

void FurnaceRecipes::staticCtor()
{
    gFurnaceRecipes = new FurnaceRecipes();
}
