#include "game/item/EmptyLootItem.hpp"

EmptyLootItem::EmptyLootItem(int weight,
                             int quality,
                             std::vector<LootItemCondition*>* conditions,
                             LootTableEntry* entry)
    : LootPoolEntry(weight, quality, conditions, entry)
{
}

uint32_t EmptyLootItem::_createItem(std::vector<not_null_ptr<ItemInstance> >* items,
                                    Random* random,
                                    LootTableContext& context)
{
    return 0;
}
