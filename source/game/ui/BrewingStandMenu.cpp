#include "game/ui/BrewingStandMenu.hpp"

#include "game/item/ItemInstance.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Items.hpp"
#include "game/item/PotionBrewing.hpp"
#include "game/item/PotionUtils.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"

BrewingStandMenu::PotionSlot::PotionSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

BrewingStandMenu::PotionSlot::~PotionSlot()
{
}

bool BrewingStandMenu::PotionSlot::mayPlaceItem(not_null_ptr<ItemInstance> item)
{
    Item* type = item->getItem();
    return type == Items::potion || type == Items::splashPotion ||
           type == Items::lingeringPotion || type == Items::glassBottle;
}

bool BrewingStandMenu::PotionSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return mayPlaceItem(item);
}

int BrewingStandMenu::PotionSlot::getMaxStackSize()
{
    return 1;
}

not_null_ptr<ItemInstance> BrewingStandMenu::PotionSlot::onTake(boost::shared_ptr<Player> player,
                                                                 not_null_ptr<ItemInstance> item)
{
    item->onCraftedBy(NULL, player, 1);
    return Slot::onTake(player, item);
}

bool BrewingStandMenu::PotionSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}

BrewingStandMenu::IngredientsSlot::IngredientsSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

BrewingStandMenu::IngredientsSlot::~IngredientsSlot()
{
}

bool BrewingStandMenu::IngredientsSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return PotionBrewing::isIngredient(item);
}

int BrewingStandMenu::IngredientsSlot::getMaxStackSize()
{
    return 64;
}

bool BrewingStandMenu::IngredientsSlot::mayCombine(not_null_ptr<ItemInstance> item)
{
    return false;
}

BrewingStandMenu::FuelSlot::FuelSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

BrewingStandMenu::FuelSlot::~FuelSlot()
{
}

bool BrewingStandMenu::FuelSlot::mayPlaceItem(not_null_ptr<ItemInstance> item)
{
    return item->getItem() == Items::blazePowder;
}

bool BrewingStandMenu::FuelSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    return mayPlaceItem(item);
}

int BrewingStandMenu::FuelSlot::getMaxStackSize()
{
    return 64;
}

BrewingStandMenu::BrewingStandMenu(boost::shared_ptr<Inventory> inventory,
                                   boost::shared_ptr<Container> container)
    : mContainer(container),
      mIngredientSlot(0),
      mLastBrewTime(0),
      mLastFuel(0)
{
    addSlot(new PotionSlot(container, 0, 56, 51));
    addSlot(new PotionSlot(container, 1, 79, 58));
    addSlot(new PotionSlot(container, 2, 102, 51));
    mIngredientSlot = addSlot(new IngredientsSlot(container, 3, 79, 17));
    addSlot(new FuelSlot(container, 4, 17, 17));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9,
                             8 + column * 18, 84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }
}

BrewingStandMenu::~BrewingStandMenu()
{
}

void BrewingStandMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setContainerData(this, 0, mContainer->getContainerData(0));
    listener->setContainerData(this, 1, mContainer->getContainerData(1));
}

void BrewingStandMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();
    int brewTime = mContainer->getContainerData(0);
    int fuel = mContainer->getContainerData(1);
    if (mLastBrewTime != brewTime)
    {
        for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
             it != m_listeners.end(); ++it)
        {
            (*it)->setContainerData(this, 0, brewTime);
        }
    }
    if (mLastFuel != fuel)
    {
        for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
             it != m_listeners.end(); ++it)
        {
            (*it)->setContainerData(this, 1, fuel);
        }
    }
    mLastBrewTime = brewTime;
    mLastFuel = fuel;
}

void BrewingStandMenu::setData(int id, int value)
{
    mContainer->setContainerData(id, value);
}

bool BrewingStandMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mContainer->stillValid(player);
}

not_null_ptr<ItemInstance> BrewingStandMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> empty = ItemInstance::EMPTY;
    Slot* source = getSlot(index);
    if (source == NULL || !source->hasItem())
    {
        return empty;
    }
    not_null_ptr<ItemInstance> item = source->getItem();
    not_null_ptr<ItemInstance> original = item->copy();
    if (index == 3)
    {
        if (!moveItemStackTo(item, 5, (int)m_slots.size(), true, true, true))
        {
            return empty;
        }
    }
    else if (index >= 0 && index < 5)
    {
        if (!moveItemStackTo(item, 5, (int)m_slots.size(), true, true, true))
        {
            return empty;
        }
    }
    else if (PotionSlot::mayPlaceItem(item))
    {
        if (!moveItemStackTo(item, 0, 3, false, true, true))
        {
            return empty;
        }
    }
    else if (PotionBrewing::isIngredient(item))
    {
        if (!moveItemStackTo(item, 3, 4, false, true, true))
        {
            return empty;
        }
    }
    else if (FuelSlot::mayPlaceItem(item))
    {
        if (!moveItemStackTo(item, 4, 5, false, true, true))
        {
            return empty;
        }
    }
    else if (index >= 5 && index < 32)
    {
        if (!moveItemStackTo(item, 32, (int)m_slots.size(), false, true, true))
        {
            return empty;
        }
    }
    else if (index >= 32 && index < (int)m_slots.size() &&
             !moveItemStackTo(item, 5, 32, false, true, true))
    {
        return empty;
    }
    else
    {
        return empty;
    }
    if (item->isEmpty())
    {
        source->set(ItemInstance::EMPTY);
    }
    else
    {
        source->setChanged();
    }
    if (item->getCount() == original->getCount())
    {
        return empty;
    }
    source->onTake(player, item);
    return original;
}
