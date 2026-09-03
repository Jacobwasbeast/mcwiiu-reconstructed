#ifndef GAME_ITEM_PICKAXEITEM_HPP
#define GAME_ITEM_PICKAXEITEM_HPP

#include "game/item/DiggerItem.hpp"
#include "game/item/Item.hpp"
#include "game/item/not_null_ptr.hpp"

class BlockState;
class ItemInstance;

class PickaxeItem : public DiggerItem
{
public:
    PickaxeItem(const Item::Tier* tier);

    static void staticCtor();

    virtual ~PickaxeItem();
    virtual float getDestroySpeed(not_null_ptr<ItemInstance> itemInstance, const BlockState* state);
    virtual bool canDestroySpecial(const BlockState* state);
};

#endif
