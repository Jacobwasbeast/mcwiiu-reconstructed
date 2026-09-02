#ifndef GAME_CORE_SLOT_HPP
#define GAME_CORE_SLOT_HPP

#include <string>

#include <boost/shared_ptr.hpp>

#include "game/item/not_null_ptr.hpp"

class Container;
class ItemInstance;
class Player;

class Slot
{
public:
    int slot;
    boost::shared_ptr<Container> container;
    int index;
    int x;
    int y;

    Slot(boost::shared_ptr<Container> container, int slot, int x, int y);
    virtual ~Slot();

    void onQuickCraft(not_null_ptr<ItemInstance> oldItemInstance, not_null_ptr<ItemInstance> newItemInstance);
    void swap(Slot* other);

    virtual void onQuickCraft(not_null_ptr<ItemInstance> itemInstance, int count);
    virtual void onSwapCraft(int count);
    virtual void checkTakeAchievements(not_null_ptr<ItemInstance> itemInstance);
    virtual not_null_ptr<ItemInstance> onTake(boost::shared_ptr<Player> player, not_null_ptr<ItemInstance> itemInstance);
    virtual bool mayPlace(not_null_ptr<ItemInstance> itemInstance);
    virtual not_null_ptr<ItemInstance> getItem();
    virtual bool hasItem();
    virtual void set(not_null_ptr<ItemInstance> itemInstance);
    virtual void setChanged();
    virtual int getMaxStackSize();
    virtual int getMaxStackSize(not_null_ptr<ItemInstance> itemInstance);
    virtual std::wstring getNoItemIcon();
    virtual not_null_ptr<ItemInstance> remove(int count);
    virtual bool isAt(boost::shared_ptr<Container> container, int slot);
    virtual bool mayPickup(boost::shared_ptr<Player> player);
    virtual bool isActive();
    virtual bool mayCombine(not_null_ptr<ItemInstance> itemInstance);
    virtual not_null_ptr<ItemInstance> combine(not_null_ptr<ItemInstance> itemInstance);
};

#endif
