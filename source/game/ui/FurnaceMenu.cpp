#include "game/ui/FurnaceMenu.hpp"

#include "game/core/FurnaceFuelSlot.hpp"
#include "game/core/FurnaceResultSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/FurnaceBlockEntity.hpp"

class FurnaceRecipes
{
public:
    static FurnaceRecipes* getInstance();
    not_null_ptr<ItemInstance> getResult(not_null_ptr<ItemInstance> item);
};

class Player
{
public:
    void awardStat(Stat* stat, arrayWithLength<unsigned char> parameters);
};

FurnaceMenu::FurnaceMenu(boost::shared_ptr<Inventory> inventory,
                         boost::shared_ptr<Container> container)
    : AbstractContainerMenu(),
      mContainer(container),
      mLitTime(0),
      mLitDuration(0),
      mCookingProgress(0),
      mCookingTotalTime(0)
{
    addSlot(new Slot(container, 0, 56, 17));
    addSlot(new FurnaceFuelSlot(container, 1, 56, 53));
    addSlot(new FurnaceResultSlot(boost::shared_ptr<Player>(), container, 2, 116, 35));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(boost::static_pointer_cast<Container>(inventory),
                             column + row * 9 + 9,
                             8 + column * 18,
                             84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(boost::static_pointer_cast<Container>(inventory), column,
                         8 + column * 18, 142));
    }
}

FurnaceMenu::~FurnaceMenu()
{
}

void FurnaceMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setContainerData(this, 0, mContainer->getContainerData(0));
    listener->setContainerData(this, 1, mContainer->getContainerData(1));
    listener->setContainerData(this, 2, mContainer->getContainerData(2));
    listener->setContainerData(this, 3, mContainer->getContainerData(3));
}

void FurnaceMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();
    for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
         it != m_listeners.end(); ++it)
    {
        net_minecraft_world_inventory::ContainerListener* listener = *it;
        int value = mContainer->getContainerData(2);
        if (mCookingProgress != value) listener->setContainerData(this, 2, value);
        value = mContainer->getContainerData(0);
        if (mLitTime != value) listener->setContainerData(this, 0, value);
        value = mContainer->getContainerData(1);
        if (mLitDuration != value) listener->setContainerData(this, 1, value);
        value = mContainer->getContainerData(3);
        if (mCookingTotalTime != value) listener->setContainerData(this, 3, value);
    }
    mCookingProgress = mContainer->getContainerData(2);
    mLitTime = mContainer->getContainerData(0);
    mLitDuration = mContainer->getContainerData(1);
    mCookingTotalTime = mContainer->getContainerData(3);
}

void FurnaceMenu::setData(int id, int value)
{
    mContainer->setContainerData(id, value);
}

bool FurnaceMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mContainer->stillValid(player);
}

unsigned int FurnaceMenu::GetInvSlotStart()
{
    return IsSmallInventory() ? 30 : 3;
}

void FurnaceMenu::fjCheckAchievements(const boost::shared_ptr<Player>& player,
                                      const not_null_ptr<ItemInstance>& item)
{
    if (mContainer->getContainerData(4) == 1 &&
        item->getItem() == NULL && item->getAuxValue() == 1)
    {
        player->awardStat(GenericStats::renewableEnergy(),
                          GenericStats::param_renewableEnergy());
        return;
    }
    if (Block::byItem(item->getItem()) == NULL)
    {
        player->awardStat(GenericStats::drySpell(), GenericStats::param_drySpell());
    }
}

not_null_ptr<ItemInstance> FurnaceMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> empty = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot == NULL || !slot->hasItem()) return empty;

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> original = item->copy();
    bool moved = false;
    int inventoryStart = GetInvSlotStart();

    if (index == 2)
    {
        moved = moveItemStackTo(item, inventoryStart, 39, true, true, true);
        if (!moved) return empty;
        slot->onQuickCraft(item, original);
        fjCheckAchievements(player, original);
        mContainer->setContainerData(4, 0);
    }
    else if (index == 0 || index == 1)
    {
        moved = moveItemStackTo(item, inventoryStart, 39, false, true, true);
        if (!moved) return empty;
    }
    else
    {
        not_null_ptr<ItemInstance> result = FurnaceRecipes::getInstance()->getResult(item);
        if (!result->isEmpty())
        {
            moved = moveItemStackTo(item, 0, 1, false, true, true);
        }
        else if (FurnaceBlockEntity::isFuel(item))
        {
            moved = moveItemStackTo(item, 1, 2, false, true, true);
        }
        else if (index >= inventoryStart && index < 30)
        {
            moved = moveItemStackTo(item, 30, 39, false, true, true);
        }
        else if (index >= 30 && index < 39)
        {
            moved = moveItemStackTo(item, inventoryStart, 30, false, true, true);
        }
        else
        {
            return empty;
        }
        if (!moved) return empty;
    }

    if (item->isEmpty()) slot->set(ItemInstance::EMPTY);
    else slot->setChanged();
    if (item->getCount() == original->getCount()) return empty;
    slot->onTake(player, item);
    return original;
}

not_null_ptr<ItemInstance> FurnaceMenu::clicked(int slot, int button, ClickType::eClickType type,
                                                boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> result = AbstractContainerMenu::clicked(slot, button, type, player);
    if (type == ClickType::eClickType_Swap && !result->isEmpty() &&
        (button == 0 || button == 1))
    {
        fjCheckAchievements(player, result);
        mContainer->setContainerData(4, 0);
    }
    return result;
}

void FurnaceMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    mContainer->stopOpen(player);
}
