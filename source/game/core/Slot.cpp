#include "game/core/Slot.hpp"

#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"

Slot::Slot(boost::shared_ptr<Container> container, int index, int x, int y)
    : mSlot(index),
      mContainer(container),
      mIndex(0),
      mX(x),
      mY(y)
{
}

Slot::~Slot()
{
}

void Slot::onQuickCraft(not_null_ptr<ItemInstance> oldItem, not_null_ptr<ItemInstance> newItem)
{
    int oldCount = oldItem->getCount();
    int newCount = newItem->getCount();
    if (newCount - oldCount > 0)
    {
        onQuickCraft(newItem, newCount - oldCount);
    }
}

void Slot::onQuickCraft(not_null_ptr<ItemInstance> item, int count)
{
}

void Slot::onSwapCraft(int count)
{
}

void Slot::checkTakeAchievements(not_null_ptr<ItemInstance> item)
{
}

not_null_ptr<ItemInstance> Slot::onTake(boost::shared_ptr<Player> player,
                                        not_null_ptr<ItemInstance> item)
{
    setChanged();
    return item;
}

bool Slot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return true;
}

not_null_ptr<ItemInstance> Slot::getItem()
{
    return mContainer->getItem(mSlot);
}

bool Slot::hasItem()
{
    return !getItem()->isEmpty();
}

void Slot::set(not_null_ptr<ItemInstance> item)
{
    mContainer->setItem(mSlot, item);
    setChanged();
}

void Slot::setChanged()
{
    mContainer->setChanged();
}

int Slot::getMaxStackSize()
{
    return mContainer->getMaxStackSize();
}

int Slot::getMaxStackSize(not_null_ptr<ItemInstance> item)
{
    return getMaxStackSize();
}

std::wstring Slot::getNoItemIcon()
{
    return std::wstring();
}

not_null_ptr<ItemInstance> Slot::remove(int count)
{
    return mContainer->removeItem(mSlot, count);
}

bool Slot::isAt(boost::shared_ptr<Container> container, int index)
{
    return container.get() == mContainer.get() && index == mSlot;
}

bool Slot::mayPickup(boost::shared_ptr<Player> player)
{
    return true;
}

bool Slot::isActive()
{
    return true;
}

bool Slot::mayCombine(not_null_ptr<ItemInstance> item)
{
    not_null_ptr<ItemInstance> current = getItem();
    if (current->isEmpty() || item->isEmpty())
    {
        return false;
    }
    if (Item::getId(current->getItem()) != Item::getId(item->getItem()))
    {
        return false;
    }
    if (current->getCount() == 1 && item->getCount() == 1 &&
        current->getItem()->canBeDepleted())
    {
        return current->isDamaged() == item->isDamaged();
    }
    return !current->isEnchanted() && !item->isEnchanted();
}

not_null_ptr<ItemInstance> Slot::combine(not_null_ptr<ItemInstance> item)
{
    return getItem();
}

void Slot::swap(Slot* other)
{
    not_null_ptr<ItemInstance> current = getItem();
    not_null_ptr<ItemInstance> incoming = other->getItem();
    if (current->isEmpty())
    {
        if (!incoming->isEmpty())
        {
            set(incoming);
            other->set(current);
        }
        return;
    }
    if (incoming->isEmpty())
    {
        other->set(current);
        set(incoming);
        return;
    }
    if (mayCombine(incoming) && other->mayCombine(current))
    {
        int capacity = other->getMaxStackSize(current) - incoming->getCount();
        if (capacity > 0)
        {
            int amount = current->getCount() < capacity ? current->getCount() : capacity;
            current->split(amount);
            incoming->grow(amount);
            other->set(incoming);
            if (current->isEmpty())
            {
                set(current);
            }
            else
            {
                set(current);
            }
            return;
        }
    }
    set(incoming);
    other->set(current);
}
