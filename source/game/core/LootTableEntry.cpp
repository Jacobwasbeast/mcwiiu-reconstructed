#include "game/core/LootTableEntry.hpp"

#include "game/core/LootTable.hpp"

LootTableEntry::LootTableEntry(LootTable* lootTable)
    : mLootTable(lootTable)
{
}

uint32_t LootTableEntry::_createItem(std::vector<not_null_ptr<ItemInstance> >* items,
                                     Random* random,
                                     LootTableContext& context)
{
    std::vector<not_null_ptr<ItemInstance> >* randomItems = mLootTable->getRandomItems(random, context);
    if (randomItems->size() > 0)
    {
        items->insert(items->end(), randomItems->begin(), randomItems->end());
        delete randomItems;
        return 1;
    }

    delete randomItems;
    return 0;
}
