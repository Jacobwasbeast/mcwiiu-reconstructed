#include "game/core/LootPoolEntry.hpp"

#include "game/core/LootTableContext.hpp"
#include "game/core/LootTableEntry.hpp"
#include "game/core/Mth.hpp"
#include "game/util/Random.hpp"

LootPoolEntry::LootPoolEntry(int weight,
                             int quality,
                             std::vector<LootItemCondition*>* conditions,
                             LootTableEntry* entry)
    : mConditions(conditions),
      mEntry(entry),
      mWeight(weight),
      mQuality(quality)
{
}

void LootPoolEntry::createItem(std::vector<not_null_ptr<ItemInstance> >* items,
                               Random* random,
                               LootTableContext& context)
{
    if (_createItem(items, random, context) && mEntry != NULL)
    {
        mEntry->_createItem(items, random, context);
    }
}

uint32_t LootPoolEntry::getWeight(float luck)
{
    int weight = Mth::floor((float)mWeight + (float)mQuality * luck);
    return weight < 0 ? 0 : (uint32_t)weight;
}
