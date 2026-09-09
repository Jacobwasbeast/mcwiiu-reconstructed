#include "game/ui/AnvilMenu.hpp"

#include "game/core/RepairResultSlot.hpp"
#include "game/data/ListTag.hpp"
#include "game/item/EnchantedBookItem.hpp"
#include "game/item/Enchantment.hpp"
#include "game/item/EnchantmentHelper.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/RepairContainer.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/network/net_minecraft_world_inventory__ContainerListener.hpp"
#include "game/world/Level.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/BlockState.hpp"
#include "game/world/generation/Player.hpp"

bool equalsIgnoreCase(const std::wstring& first, const std::wstring& second);

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level,
                     const BlockPos& position, boost::shared_ptr<Player> player)
    : AbstractContainerMenu(), mResultSlots(), mInputSlots(), mLevel(NULL),
      mPosition(), mRepairCost(0), mMaterialCost(0), mItemName(),
      mItemNameCensored(false), mPlayer()
{
    _init(inventory, level, position, player);
}

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level,
                     boost::shared_ptr<Player> player)
    : AbstractContainerMenu(), mResultSlots(), mInputSlots(), mLevel(NULL),
      mPosition(), mRepairCost(0), mMaterialCost(0), mItemName(),
      mItemNameCensored(false), mPlayer()
{
    _init(inventory, level, BlockPos::ZERO, player);
}

AnvilMenu::~AnvilMenu()
{
}

void AnvilMenu::_init(boost::shared_ptr<Inventory> inventory, Level* level,
                      const BlockPos& position, boost::shared_ptr<Player> player)
{
    mResultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    mInputSlots = boost::shared_ptr<RepairContainer>(new RepairContainer(this, 0xc60bb915, true, 2));
    mLevel = level;
    mPosition = position;
    mRepairCost = 0;
    mMaterialCost = 0;
    mItemName.clear();
    mItemNameCensored = false;
    mPlayer = player;

    boost::shared_ptr<Container> input = mInputSlots;
    boost::shared_ptr<Container> result = mResultSlots;
    boost::shared_ptr<Container> playerInventory = inventory;
    addSlot(new Slot(input, 0, 27, 47));
    addSlot(new Slot(input, 1, 76, 47));
    addSlot(new RepairResultSlot(this, mPosition, result, 2, 134, 47));

    int row;
    int column;
    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(playerInventory, column + row * 9 + 9,
                             8 + column * 18, 84 + row * 18));
        }
    }
    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(playerInventory, column, 8 + column * 18, 142));
    }
}

