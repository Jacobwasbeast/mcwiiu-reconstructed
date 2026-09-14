#include "game/item/SimpleContainer.hpp"

#include "game/core/L10N.hpp"
#include "game/item/ContainerHelper.hpp"
#include "game/item/ItemInstance.hpp"

SimpleContainer::SimpleContainer(int id, const std::wstring& name, bool hasCustomName, int size)
    : mId(0), mName(), mSize(0), mItems(), mListeners(), mHasCustomName(false)
{
    _init(id, name, hasCustomName, size);
}

SimpleContainer::SimpleContainer(const std::wstring& name, int size)
    : mId(0), mName(), mSize(0), mItems(), mListeners(), mHasCustomName(false)
{
    _init(0, name, true, size);
}

SimpleContainer::~SimpleContainer()
{
}

void SimpleContainer::_init(int id, const std::wstring& name, bool hasCustomName, int size)
{
    mId = id;
    mName.assign(name, 0, 0xffffffff);
    mHasCustomName = hasCustomName;
    mSize = size;
    mItems = NonNullList<not_null_ptr<ItemInstance> >::withSize((unsigned int)size, ItemInstance::EMPTY);
}

std::wstring SimpleContainer::getName()
{
    if (mName.empty())
    {
        return std::wstring(L10N::GetString(mId));
    }
    return mName;
}

std::wstring SimpleContainer::getCustomName()
{
    if (hasCustomName())
    {
        return mName;
    }
    return std::wstring(L"container");
}

bool SimpleContainer::hasCustomName()
{
    return mHasCustomName;
}

std::wstring SimpleContainer::getDisplayName()
{
    if (hasCustomName())
    {
        return getCustomName();
    }
    return getName();
}

unsigned int SimpleContainer::getContainerSize()
{
    return (unsigned int)mSize;
}

bool SimpleContainer::isEmpty()
{
    for (unsigned int i = 0; i < (unsigned int)mSize; ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> SimpleContainer::getItem(unsigned int slot)
{
    if (slot >= (unsigned int)mSize)
    {
        return ItemInstance::EMPTY;
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> SimpleContainer::removeItem(unsigned int slot, int count)
{
    not_null_ptr<ItemInstance> result = ContainerHelper::removeItem(mItems, slot, (unsigned int)count);
    if (!result->isEmpty())
    {
        setChanged();
    }
    return result;
}

not_null_ptr<ItemInstance> SimpleContainer::removeItemNoUpdate(int slot)
{
    not_null_ptr<ItemInstance> result = mItems[(unsigned int)slot];
    if (!result->isEmpty())
    {
        mItems[(unsigned int)slot] = ItemInstance::EMPTY;
        return result;
    }
    return ItemInstance::EMPTY;
}

void SimpleContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot >= (unsigned int)mSize)
    {
        return;
    }
    mItems[slot] = item;
    if (!item->isEmpty() && item->getCount() > getMaxStackSize())
    {
        item->setCount(getMaxStackSize());
    }
    setChanged();
}

int SimpleContainer::getMaxStackSize()
{
    return 64;
}

void SimpleContainer::setChanged()
{
    for (std::vector<net_minecraft_world::ContainerListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        (*it)->containerChanged(this);
    }
}

bool SimpleContainer::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

void SimpleContainer::startOpen(boost::shared_ptr<Player> player)
{
}

void SimpleContainer::stopOpen(boost::shared_ptr<Player> player)
{
}

bool SimpleContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

int SimpleContainer::getContainerData(int id)
{
    return 0;
}

void SimpleContainer::setContainerData(int id, int value)
{
}

int SimpleContainer::getContainerDataCount()
{
    return 0;
}

void SimpleContainer::clearContent()
{
    for (unsigned int i = 0; i < (unsigned int)mSize; ++i)
    {
        mItems[i] = ItemInstance::EMPTY;
    }
}

void SimpleContainer::addListener(net_minecraft_world::ContainerListener* listener)
{
    mListeners.push_back(listener);
}

void SimpleContainer::removeListener(net_minecraft_world::ContainerListener* listener)
{
    for (std::vector<net_minecraft_world::ContainerListener*>::iterator it = mListeners.begin(); it != mListeners.end(); ++it)
    {
        if (*it == listener)
        {
            mListeners.erase(it);
            return;
        }
    }
}

not_null_ptr<ItemInstance> SimpleContainer::addItem(not_null_ptr<ItemInstance> item)
{
    not_null_ptr<ItemInstance> remainder = item->copy();
    for (int i = 0; i < mSize; ++i)
    {
        not_null_ptr<ItemInstance> current = getItem((unsigned int)i);
        if (current->isEmpty())
        {
            setItem((unsigned int)i, remainder);
            setChanged();
            return ItemInstance::EMPTY;
        }
        else if (ItemInstance::isSame(current, remainder))
        {
            int amount = getMaxStackSize();
            if (current->getMaxStackSize() < amount)
            {
                amount = current->getMaxStackSize();
            }
            int count = remainder->getCount();
            if (amount - current->getCount() < count)
            {
                count = amount - current->getCount();
            }
            if (count > 0)
            {
                current->grow(count);
                remainder->shrink(count);
                if (remainder->isEmpty())
                {
                    setChanged();
                    return ItemInstance::EMPTY;
                }
            }
        }
    }
    if (remainder->getCount() != item->getCount())
    {
        setChanged();
    }
    return remainder;
}

void SimpleContainer::setCustomName(const std::wstring& name)
{
    mHasCustomName = true;
    mName.assign(name, 0, 0xffffffff);
}
