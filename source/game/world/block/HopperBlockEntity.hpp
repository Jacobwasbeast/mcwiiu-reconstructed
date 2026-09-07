#ifndef GAME_WORLD_BLOCK_HOPPERBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_HOPPERBLOCKENTITY_HPP

#include <string>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/Hopper.hpp"
#include "game/core/Direction.hpp"
#include "game/item/Container.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class CompoundTag;
class DataFixerUpper;
class Entity;
class ItemEntity;
class Inventory;
class Level;
class Player;

class HopperBlockEntity : public RandomizableLockableContainerBlockEntity, public Hopper
{
public:
    HopperBlockEntity();
    virtual ~HopperBlockEntity();

    static BlockEntity* create();
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> itemInstance);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory,
                                               boost::shared_ptr<Player> player);
    virtual int getTypeId(bool large);
    virtual Level* getLevel();
    virtual double getLevelX();
    virtual double getLevelY();
    virtual double getLevelZ();
    static void addDataWalkers(DataFixerUpper* fixer);
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);
    virtual void tick();

    bool inventoryEmpty();
    bool inventoryFull();
    static std::vector<boost::shared_ptr<Entity> >* getItemsAtAndAbove(Level* level, double x, double y, double z);
    static boost::shared_ptr<Container> getContainerAt(Level* level, double x, double y, double z);
    static boost::shared_ptr<Container> getSourceContainer(Hopper* hopper);
    static bool isEmptyContainer(boost::shared_ptr<Container> container, const Direction* direction);
    static bool canTakeItemFromContainer(Container* container, not_null_ptr<ItemInstance> item, int slot, const Direction* direction);
    static bool canPlaceItemInContainer(Container* container, not_null_ptr<ItemInstance> item, int slot, const Direction* direction);
    static bool canMergeItems(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second);
    bool tryMoveInItem(Container* container, not_null_ptr<ItemInstance> item, int slot, const Direction* direction);
    static not_null_ptr<ItemInstance> addItem(Container* source, Container* destination,
                                               not_null_ptr<ItemInstance> item, const Direction* direction);
    static bool tryTakeInItemFromSlot(Hopper* hopper, Container* container, int slot, const Direction* direction);
    static bool addItem(Container* source, Container* destination, const boost::shared_ptr<ItemEntity>& entity);
    static bool suckInItems(Hopper* hopper);
    bool tryMoveItems();
    static bool isFullContainer(boost::shared_ptr<Container> container, const Direction* direction);
    bool ejectItems();
    boost::shared_ptr<Container> getAttachedContainer();
    void setCooldown(int cooldown);
    virtual bool isOnCooldown();
    bool isOnCustomCooldown();
    virtual NonNullList<not_null_ptr<ItemInstance> > getItems();
    virtual void setItems(NonNullList<not_null_ptr<ItemInstance> > items);
    virtual std::wstring getDisplayName();
    virtual unsigned long long GetType();

private:
    int mCooldown;
    int mTransferCooldown;
    bool mEnabled;
};

#endif
