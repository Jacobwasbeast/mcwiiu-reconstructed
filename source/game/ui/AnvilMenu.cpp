#include "game/ui/AnvilMenu.hpp"
#include "game/core/StringIds.hpp"

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
#include "game/core/StringUtils.hpp"

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level,
                     const BlockPos& position, boost::shared_ptr<Player> player)
    : AbstractContainerMenu(), mResultSlots(), mInputSlots(),
      mPosition(), mItemName(), mPlayer()
{
    _init(inventory, level, position, player);
}

AnvilMenu::AnvilMenu(boost::shared_ptr<Inventory> inventory, Level* level,
                     boost::shared_ptr<Player> player)
    : AbstractContainerMenu(), mResultSlots(), mInputSlots(),
      mPosition(), mItemName(), mPlayer()
{
    _init(inventory, level, BlockPos::ZERO, player);
}

void AnvilMenu::_init(boost::shared_ptr<Inventory> inventory, Level* level,
                      const BlockPos& position, boost::shared_ptr<Player> player)
{
    mResultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    mInputSlots = boost::shared_ptr<RepairContainer>(new RepairContainer(this, IDS_REPAIR_AND_NAME, true, 2));
    mLevel = level;
    mPosition = position;
    mRepairCost = 0;
    mMaterialCost = 0;
    mPlayer = player;

    addSlot(new Slot(mInputSlots, 0, 27, 47));
    addSlot(new Slot(mInputSlots, 1, 76, 47));
    addSlot(new RepairResultSlot(this, position, mResultSlots, 2, 134, 47));

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
    mItemNameCensored = false;
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
    int baseCost = first->getBaseRepairCost() + (second->isEmpty() ? 0 : second->getBaseRepairCost());
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
    if (!mLevel->mIsClientSide)
    {
        clearContainer(player, mLevel, mInputSlots);
    }
}

bool AnvilMenu::stillValid(boost::shared_ptr<Player> player)
{
    if (mLevel->getBlockState(mPosition)->getBlock() == Blocks::anvil)
    {
        if (player->distanceAsIfHitscanned(mPosition, *player->mBoundingBox) <= 30.25f)
            return true;
    }
    return false;
}

not_null_ptr<ItemInstance> AnvilMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> clicked = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> stack = slot->getItem();
        clicked = stack->copy();
        if (index == 2)
        {
            if (moveItemStackTo(stack, 3, 39, true, true, true) != true) return ItemInstance::EMPTY;
            player->OnTakeFromAnvil(clicked);
            slot->onQuickCraft(stack, clicked);
        }
        else if (index == 0 || index == 1)
        {
            if (moveItemStackTo(stack, 3, 39, false, true, true) != true) return ItemInstance::EMPTY;
        }
        else if ((unsigned int)(index - 3) < 36u)
        {
            if (moveItemStackTo(stack, 0, 2, false, true, true) != true) return ItemInstance::EMPTY;
        }
        if (stack->isEmpty()) slot->set(ItemInstance::EMPTY);
        else slot->setChanged();
        if (stack->getCount() == clicked->getCount()) return ItemInstance::EMPTY;
        slot->onTake(player, stack);
    }
    return clicked;
}

void AnvilMenu::setItemName(const std::wstring& name, bool censored)
{
    mItemName = name;
    mItemNameCensored = censored;
    if (getSlot(2)->hasItem())
    {
        not_null_ptr<ItemInstance> result = getSlot(2)->getItem();
        if (name.empty()) result->resetHoverName();
        else
        {
            result->setHoverName(mItemName);
            result->setCensorshipInfo(mPlayer->getUUID(), censored);
        }
    }
    createResult();
}
