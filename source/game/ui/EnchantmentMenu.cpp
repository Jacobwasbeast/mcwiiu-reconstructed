#include "game/ui/EnchantmentMenu.hpp"

#include "game/core/DyeColor.hpp"
#include "game/core/Slot.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentCostSlot.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Items.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/generation/Player.hpp"

EnchantmentMenu::EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level, const BlockPos& pos)
    : AbstractContainerMenu(), mEnchantmentContainer(new EnchantmentContainer(this)), mLevel(level), mPosition(pos), mRandom(), mDataChanged(false)
{
    mCosts[0] = 0;
    mCosts[1] = 0;
    mCosts[2] = 0;
    mEnchantmentIds[0] = -1;
    mEnchantmentIds[1] = -1;
    mEnchantmentIds[2] = -1;
    mEnchantmentLevels[0] = -1;
    mEnchantmentLevels[1] = -1;
    mEnchantmentLevels[2] = -1;

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

    mEnchantmentSeed = static_cast<long long>(static_cast<int>(inventory->m_player->getEnchantmentSeed()));
}

EnchantmentMenu::EnchantmentMenu(boost::shared_ptr<Inventory> inventory, Level* level)
    : AbstractContainerMenu(), mEnchantmentContainer(new EnchantmentContainer(this)), mLevel(level), mPosition(BlockPos::ZERO), mRandom(), mDataChanged(false)
{
    mCosts[0] = 0;
    mCosts[1] = 0;
    mCosts[2] = 0;
    mEnchantmentIds[0] = -1;
    mEnchantmentIds[1] = -1;
    mEnchantmentIds[2] = -1;
    mEnchantmentLevels[0] = -1;
    mEnchantmentLevels[1] = -1;
    mEnchantmentLevels[2] = -1;

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

    mEnchantmentSeed = static_cast<long long>(static_cast<int>(inventory->m_player->getEnchantmentSeed()));
}

EnchantmentMenu::~EnchantmentMenu()
{
}

void EnchantmentMenu::broadcastData(net_minecraft_world_inventory::ContainerListener* listener)
{
    listener->setContainerData(this, 0, mCosts[0]);
    listener->setContainerData(this, 1, mCosts[1]);
    listener->setContainerData(this, 2, mCosts[2]);
    listener->setContainerData(this, 3, static_cast<int>(mEnchantmentSeed) & 0xfffffff0);
    listener->setContainerData(this, 4, mEnchantmentIds[0]);
    listener->setContainerData(this, 5, mEnchantmentIds[1]);
    listener->setContainerData(this, 6, mEnchantmentIds[2]);
    listener->setContainerData(this, 7, mEnchantmentLevels[0]);
    listener->setContainerData(this, 8, mEnchantmentLevels[1]);
    listener->setContainerData(this, 9, mEnchantmentLevels[2]);
}

void EnchantmentMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    broadcastData(listener);
}

void EnchantmentMenu::broadcastChanges()
{
    AbstractContainerMenu::broadcastChanges();

    if (mDataChanged)
    {
        for (std::vector<net_minecraft_world_inventory::ContainerListener*>::iterator it = m_listeners.begin();
             it != m_listeners.end(); ++it)
        {
            broadcastData(*it);
        }
        mDataChanged = false;
    }
}

void EnchantmentMenu::setData(int id, int value)
{
    unsigned int uid = static_cast<unsigned int>(id);
    if (uid < 3)
    {
        mDataChanged = true;
        mCosts[uid] = value;
    }
    else if (uid == 3)
    {
        mEnchantmentSeed = static_cast<long long>(value);
    }
    else if (uid - 4 < 3)
    {
        mEnchantmentIds[uid - 4] = value;
    }
    else if (uid - 7 < 3)
    {
        mEnchantmentLevels[uid - 7] = value;
    }
    else
    {
        AbstractContainerMenu::setData(id, value);
    }
}

std::vector<EnchantmentInstance*>* EnchantmentMenu::getEnchantmentList(not_null_ptr<ItemInstance> item, int enchantSlot, int cost)
{
    mRandom.setSeed(mEnchantmentSeed + static_cast<long long>(enchantSlot));
    std::vector<EnchantmentInstance*>* results = EnchantmentHelper::selectEnchantment(&mRandom, item, cost, false);
    if (results != NULL && item->getItem() == Items::book && results->size() > 1)
        results->erase(results->begin() + mRandom.nextInt(static_cast<int>(results->size())));
    return results;
}

