#include "game/world/block/ChestBlockEntity.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/Level.hpp"

ChestBlockEntity::ChestBlockEntity(bool large)
    : RandomizableLockableContainerBlockEntity(),
      mType(ChestBlock::TYPE_SINGLE),
      mLarge(large),
      mOpenersCounter(0),
      mOpen(0),
      mOpenOld(0),
      mTickCount(0),
      mNeighbour(),
      mNeighborsChecked(false),
      mRefilledAnimationTicks(0),
      mCachedType(-1)
{
    _init(large);
}

ChestBlockEntity::ChestBlockEntity(ChestBlock::Type type, bool large)
    : RandomizableLockableContainerBlockEntity(),
      mType(type),
      mLarge(large),
      mOpenersCounter(0),
      mOpen(0),
      mOpenOld(0),
      mTickCount(0),
      mNeighbour(),
      mNeighborsChecked(false),
      mRefilledAnimationTicks(0),
      mCachedType(-1)
{
    _init(large);
}

ChestBlockEntity::~ChestBlockEntity()
{
    removeNeighbour();
}

BlockEntity* ChestBlockEntity::create()
{
    return new ChestBlockEntity(false);
}

void ChestBlockEntity::_init(bool large)
{
    mLarge = large;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(27, not_null_ptr<ItemInstance>());
    mOpenersCounter = 0;
    mOpen = 0;
    mOpenOld = 0;
    mTickCount = 0;
    mNeighborsChecked = false;
    mRefilledAnimationTicks = 0;
    mCachedType = -1;
}

std::wstring ChestBlockEntity::getName()
{
    if (hasCustomName())
    {
        return mCustomName ? mCustomName->get() : std::wstring();
    }
    return L"container.chest";
}

std::wstring ChestBlockEntity::getCustomName()
{
    if (hasCustomName())
    {
        return mCustomName ? mCustomName->get() : std::wstring();
    }
    return L"container.chest";
}

unsigned int ChestBlockEntity::getContainerSize()
{
    return mItems.size();
}

bool ChestBlockEntity::isEmpty()
{
    unsigned int i;
    for (i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i] || !mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

int ChestBlockEntity::getMaxStackSize()
{
    return 64;
}

void ChestBlockEntity::setChanged()
{
    BlockEntity::setChanged();
}

bool ChestBlockEntity::stillValid(boost::shared_ptr<Player> player)
{
    if (isRemoved() || !player)
    {
        return false;
    }
    return RandomizableLockableContainerBlockEntity::stillValid(player);
}

void ChestBlockEntity::startOpen(boost::shared_ptr<Player> player)
{
    if (!player)
    {
        return;
    }
    ++mOpenersCounter;
    RandomizableLockableContainerBlockEntity::startOpen(player);
}

void ChestBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    if (mOpenersCounter > 0)
    {
        --mOpenersCounter;
    }
    RandomizableLockableContainerBlockEntity::stopOpen(player);
}

bool ChestBlockEntity::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return slot >= 0 && static_cast<unsigned int>(slot) < getContainerSize() && !!item;
}

AbstractContainerMenu* ChestBlockEntity::createMenu(boost::shared_ptr<Inventory> inventory, boost::shared_ptr<Player> player)
{
    if (player)
    {
        unpackLootTable(player);
    }
    return NULL;
}

int ChestBlockEntity::getTypeId(bool large)
{
    if (mLarge)
    {
        return 13;
    }
    return large ? 14 : 0;
}

void ChestBlockEntity::addDataWalkers(DataFixerUpper* fixer)
{
    if (fixer)
    {
        fixer->addWalker(DataFixTypes::eDataFixTypes_BlockEntity,
                         (DataWalker*)new ItemListDataWalker(0x1e00201ULL, L"Items"));
    }
}

void ChestBlockEntity::load(CompoundTag* tag)
{
    LockableContainerBlockEntity::load(tag);
    ContainerHelper::loadAllItems(tag, mItems);
}

void ChestBlockEntity::save(CompoundTag* tag)
{
    LockableContainerBlockEntity::save(tag);
    ContainerHelper::saveAllItems(tag, mItems);
}

void ChestBlockEntity::clearCache()
{
    BlockEntity::clearCache();
    mNeighborsChecked = false;
}

void ChestBlockEntity::heyImYourNeighbor(boost::shared_ptr<ChestBlockEntity> neighbour, const Direction* direction)
{
    if (neighbour && direction)
    {
        mNeighbour = neighbour;
        mNeighborsChecked = true;
    }
}

boost::shared_ptr<ChestBlockEntity> ChestBlockEntity::getNeighbour()
{
    return mNeighbour;
}

void ChestBlockEntity::removeNeighbour()
{
    mNeighbour.reset();
}

bool ChestBlockEntity::isSameChest(const BlockPos& position)
{
    return getBlockPos() == position;
}

void ChestBlockEntity::findAndGreetChestEntity(const Direction* direction)
{
    if (direction)
    {
        mNeighborsChecked = true;
    }
}

void ChestBlockEntity::checkNeighbors()
{
    if (!mNeighborsChecked)
    {
        findAndGreetChestEntity(Direction::NORTH);
    }
}

void ChestBlockEntity::tickRefilledAnimation()
{
    if (mRefilledAnimationTicks > 0)
    {
        --mRefilledAnimationTicks;
    }
}

void ChestBlockEntity::tick()
{
    ++mTickCount;
    tickRefilledAnimation();
    checkNeighbors();
    mOpenOld = mOpen;
    if (mOpenersCounter > 0 && mOpen < 1)
    {
        ++mOpen;
    }
    else if (mOpenersCounter == 0 && mOpen > 0)
    {
        --mOpen;
    }
}

bool ChestBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenersCounter = param;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void ChestBlockEntity::setRemoved()
{
    removeNeighbour();
    BlockEntity::setRemoved();
}

int ChestBlockEntity::getType()
{
    return mCachedType == -1 ? static_cast<int>(mType) : mCachedType;
}

NonNullList<not_null_ptr<ItemInstance> > ChestBlockEntity::getItems()
{
    return mItems;
}

void ChestBlockEntity::setItems(NonNullList<not_null_ptr<ItemInstance> > items)
{
    mItems = items;
}

boost::shared_ptr<BlockEntity> ChestBlockEntity::clone()
{
    boost::shared_ptr<ChestBlockEntity> result(new ChestBlockEntity(mType, mLarge));
    result->setItems(mItems);
    result->mOpenersCounter = mOpenersCounter;
    result->mOpen = mOpen;
    result->mOpenOld = mOpenOld;
    result->mTickCount = mTickCount;
    return result;
}

void ChestBlockEntity::startRefilledAnimation()
{
    mRefilledAnimationTicks = 200;
}

CompoundTag* ChestBlockEntity::getUpdateTagForPistonMovement()
{
    CompoundTag* tag = new CompoundTag();
    save(tag);
    return tag;
}

std::wstring ChestBlockEntity::getDisplayName()
{
    return LockableContainerBlockEntity::getDisplayName();
}

unsigned long long ChestBlockEntity::GetType()
{
    return 0x01e0020100000000ULL;
}
