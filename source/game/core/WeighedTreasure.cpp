#include "game/core/WeighedTreasure.hpp"

#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/util/Random.hpp"

WeighedTreasure::WeighedTreasure(Item* item, int auxValue, int minCount, int maxCount, int weight)
    : mWeight(weight)
    , mItem(new ItemInstance(item, 1, auxValue))
    , mMinCount(minCount)
    , mMaxCount(maxCount)
{
}

WeighedTreasure::WeighedTreasure(not_null_ptr<ItemInstance> item, int minCount, int maxCount, int weight)
    : mWeight(weight)
    , mItem(item)
    , mMinCount(minCount)
    , mMaxCount(maxCount)
{
}

WeighedTreasure::~WeighedTreasure()
{
}

not_null_ptr<ItemInstance> WeighedTreasure::getItem()
{
    return mItem;
}

void WeighedTreasure::addToTreasure(arrayWithLength<WeighedTreasure*> treasures, WeighedTreasure* treasure)
{
    if (treasure == NULL || treasures.data == NULL || treasures.length == 0)
        return;
    for (unsigned int i = 0; i < treasures.length; ++i)
    {
        if (treasures.data[i] == NULL)
        {
            treasures.data[i] = treasure;
            return;
        }
    }
}

void WeighedTreasure::addChestItems(Random* random, arrayWithLength<WeighedTreasure*> treasures,
                                    boost::shared_ptr<Container> container, int count, bool allowEmpty)
{
    if (random == NULL || container.get() == NULL || treasures.data == NULL || treasures.length == 0)
        return;
    for (int i = 0; i < count; ++i)
    {
        int totalWeight = 0;
        for (unsigned int j = 0; j < treasures.length; ++j)
            if (treasures.data[j] != NULL)
                totalWeight += treasures.data[j]->mWeight;
        if (totalWeight <= 0)
            return;
        int selection = random->nextInt(totalWeight);
        WeighedTreasure* selected = NULL;
        for (unsigned int j = 0; j < treasures.length; ++j)
        {
            if (treasures.data[j] == NULL)
                continue;
            selection -= treasures.data[j]->mWeight;
            if (selection < 0)
            {
                selected = treasures.data[j];
                break;
            }
        }
        if (selected == NULL)
            continue;
        not_null_ptr<ItemInstance> item = selected->mItem;
        int amount = selected->mMinCount;
        if (selected->mMaxCount > selected->mMinCount)
            amount += random->nextInt(selected->mMaxCount - selected->mMinCount + 1);
        item->setCount(amount);
        int slot = random->nextInt((int)container->getContainerSize());
        if (allowEmpty || container->getItem(slot)->isEmpty())
            container->setItem(slot, item);
    }
}
