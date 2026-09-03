#ifndef GAME_CORE_WEIGHTEDTREASURE_HPP
#define GAME_CORE_WEIGHTEDTREASURE_HPP

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"
#include "game/world/biome/arrayWithLength.hpp"

class Container;
class Item;
class ItemInstance;
class Random;

class WeighedTreasure
{
public:
    WeighedTreasure(Item* item, int auxValue, int minCount, int maxCount, int weight);
    WeighedTreasure(not_null_ptr<ItemInstance> item, int minCount, int maxCount, int weight);
    virtual ~WeighedTreasure();
    not_null_ptr<ItemInstance> getItem();
    static void addChestItems(Random* random, arrayWithLength<WeighedTreasure*> treasures,
                              boost::shared_ptr<Container> container, int count, bool allowEmpty);
    static void addToTreasure(arrayWithLength<WeighedTreasure*> treasures, WeighedTreasure* treasure);

private:
    int mWeight;
    not_null_ptr<ItemInstance> mItem;
    int mMinCount;
    int mMaxCount;
};

#endif
