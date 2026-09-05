#ifndef GAME_WORLD_BLOCK_SHULKERBOXBLOCKENTITY_HPP
#define GAME_WORLD_BLOCK_SHULKERBOXBLOCKENTITY_HPP

#include <string>
#include <boost/shared_ptr.hpp>

#include "game/core/Direction.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/NonNullList.hpp"
#include "game/item/WorldlyContainer.hpp"
#include "game/item/not_null_ptr.hpp"
#include "game/world/block/RandomizableLockableContainerBlockEntity.hpp"

class AbstractContainerMenu;
class AABB;
class BlockEntity;
class BlockState;
class CompoundTag;
class DataFixerUpper;
class Inventory;
class Packet;
class Player;

class ShulkerBoxBlockEntity : public RandomizableLockableContainerBlockEntity,
                              public WorldlyContainer,
                              public InteractionObject
{
public:
    ShulkerBoxBlockEntity(const DyeColor* color);
    ShulkerBoxBlockEntity();
    virtual ~ShulkerBoxBlockEntity();

    static BlockEntity* create();
    void _init(const DyeColor* color);
    virtual std::wstring getName();
    virtual std::wstring getCustomName();
    virtual unsigned int getContainerSize();
    virtual bool isEmpty();
    virtual int getMaxStackSize();
    virtual void setChanged();
    virtual void startOpen(boost::shared_ptr<Player> player);
    virtual void stopOpen(boost::shared_ptr<Player> player);
    virtual void clearContent();
    virtual AbstractContainerMenu* createMenu(boost::shared_ptr<Inventory> inventory,
                                               boost::shared_ptr<Player> player);
    virtual int getTypeId(bool large);
    virtual arrayWithLength<int> getSlotsForFace(const Direction* face);
    virtual bool canPlaceItemThroughFace(int slot, not_null_ptr<ItemInstance> item,
                                         const Direction* face);
    virtual bool canTakeItemThroughFace(int slot, not_null_ptr<ItemInstance> item,
                                        const Direction* face);

    int getAnimationStatus();
    void setDestroyedByCreativePlayer(bool value);
    bool isCleared();
    bool isDestroyedByCreativePlayer();
    bool shouldDrop();
    CompoundTag* saveToTag(CompoundTag* tag);
    float getProgress(float partialTick);
    AABB* getBoundingBox(const Direction* direction);
    AABB* getBoundingBox(const BlockState* state);
    AABB* getTopBoundingBox(const Direction* direction);
    void moveCollidedEntities();
    void updateAnimation();
    virtual void tick();
    virtual bool triggerEvent(int id, int param);
    static void addDataWalkers(DataFixerUpper* fixer);
    void loadFromTag(CompoundTag* tag);
    virtual void load(CompoundTag* tag);
    virtual void save(CompoundTag* tag);
    virtual NonNullList<not_null_ptr<ItemInstance> > getItems();
    virtual void setItems(NonNullList<not_null_ptr<ItemInstance> > items);
    const DyeColor* getColor();
    virtual boost::shared_ptr<Packet> getUpdatePacket();
    virtual boost::shared_ptr<BlockEntity> clone();
    virtual unsigned long long GetType();
    void operator=(const ShulkerBoxBlockEntity& other);

private:
    bool mCleared;
    int mOpenCount;
    int mAnimationStatus;
    float mProgress;
    float mProgressOld;
    const DyeColor* mColor;
    bool mDestroyedByCreativePlayer;
};

#endif