void AnvilMenu::createResult()
{
    not_null_ptr<ItemInstance> first = mInputSlots->getItem(0);
    int operationCost = 0;
    int renameCost = 0;
    mRepairCost = 1;
    if (first->isEmpty())
    {
        mResultSlots->setItem(0, ItemInstance::EMPTY);
        mRepairCost = 0;
        return;
    }

    not_null_ptr<ItemInstance> result = first->copy();
    not_null_ptr<ItemInstance> second = mInputSlots->getItem(1);
    boost::unordered_map<Enchantment*, int>* enchantments =
        EnchantmentHelper::getEnchantments(result);
    int baseCost = first->getBaseRepairCost();
    if (!second->isEmpty()) baseCost += second->getBaseRepairCost();
    mMaterialCost = 0;

    if (!second->isEmpty())
    {
        bool enchantedBook = second->getItem() == Items::enchantedBook &&
                             EnchantedBookItem::getEnchantments(second)->size() > 0;
        if (result->isDamageableItem() &&
            result->getItem()->isValidRepairItem(first, second))
        {
            int repairAmount = result->getDamageValue();
            int quarterDurability = result->getMaxDamage() / 4;
            if (quarterDurability < repairAmount) repairAmount = quarterDurability;
            if (repairAmount < 1)
            {
                mResultSlots->setItem(0, ItemInstance::EMPTY);
                mRepairCost = 0;
                return;
            }

            int used = 0;
            while (repairAmount > 0 && used < second->getCount())
            {
                result->setAuxValue(result->getDamageValue() - repairAmount);
                ++operationCost;
                ++used;
                repairAmount = result->getDamageValue();
                quarterDurability = result->getMaxDamage() / 4;
                if (quarterDurability < repairAmount) repairAmount = quarterDurability;
            }
            mMaterialCost = used;
        }
        else
        {
            if (!enchantedBook &&
                (result->getItem() != second->getItem() || !result->isDamageableItem()))
            {
                mResultSlots->setItem(0, ItemInstance::EMPTY);
                mRepairCost = 0;
                return;
            }

            if (result->isDamageableItem() && !enchantedBook)
            {
                int remainingFirst = first->getMaxDamage() - first->getDamageValue();
                int remainingSecond = second->getMaxDamage() - second->getDamageValue();
                int damage = result->getMaxDamage() -
                             (remainingFirst + remainingSecond + result->getMaxDamage() * 12 / 100);
                if (damage < 0) damage = 0;
                if (damage < result->getAuxValue())
                {
                    result->setAuxValue(damage);
                    operationCost = 2;
                }
            }

            boost::unordered_map<Enchantment*, int>* addedEnchantments =
                EnchantmentHelper::getEnchantments(second);
            bool appliedEnchantment = false;
            bool hadEnchantment = false;
            boost::unordered_map<Enchantment*, int>::iterator added;
            for (added = addedEnchantments->begin(); added != addedEnchantments->end(); ++added)
            {
                Enchantment* enchantment = added->first;
                int oldLevel = 0;
                boost::unordered_map<Enchantment*, int>::iterator current =
                    enchantments->find(enchantment);
                if (current != enchantments->end()) oldLevel = current->second;
                int level = oldLevel == added->second ? added->second + 1 : added->second;
                if (level < oldLevel) level = oldLevel;

                bool canApply = enchantment->canEnchant(result);
                if (mPlayer->mAbilities.mInstabuild || first->getItem() == Items::enchantedBook)
                    canApply = true;

                boost::unordered_map<Enchantment*, int>::iterator existing;
                for (existing = enchantments->begin(); existing != enchantments->end(); ++existing)
                {
                    if (existing->first != enchantment &&
                        !enchantment->isCompatibleWith(existing->first))
                    {
                        ++operationCost;
                        canApply = false;
                    }
                }

                if (canApply)
                {
                    appliedEnchantment = true;
                    if (enchantment->getMaxLevel() < level) level = enchantment->getMaxLevel();
                    (*enchantments)[enchantment] = level;

                    int rarityCost = 0;
                    if (enchantment->getRarity()->getWeight() == Rarity::COMMON->getWeight())
                        rarityCost = 1;
                    else if (enchantment->getRarity()->getWeight() == Rarity::UNCOMMON->getWeight())
                        rarityCost = 2;
                    else if (enchantment->getRarity()->getWeight() == Rarity::RARE->getWeight())
                        rarityCost = 4;
                    else if (enchantment->getRarity()->getWeight() == Rarity::EPIC->getWeight())
                        rarityCost = 8;
                    if (enchantedBook)
                    {
                        rarityCost >>= 1;
                        if (rarityCost == 0) rarityCost = 1;
                    }
                    operationCost += rarityCost * level;
                    if (first->getCount() > 1) operationCost = 40;
                }
                else
                {
                    hadEnchantment = true;
                }
            }
            delete addedEnchantments;

            if (hadEnchantment && !appliedEnchantment)
            {
                mResultSlots->setItem(0, ItemInstance::EMPTY);
                mRepairCost = 0;
                return;
            }
        }
    }

    if (mItemName.empty())
    {
        if (first->hasCustomHoverName())
        {
            renameCost = 1;
            ++operationCost;
            result->resetHoverName();
        }
    }
    else if (!equalsIgnoreCase(mItemName, first->getHoverNameUnfiltered()))
    {
        renameCost = 1;
        ++operationCost;
        result->setHoverName(mItemName);
        result->setCensorshipInfo(mPlayer->getUUID(), mItemNameCensored);
    }

    mRepairCost = baseCost + operationCost;
    if (operationCost < 1) result = ItemInstance::EMPTY;
    if (mRepairCost > 39)
    {
        if (renameCost == operationCost && renameCost != 0)
            mRepairCost = 39;
        else if (!mPlayer->mAbilities.mInstabuild)
            result = ItemInstance::EMPTY;
    }

    if (!result->isEmpty())
    {
        int repairCost = result->getBaseRepairCost();
        if (!second->isEmpty() && repairCost < second->getBaseRepairCost())
            repairCost = second->getBaseRepairCost();
        if (renameCost != operationCost || renameCost == 0)
            repairCost = repairCost * 2 + 1;
        result->setRepairCost(repairCost);
        EnchantmentHelper::setEnchantments(enchantments, result);
    }

    mResultSlots->setItem(0, result);
    broadcastChanges();
}

void AnvilMenu::slotsChanged(boost::shared_ptr<Container> container)
{
    AbstractContainerMenu::slotsChanged();
    if (container.get() == mInputSlots.get()) createResult();
}

void AnvilMenu::addSlotListener(net_minecraft_world_inventory::ContainerListener* listener)
{
    AbstractContainerMenu::addSlotListener(listener);
    listener->setContainerData(this, 0, mRepairCost);
}

void AnvilMenu::setData(int id, int value)
{
    if (id == 0) mRepairCost = value;
}

void AnvilMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    if (mLevel != NULL && !mLevel->mIsClientSide)
    {
        boost::shared_ptr<Container> input = mInputSlots;
        clearContainer(player, mLevel, input);
    }
}

bool AnvilMenu::stillValid(boost::shared_ptr<Player> player)
{
    return mLevel->getBlockState(mPosition)->getBlock() == Blocks::anvil &&
           player->distanceAsIfHitscanned(mPosition, *player->mBoundingBox) <= 30.25f;
}

not_null_ptr<ItemInstance> AnvilMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    Slot* slot = m_slots.at(index);
    if (slot == NULL || !slot->hasItem()) return ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> copy = item->copy();
    bool moved;
    if (index == 2)
    {
        moved = moveItemStackTo(item, 3, (int)m_slots.size(), true, true, true) != 0;
        if (moved)
        {
            player->OnTakeFromAnvil(copy);
            slot->onQuickCraft(item, copy);
        }
    }
    else if (index < 3)
        moved = moveItemStackTo(item, 3, (int)m_slots.size(), false, true, true) != 0;
    else
        moved = moveItemStackTo(item, 0, 2, false, true, true) != 0;
    if (!moved) return ItemInstance::EMPTY;
    if (item->isEmpty()) slot->set(ItemInstance::EMPTY);
    else slot->setChanged();
    if (item->getCount() == copy->getCount()) return ItemInstance::EMPTY;
    slot->onTake(player, item);
    return copy;
}

void AnvilMenu::setItemName(const std::wstring& name, bool censored)
{
    mItemName = name;
    mItemNameCensored = censored;
    Slot* resultSlot = getSlot(2);
    if (resultSlot->hasItem())
    {
        not_null_ptr<ItemInstance> result = resultSlot->getItem();
        if (mItemName.empty()) result->resetHoverName();
        else
        {
            result->setHoverName(mItemName);
            result->setCensorshipInfo(mPlayer->getUUID(), censored);
        }
    }
    createResult();
}
