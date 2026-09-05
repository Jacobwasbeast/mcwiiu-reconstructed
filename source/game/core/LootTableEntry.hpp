#ifndef GAME_CORE_LOOTTABLEENTRY_HPP
#define GAME_CORE_LOOTTABLEENTRY_HPP

#include <stdint.h>
#include <vector>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class LootTable;
class LootTableContext;
class Random;

class LootTableEntry
{
public:
    LootTableEntry(LootTable* lootTable);

    virtual uint32_t _createItem(std::vector<not_null_ptr<ItemInstance> >* items,
                                 Random* random,
                                 LootTableContext& context);

private:
    uint32_t mUnknown04;
    uint32_t mUnknown08;
    uint32_t mUnknown0C;
    uint32_t mUnknown10;
    LootTable* mLootTable;
};

#endif
