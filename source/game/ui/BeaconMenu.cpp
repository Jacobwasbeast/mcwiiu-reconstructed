#include "game/ui/BeaconMenu.hpp"

#include "game/item/Container.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/world/Level.hpp"
#include "game/world/generation/Player.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"

BeaconMenu::BeaconMenu(boost::shared_ptr<Container> playerContainer,
                       boost::shared_ptr<Container> beaconContainer)
{
    mBeaconContainer = beaconContainer;
    mPaymentSlot = new PaymentSlot(beaconContainer, 0, 136, 110);
    addSlot(mPaymentSlot);

    int row;
    int group;
    for (row = 0; row < 3; ++row)
    {
        for (group = 0; group < 3; ++group)
        {
            addSlot(new Slot(playerContainer, row * 9 + group * 3 + 9,
                             36 + group * 54, 137 + row * 18));
            addSlot(new Slot(playerContainer, row * 9 + group * 3 + 10,
                             54 + group * 54, 137 + row * 18));
            addSlot(new Slot(playerContainer, row * 9 + group * 3 + 11,
                             72 + group * 54, 137 + row * 18));
        }
    }
    for (group = 0; group < 9; ++group)
        addSlot(new Slot(playerContainer, group, 36 + group * 18, 195));
}

BeaconMenu::~BeaconMenu()
{
}

void BeaconMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setAllContainerData(this, mBeaconContainer);
}

void BeaconMenu::setData(int id, int value)
{
    mBeaconContainer->setContainerData(id, value);
}

void BeaconMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (player.get() != NULL && !player->mLevel->mIsClientSide)
    {
        not_null_ptr<ItemInstance> item = mPaymentSlot->getItem();
        if (!item->isEmpty())
            player->drop(item, false);
    }
}

boost::shared_ptr<Container> BeaconMenu::getBeaconContainer()
{
    return mBeaconContainer;
}

bool BeaconMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mBeaconContainer->stillValid(player);
}

not_null_ptr<ItemInstance> BeaconMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    Slot* slot = getSlot(index);
    if (slot == NULL || !slot->hasItem())
        return ItemInstance::EMPTY;

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> copy = item->copy();

    if (index == 0)
    {
        if (!moveItemStackTo(item, 1, 37, true, true, true))
            return ItemInstance::EMPTY;
        slot->onQuickCraft(item, copy);
    }
    else if (!mPaymentSlot->hasItem() && mPaymentSlot->mayPlace(item) && item->getCount() == 1)
    {
        if (!moveItemStackTo(item, 0, 1, false, true, true))
            return ItemInstance::EMPTY;
    }
    else if (index - 1 < 27)
    {
        if (!moveItemStackTo(item, 28, 37, false, true, true))
            return ItemInstance::EMPTY;
    }
    else if (index - 28 < 9)
    {
        if (!moveItemStackTo(item, 1, 28, false, true, true))
            return ItemInstance::EMPTY;
    }
    else
    {
        if (!moveItemStackTo(item, 1, 37, false, true, true))
            return ItemInstance::EMPTY;
    }

    if (item->isEmpty())
        slot->set(ItemInstance::EMPTY);
    else
        slot->setChanged();
    if (item->getCount() == copy->getCount())
        return ItemInstance::EMPTY;
    slot->onTake(player, item);
    return copy;
}

BeaconMenu::PaymentSlot::PaymentSlot(boost::shared_ptr<Container> container,
                                     int index, int x, int y)
    : Slot(container, index, x, y)
{
}

BeaconMenu::PaymentSlot::~PaymentSlot()
{
}

bool BeaconMenu::PaymentSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    Item* type = item->getItem();
    return type == Items::emerald || type == Items::diamond ||
           type == Items::goldIngot || type == Items::ironIngot;
}

int BeaconMenu::PaymentSlot::getMaxStackSize()
{
    return 1;
}
