#ifndef GAME_ITEM_EMPTYLOOTITEM_HPP
#define GAME_ITEM_EMPTYLOOTITEM_HPP

#include <stdint.h>
#include <vector>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class LootTableContext;
class Random;

class EmptyLootItem
{
public:
    virtual uint32_t _createItem(std::vector<not_null_ptr<ItemInstance> >* items,
                                 Random* random,
                                 LootTableContext& context);
};

#endif