void EnchantmentMenu::slotsChanged()
{
    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(0);
    if (item->isEmpty() || !item->isEnchantable())
    {
        for (int slot = 0; slot < 3; ++slot)
        {
            mCosts[slot] = 0;
            mEnchantmentIds[slot] = -1;
            mEnchantmentLevels[slot] = -1;
        }
        mDataChanged = true;
        return;
    }
    if (mLevel->mIsClientSide)
        return;

    int bookshelves = 0;
    for (int dz = -1; dz <= 1; ++dz)
        for (int dx = -1; dx <= 1; ++dx)
        {
            if (dx == 0 && dz == 0)
                continue;
            if (!mLevel->isEmptyBlock(mPosition.offset(dx, 0, dz)) || !mLevel->isEmptyBlock(mPosition.offset(dx, 1, dz)))
                continue;
            if (mLevel->getBlockState(mPosition.offset(dx * 2, 0, dz * 2))->getBlock() == Blocks::bookshelf)
                ++bookshelves;
            if (mLevel->getBlockState(mPosition.offset(dx * 2, 1, dz * 2))->getBlock() == Blocks::bookshelf)
                ++bookshelves;
            if (dx != 0 && dz != 0)
            {
                if (mLevel->getBlockState(mPosition.offset(dx * 2, 0, dz))->getBlock() == Blocks::bookshelf)
                    ++bookshelves;
                if (mLevel->getBlockState(mPosition.offset(dx * 2, 1, dz))->getBlock() == Blocks::bookshelf)
                    ++bookshelves;
                if (mLevel->getBlockState(mPosition.offset(dx, 0, dz * 2))->getBlock() == Blocks::bookshelf)
                    ++bookshelves;
                if (mLevel->getBlockState(mPosition.offset(dx, 1, dz * 2))->getBlock() == Blocks::bookshelf)
                    ++bookshelves;
            }
        }

    mRandom.setSeed(mEnchantmentSeed);
    for (int slot = 0; slot < 3; ++slot)
    {
        const int cost = EnchantmentHelper::getEnchantmentCost(&mRandom, slot, bookshelves, item);
        mEnchantmentIds[slot] = -1;
        mEnchantmentLevels[slot] = -1;
        mCosts[slot] = cost < slot + 1 ? 0 : cost;
    }

    for (int slot = 0; slot < 3; ++slot)
    {
        if (mCosts[slot] <= 0)
            continue;
        std::vector<EnchantmentInstance*>* results = getEnchantmentList(item, slot, mCosts[slot]);
        if (results == NULL)
            continue;
        if (!results->empty())
        {
            EnchantmentInstance* instance = (*results)[mRandom.nextInt(static_cast<int>(results->size()))];
            mEnchantmentIds[slot] = Enchantment::getId(instance->enchantment);
            mEnchantmentLevels[slot] = instance->level;
        }
        while (!results->empty())
        {
            delete results->back();
            results->pop_back();
        }
        delete results;
    }
    mDataChanged = true;
    broadcastChanges();
}

bool EnchantmentMenu::clickMenuButton(boost::shared_ptr<Player> player, int id)
{
    not_null_ptr<ItemInstance> item = mEnchantmentContainer->getItem(0);
    not_null_ptr<ItemInstance> lapis = mEnchantmentContainer->getItem(1);
    const int lapisRequired = id + 1;

    if ((lapis->isEmpty() || lapis->getCount() < lapisRequired) && !player->mAbilities.mInstabuild)
        return false;

    if (mCosts[id] <= 0 || item->isEmpty())
        return false;

    if ((player->mExperienceLevel < lapisRequired || player->mExperienceLevel < mCosts[id]) &&
        !player->mAbilities.mInstabuild)
        return false;

    if (!mLevel->mIsClientSide)
    {
        std::vector<EnchantmentInstance*>* results = getEnchantmentList(item, id, mCosts[id]);
        if (results != 0 && !results->empty())
        {
            player->onEnchantmentPerformed(lapisRequired);

            if (item->getItem() == Items::book)
            {
                not_null_ptr<ItemInstance> book(new ItemInstance(Items::enchantedBook));
                mEnchantmentContainer->setItem(0, book);
                for (std::vector<EnchantmentInstance*>::iterator it = results->begin();
                     it != results->end(); ++it)
                {
                    if (*it != 0)
                        EnchantedBookItem::addEnchantment(book, *it);
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

            while (!results->empty())
            {
                EnchantmentInstance* instance = results->back();
                results->pop_back();
                if (instance != 0)
                    delete instance;
            }
            delete results;

            if (!player->mAbilities.mInstabuild)
            {
                lapis->shrink(lapisRequired);
                if (lapis->isEmpty())
                    mEnchantmentContainer->setItem(1, ItemInstance::EMPTY);
            }

            mEnchantmentContainer->setChanged();
        }
        else if (results != 0)
        {
            delete results;
        }
    }

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
    if (!mLevel->mIsClientSide)
        clearContainer(player, mLevel, mEnchantmentContainer);
}

bool EnchantmentMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockState(mPosition)->getBlock() != Blocks::enchanting_table)
        return false;
    return player->distanceAsIfHitscanned(mPosition, *player->mBoundingBox) <= 30.25f;
}

not_null_ptr<ItemInstance> EnchantmentMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    Slot* slot = m_slots.at(index);
    Slot* itemSlot = m_slots.at(0);
    if (slot == NULL || !slot->hasItem())
        return ItemInstance::EMPTY;

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> copy = item->copy();

    if (index == 0 || index == 1)
    {
        if (!moveItemStackTo(item, 2, 29, true, true, true))
        {
            if (!moveItemStackTo(item, 29, 38, false, true, true))
                return ItemInstance::EMPTY;
        }
    }
    else if (item->getItem() == Items::dye && DyeColor::byItemData(item->getAuxValue()) == DyeColor::BLUE)
    {
        if (!moveItemStackTo(item, 1, 2, true, true, true))
            return ItemInstance::EMPTY;
    }
    else if (index - 2 < 27)
    {
        if (!item->isEnchantable() || itemSlot->hasItem())
        {
            if (!moveItemStackTo(item, 29, 38, false, true, true))
                return ItemInstance::EMPTY;
        }
        else
        {
            if (!moveItemStackTo(item, 0, 1, false, true, true))
                return ItemInstance::EMPTY;
        }
    }
    else if (index - 29 < 9)
    {
        if (!item->isEnchantable() || itemSlot->hasItem())
        {
            if (!moveItemStackTo(item, 2, 29, false, true, true))
                return ItemInstance::EMPTY;
        }
        else
        {
            if (!moveItemStackTo(item, 0, 1, false, true, true))
                return ItemInstance::EMPTY;
        }
    }
    else
    {
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
