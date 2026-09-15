#include "game/item/CraftingContainer.hpp"

#include "game/item/ItemInstance.hpp"

CraftingContainer::CraftingContainer(AbstractContainerMenu* menu, unsigned int width, unsigned int height)
    : Container(),
      mMenu(menu),
      mItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(width * height, ItemInstance::EMPTY)),
      mWidth(width),
      mHeight(height)
{
}

CraftingContainer::~CraftingContainer()
{
}

std::wstring CraftingContainer::getName()
{
    return std::wstring(L"container.crafting");
}

std::wstring CraftingContainer::getCustomName()
{
    return std::wstring();
}

bool CraftingContainer::hasCustomName()
{
    return false;
}

std::wstring CraftingContainer::getDisplayName()
{
    return getName();
}

unsigned int CraftingContainer::getContainerSize()
{
    return mItems.size();
}

bool CraftingContainer::isEmpty()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        if (!mItems[i]->isEmpty())
        {
            return false;
        }
    }
    return true;
}

not_null_ptr<ItemInstance> CraftingContainer::getItem(unsigned int slot)
{
    if (slot >= getContainerSize())
    {
        return ItemInstance::EMPTY;
    }
    return mItems[slot];
}

not_null_ptr<ItemInstance> CraftingContainer::getItem(unsigned int row, unsigned int column)
{
    if (row >= mHeight || column >= mWidth)
        return ItemInstance::EMPTY;
    return getItem(row + column * mHeight);
}

not_null_ptr<ItemInstance> CraftingContainer::removeItem(unsigned int slot, int count)
{
    if (slot >= mItems.size() || count <= 0)
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> item = mItems[slot];
    if (item->isEmpty())
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> result = item->split(count);
    setChanged();
    return result;
}

not_null_ptr<ItemInstance> CraftingContainer::removeItemNoUpdate(int slot)
{
    if (slot < 0 || (unsigned int)slot >= mItems.size())
    {
        return ItemInstance::EMPTY;
    }
    not_null_ptr<ItemInstance> result = mItems[(unsigned int)slot];
    mItems[(unsigned int)slot] = ItemInstance::EMPTY;
    return result;
}

void CraftingContainer::setItem(unsigned int slot, not_null_ptr<ItemInstance> item)
{
    if (slot >= mItems.size())
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

int CraftingContainer::getMaxStackSize()
{
    return 64;
}

void CraftingContainer::setChanged()
{
}

bool CraftingContainer::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

void CraftingContainer::startOpen(boost::shared_ptr<Player> player)
{
}

void CraftingContainer::stopOpen(boost::shared_ptr<Player> player)
{
}

bool CraftingContainer::canPlaceItem(int slot, not_null_ptr<ItemInstance> item)
{
    return true;
}

int CraftingContainer::getContainerData(int id)
{
    return 0;
}

void CraftingContainer::setContainerData(int id, int value)
{
}

int CraftingContainer::getContainerDataCount()
{
    return 0;
}

void CraftingContainer::clearContent()
{
    for (unsigned int i = 0; i < mItems.size(); ++i)
    {
        mItems[i] = ItemInstance::EMPTY;
    }
}

unsigned int CraftingContainer::getHeight()
{
    return mHeight;
}

unsigned int CraftingContainer::getWidth()
{
    return mWidth;
}
