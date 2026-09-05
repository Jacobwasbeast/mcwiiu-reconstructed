#ifndef GAME_ITEM_MINECARTCONTAINER_HPP
#define GAME_ITEM_MINECARTCONTAINER_HPP

#include <boost/shared_ptr.hpp>
#include <string>

#include "game/core/InteractionHand.hpp"
#include "game/core/LockCode.hpp"
#include "game/core/Minecart.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/data/DataFixerUpper.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/LockableContainer.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/ui/EntityTooltipDataHolder.hpp"
#include "game/util/ResourceLocation.hpp"

class Player;

class MinecartContainer : public Minecart, public LockableContainer
{
public:
    MinecartContainer(Level* level, double x, double y, double z);
    MinecartContainer(Level* level);
    virtual ~MinecartContainer();

    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual bool hasCustomName();
    virtual std::wstring getDisplayName();
    virtual bool isEmpty();
    void unpackLootTable(const boost::shared_ptr<Player>& player);
    virtual not_null_ptr<ItemInstance> getItem(unsigned int slot);
    virtual not_null_ptr<ItemInstance> removeItem(unsigned int slot, int count);
    virtual not_null_ptr<ItemInstance> removeItemNoUpdate(int slot);
    virtual void setItem(unsigned int slot, not_null_ptr<ItemInstance> item);
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual bool stillValid(boost::shared_ptr<Player> player);
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual bool canPlaceItem(int slot, not_null_ptr<ItemInstance> item);
    virtual int getContainerData(int id);
    virtual void setContainerData(int id, int value);
    virtual int getContainerDataCount();
    virtual void clearContent();
    virtual bool isLocked();
    virtual void setLocked(LockCode* lockCode);
    virtual LockCode* getLockKeyCode();
    virtual void unlock();
    virtual ResourceLocation* getLootTable();
    virtual long long getLootTableSeed();

    void _init();
    static void addDataWalkers(DataFixerUpper* fixer, unsigned long long gameTime);
    virtual void destroy(const DamageSource* source);
    virtual void changeDimension(int dimension);
    virtual void remove();
    virtual void addAdditonalSaveData(CompoundTag* tag);
    virtual void readAdditionalSaveData(CompoundTag* tag);
    virtual bool interact(const boost::shared_ptr<Player>& player, InteractionHand::EInteractionHand hand);
    virtual void applyNaturalSlowdown();
    void setDropContainerContent(bool dropContents);
    void setLootTable(ResourceLocation* lootTable, long long seed);
    virtual unsigned int GetInteractTooltip(const EntityTooltipDataHolder& tooltipData);

protected:
    NonNullList<not_null_ptr<ItemInstance> > mItems;
    bool mDropContentsWhenDead;
    ResourceLocation* mLootTable;
    long long mLootTableSeed;
};

#endif
