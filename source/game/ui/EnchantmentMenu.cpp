#include "game/ui/EnchantmentMenu.hpp"

#include "game/core/Slot.hpp"
#include "game/item/EnchantmentCostSlot.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/generation/Player.hpp"

EnchantmentMenu::EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos)
    : AbstractContainerMenu(), mEnchantmentContainer(new EnchantmentContainer(this)), mLevel(level), mPosition(pos), mRandom()
{
    addSlot(new Slot(mEnchantmentContainer, 0, 15, 47));
    addSlot(new EnchantmentCostSlot(mEnchantmentContainer, 1, 35, 47));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 3; ++column)
        {
            addSlot(new Slot(boost::static_pointer_cast<Container>(inventory), row * 9 + column + 9, 8 + column * 18, 84 + row * 18));
        }
    }
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
            addSlot(new Slot(boost::static_pointer_cast<Container>(inventory), row * 9 + column, 8 + column * 18, 142 + row * 18));
    }
}

EnchantmentMenu::EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level)
    : AbstractContainerMenu(), mEnchantmentContainer(new EnchantmentContainer(this)), mLevel(level), mPosition(BlockPos::ZERO), mRandom()
{
    addSlot(new Slot(mEnchantmentContainer, 0, 15, 47));
    addSlot(new EnchantmentCostSlot(mEnchantmentContainer, 1, 35, 47));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 3; ++column)
        {
            addSlot(new Slot(boost::static_pointer_cast<Container>(inventory), row * 9 + column + 9, 8 + column * 18, 84 + row * 18));
        }
    }
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
            addSlot(new Slot(boost::static_pointer_cast<Container>(inventory), row * 9 + column, 8 + column * 18, 142 + row * 18));
    }
}

EnchantmentMenu::~EnchantmentMenu()
{
}

void EnchantmentMenu::broadcastData(net_minecraft_world_inventory::ContainerListener* listener)
{
    int i;
    for (i = 0; i < 3; ++i)
        listener->setContainerData(this, i, mEnchantmentContainer->getContainerData(i));
    listener->setContainerData(this, 3, mEnchantmentContainer->getContainerData(3));
    for (i = 4; i < 10; ++i)
        listener->setContainerData(this, i, mEnchantmentContainer->getContainerData(i));
}

void EnchantmentMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    broadcastData(listener);
}

void EnchantmentMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();

    for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
         it != m_listeners.end(); ++it)
    {
        if (*it != 0)
            broadcastData(*it);
    }
}

void EnchantmentMenu::setData(int id, int value)
{
    if (id >= 0 && id < 10)
        mEnchantmentContainer->setContainerData(id, value);
    else
        AbstractContainerMenu::setData(id, value);
}

std::vector<EnchantmentInstance*>* EnchantmentMenu::getEnchantmentList(not_null_ptr<ItemInstance> item, int level, int slot)
{
    mRandom.setSeed(static_cast<long long>(level) + mRandom.nextLong());
    return EnchantmentHelper::selectEnchantment(&mRandom, item, slot, false);
}

void EnchantmentMenu::slotsChanged()
{
    AbstractContainerMenu::slotsChanged();
}

bool EnchantmentMenu::clickMenuButton(boost::shared_ptr<Player> player, int id)
{
    if (!player || id < 0 || id >= 3)
        return false;

    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(0);
    if (item->isEmpty() || mCosts[id] <= 0)
        return false;

    const int lapisRequired = id + 1;
    not_null_ptr<ItemInstance> lapis = mEnchantmentContainer->getItem(1);
    if (lapis->isEmpty() || lapis->getCount() < lapisRequired)
        return false;

    if (!player->mAbilities.mInstabuild &&
        (player->mExperienceLevel < lapisRequired || player->mExperienceLevel < mCosts[id]))
        return false;

    std::vector<EnchantmentInstance*>* results =
        getEnchantmentList(item, mCosts[id], id);
    if (results == 0 || results->empty())
        return false;

    if (item->getItem() == Items::enchantedBook)
    {
        for (std::vector<EnchantmentInstance*>::iterator it = results->begin();
             it != results->end(); ++it)
        {
            if (*it != 0)
                EnchantedBookItem::addEnchantment(item, *it);
        }
    }
    else
    {
        for (std::vector<EnchantmentInstance*>::iterator it = results->begin();
             it != results->end(); ++it)
        {
            if (*it != 0)
                item->enchant((*it)->enchantment, (*it)->level);
        }
    }
    delete results;

    if (!player->mAbilities.mInstabuild)
        player->giveExperienceLevels(-mCosts[id]);
    lapis->shrink(lapisRequired);
    slotsChanged();
    return true;
}

int EnchantmentMenu::getGoldCount()
{
    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(1);
    if (item->isEmpty())
        return 0;
    return item->getCount();
}

void EnchantmentMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    mEnchantmentContainer->clearContent();
}

bool EnchantmentMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mEnchantmentContainer->stillValid(player);
}

not_null_ptr<ItemInstance> EnchantmentMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    if (index < 0 || index >= static_cast<int>(m_slots.size()))
        return ItemInstance::EMPTY;
    Slot* slot = getSlot(index);
    if (slot == 0 || !slot->hasItem())
        return ItemInstance::EMPTY;

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> result = item->copy();
    const int inventoryStart = 2;
    const int inventoryEnd = static_cast<int>(m_slots.size());
    if (index < inventoryStart)
    {
        if (!moveItemStackTo(item, inventoryStart, inventoryEnd, true, false, false))
            return ItemInstance::EMPTY;
    }
    else if (!moveItemStackTo(item, 0, inventoryStart, false, false, false))
    {
        return ItemInstance::EMPTY;
    }
    if (item->isEmpty())
        slot->set(ItemInstance::EMPTY);
    else
        slot->setChanged();
    slot->onTake(player, item);
    return result;
}
