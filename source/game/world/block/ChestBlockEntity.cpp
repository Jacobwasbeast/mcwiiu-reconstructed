#include "game/world/block/ChestBlockEntity.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/world/Level.hpp"

ChestBlockEntity::ChestBlockEntity(bool large)
    : RandomizableLockableContainerBlockEntity(),
      mLarge(large),
      mRefilledAnimationTicks(-1),
      mNeighborsChecked(false),
      mNorthNeighbour(),
      mEastNeighbour(),
      mWestNeighbour(),
      mSouthNeighbour(),
      mOpen(0.0F),
      mOpenOld(0.0F),
      mOpenCount(0),
      mTickCount(0),
      mCachedType(-1),
      mNeighbourRemoved(false)
{
    _init(large);
}

ChestBlockEntity::ChestBlockEntity(ChestBlock::Type type, bool large)
    : RandomizableLockableContainerBlockEntity(),
      mLarge(large),
      mRefilledAnimationTicks(-1),
      mNeighborsChecked(false),
      mNorthNeighbour(),
      mEastNeighbour(),
      mWestNeighbour(),
      mSouthNeighbour(),
      mOpen(0.0F),
      mOpenOld(0.0F),
      mOpenCount(0),
      mTickCount(0),
      mCachedType(type),
      mNeighbourRemoved(false)
{
    _init(large);
    mCachedType = type;
}

ChestBlockEntity::~ChestBlockEntity()
{
}

BlockEntity* ChestBlockEntity::create()
{
    return new ChestBlockEntity(false);
}

void ChestBlockEntity::_init(bool large)
{
    mLarge = large;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(27, not_null_ptr<ItemInstance>());
    mRefilledAnimationTicks = -1;
    mOpen = 0.0F;
    mOpenOld = 0.0F;
    mOpenCount = 0;
    mTickCount = 0;
    mNeighborsChecked = false;
    mCachedType = -1;
    mNeighbourRemoved = false;
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
    ++mOpenCount;
    RandomizableLockableContainerBlockEntity::startOpen(player);
}

void ChestBlockEntity::stopOpen(boost::shared_ptr<Player> player)
{
    if (mOpenCount > 0)
    {
        --mOpenCount;
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
    if (!neighbour || !direction)
    {
        mNeighborsChecked = false;
        return;
    }

    boost::shared_ptr<ChestBlockEntity> current;
    switch (direction->get3DDataValue())
    {
    case Direction::eDirection_North:
        current = mNorthNeighbour.lock();
        mNorthNeighbour = neighbour;
        break;
    case Direction::eDirection_South:
        current = mSouthNeighbour.lock();
        mSouthNeighbour = neighbour;
        break;
    case Direction::eDirection_West:
        current = mWestNeighbour.lock();
        mWestNeighbour = neighbour;
        break;
    case Direction::eDirection_East:
        current = mEastNeighbour.lock();
        mEastNeighbour = neighbour;
        break;
    default:
        return;
    }
    if (current != neighbour)
    {
        mNeighborsChecked = false;
    }
    if (mLevel)
    {
        mLevel->updateNeighborsAtExceptFromFacing(mPos, getBlock(), direction);
    }
}

boost::shared_ptr<ChestBlockEntity> ChestBlockEntity::getNeighbour()
{
    boost::shared_ptr<ChestBlockEntity> neighbour = mWestNeighbour.lock();
    if (!neighbour)
    {
        neighbour = mEastNeighbour.lock();
    }
    if (!neighbour)
    {
        neighbour = mNorthNeighbour.lock();
    }
    if (!neighbour)
    {
        neighbour = mSouthNeighbour.lock();
    }
    return neighbour;
}

void ChestBlockEntity::removeNeighbour()
{
    mNeighbourRemoved = true;
}

bool ChestBlockEntity::isSameChest(const BlockPos& position)
{
    if (!mLevel)
    {
        return false;
    }
    boost::shared_ptr<ChestBlockEntity> chest =
        boost::dynamic_pointer_cast<ChestBlockEntity>(mLevel->getBlockEntity(position));
    return chest && chest->getType() == getType();
}

boost::shared_ptr<ChestBlockEntity> ChestBlockEntity::findAndGreetChestEntity(const Direction* direction)
{
    if (!mLevel || !direction)
    {
        return boost::shared_ptr<ChestBlockEntity>();
    }
    BlockPos position = mPos.relative(direction);
    if (!isSameChest(position))
    {
        return boost::shared_ptr<ChestBlockEntity>();
    }
    boost::shared_ptr<ChestBlockEntity> neighbour =
        boost::static_pointer_cast<ChestBlockEntity>(mLevel->getBlockEntity(position));
    if (neighbour)
    {
        neighbour->heyImYourNeighbor(boost::static_pointer_cast<ChestBlockEntity>(shared_from_this()),
                                     direction->getOpposite());
    }
    return neighbour;
}

void ChestBlockEntity::checkNeighbors()
{
    if (!mNeighborsChecked && mWestNeighbour.expired() && mEastNeighbour.expired() &&
        mNorthNeighbour.expired() && mSouthNeighbour.expired())
    {
        mNeighborsChecked = true;
        mWestNeighbour = findAndGreetChestEntity(Direction::WEST);
        if (mWestNeighbour.expired())
        {
            mEastNeighbour = findAndGreetChestEntity(Direction::EAST);
            if (mEastNeighbour.expired())
            {
                mNorthNeighbour = findAndGreetChestEntity(Direction::NORTH);
                if (mNorthNeighbour.expired())
                {
                    mSouthNeighbour = findAndGreetChestEntity(Direction::SOUTH);
                }
            }
        }
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
    if (mOpenCount > 0 && mOpen < 1.0F)
    {
        mOpen += 0.1F;
        if (mOpen > 1.0F)
        {
            mOpen = 1.0F;
        }
    }
    else if (mOpenCount == 0 && mOpen > 0.0F)
    {
        mOpen -= 0.1F;
        if (mOpen < 0.0F)
        {
            mOpen = 0.0F;
        }
    }
}

bool ChestBlockEntity::triggerEvent(int id, int param)
{
    if (id == 1)
    {
        mOpenCount = param;
        return true;
    }
    return BlockEntity::triggerEvent(id, param);
}

void ChestBlockEntity::setRemoved()
{
    BlockEntity::setRemoved();
    checkNeighbors();
    boost::shared_ptr<ChestBlockEntity> neighbour = getNeighbour();
    if (neighbour)
    {
        neighbour->removeNeighbour();
    }
}

int ChestBlockEntity::getType()
{
    if (mCachedType == -1)
    {
        ChestBlock* chestBlock = dynamic_cast<ChestBlock*>(getBlock());
        if (!mLevel || !chestBlock)
        {
            return 0;
        }
        mCachedType = 0;
    }
    return mCachedType;
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
    boost::shared_ptr<ChestBlockEntity> result(new ChestBlockEntity((ChestBlock::Type)getType(), mLarge));
    result->setItems(mItems);
    result->mOpenCount = mOpenCount;
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
