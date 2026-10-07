#include "game/ui/InventoryMenu.hpp"

#include "game/core/ArmorSlot.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/EquipmentSlot.hpp"
#include "game/core/GenericStats.hpp"
#include "game/core/MemSect.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/ResultSlot.hpp"
#include "game/entity/mob/Mob.hpp"
#include "game/item/ArmorRecipes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipes.hpp"
#include "game/item/ResultContainer.hpp"
#include "game/minigame/MiniGameDef.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/generation/Player.hpp"

static const EquipmentSlot* gInventoryMenuArmorSlots[4];

InventoryMenu::InventoryMenu(boost::shared_ptr<Inventory> inventory, bool active, Player* player)
{
    mPlayer = player;
    _init(inventory, active);
}

void InventoryMenu::staticCtor()
{
    gInventoryMenuArmorSlots[3] = EquipmentSlot::FEET;
    gInventoryMenuArmorSlots[1] = EquipmentSlot::CHEST;
    gInventoryMenuArmorSlots[0] = EquipmentSlot::HEAD;
    gInventoryMenuArmorSlots[2] = EquipmentSlot::LEGS;
}

void InventoryMenu::_init(boost::shared_ptr<Inventory> inventory, bool active)
{
    mCraftSlots = boost::shared_ptr<CraftingContainer>(new CraftingContainer(this, 2, 2));
    mResultSlots = boost::shared_ptr<ResultContainer>(new ResultContainer());
    mActive = active;

    addSlot(new ResultSlot(mPlayer, mCraftSlots, mResultSlots, 0, 154, 28));

    int row;
    int column;
    for (row = 0; row < 2; ++row)
    {
        for (column = 0; column < 2; ++column)
        {
            addSlot(new Slot(mCraftSlots, column + row * 2, 98 + column * 18, 18 + row * 18));
        }
    }

    for (int armor = 0; armor < 4; ++armor)
    {
        addSlot(new ArmorSlot(gInventoryMenuArmorSlots[armor], inventory, 39 - armor, 8, 8 + armor * 18));
    }

    for (row = 0; row < 3; ++row)
    {
        for (column = 0; column < 9; ++column)
        {
            addSlot(new Slot(inventory, column + row * 9 + 9, 8 + column * 18, 84 + row * 18));
        }
    }

    for (column = 0; column < 9; ++column)
    {
        addSlot(new Slot(inventory, column, 8 + column * 18, 142));
    }

    addSlot(new OffhandSlot(inventory, 40, 77, 62));
    slotsChanged();
}

void InventoryMenu::slotsChanged()
{
    MemSect(0x17);

    not_null_ptr<ItemInstance> resultItem =
        Recipes::getInstance()->getItemFor(mCraftSlots, mPlayer->mLevel, NULL, Recipes::eRecipeCategory_Crafting);

    int craftColor = -1;
    int dyeColor = -1;
    for (unsigned int i = 0; i < mCraftSlots->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> ingredient = mCraftSlots->getItem(i);
        if (!ingredient->isEmpty())
        {
            if (ingredient->getItem() == resultItem->getItem() && craftColor == -1)
            {
                Block* block = Block::byItem(ingredient->getItem());
                if (Block::getId(block) - 219U < 16)
                {
                    craftColor = static_cast<ShulkerBoxBlock*>(block)->getDyeColor()->getBlockData();
                }
                else if (ingredient->getItem() == Items::bed)
                {
                    craftColor = ingredient->getAuxValue();
                }
            }
            if (ingredient->getItem() == Items::dye)
            {
                dyeColor = DyeColor::byItemData(ingredient->getAuxValue())->getItemData();
            }
        }
    }

    if (craftColor == dyeColor && craftColor != -1 && dyeColor != -1)
    {
        resultItem = ItemInstance::EMPTY;
    }

    mResultSlots->setItem(0, resultItem);

    MemSect(0);
}

void InventoryMenu::removed(boost::shared_ptr<Player> player)
{
    AbstractContainerMenu::removed(player);
    clearContainer(player, mPlayer->mLevel, mCraftSlots);
    mResultSlots->setItem(0, ItemInstance::EMPTY);
}

bool InventoryMenu::stillValid(boost::shared_ptr<Player> player)
{
    return true;
}

not_null_ptr<ItemInstance> InventoryMenu::quickMoveStack(boost::shared_ptr<Player> player, int index)
{
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    Slot* slot = m_slots.at(index);
    Slot* headSlot = m_slots.at(5);
    Slot* chestSlot = m_slots.at(6);
    Slot* legsSlot = m_slots.at(7);
    Slot* feetSlot = m_slots.at(8);

    if (slot != NULL && slot->hasItem())
    {
        not_null_ptr<ItemInstance> item = slot->getItem();
        result = item->copy();
        const EquipmentSlot* equipmentSlot = Mob::getEquipmentSlotForItem(result);

        bool smallInventory = false;
        if (Minecraft::InMiniGame(eMiniGameId_Any, true) &&
            Minecraft::GetInstance()->GetMiniGame()->IsSmallInventoryEnabled())
        {
            smallInventory = true;
        }

        if (index == 0)
        {
            if (!moveItemStackTo(item, 9, 45, true, true, true))
            {
                return ItemInstance::EMPTY;
            }
            slot->onQuickCraft(item, result);
        }
        else if ((unsigned int)(index - 1) < 4)
        {
            if (!moveItemStackTo(item, smallInventory ? 36 : 9, 45, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if ((unsigned int)(index - 5) < 4)
        {
            if (!moveItemStackTo(item, smallInventory ? 36 : 9, 45, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (equipmentSlot->getType() == EquipmentSlot::ARMOR && !m_slots.at(8 - equipmentSlot->getIndex())->hasItem())
        {
            int armorIndex = 8 - equipmentSlot->getIndex();
            if (!moveItemStackTo(item, armorIndex, armorIndex + 1, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (equipmentSlot == EquipmentSlot::OFFHAND && !m_slots.at(45)->hasItem())
        {
            if (!moveItemStackTo(item, 45, 46, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if ((unsigned int)(index - 9) < 27)
        {
            if (ArmorRecipes::GetArmorType(item->getItem()) == 1 && !headSlot->hasItem())
            {
                if (!moveItemStackTo(item, 5, 6, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 2 && !chestSlot->hasItem())
            {
                if (!moveItemStackTo(item, 6, 7, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 3 && !legsSlot->hasItem())
            {
                if (!moveItemStackTo(item, 7, 8, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 4 && !feetSlot->hasItem())
            {
                if (!moveItemStackTo(item, 8, 9, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (!moveItemStackTo(item, 36, 45, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if ((unsigned int)(index - 36) < 9)
        {
            if (ArmorRecipes::GetArmorType(item->getItem()) == 1 && !headSlot->hasItem())
            {
                if (!moveItemStackTo(item, 5, 6, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 2 && !chestSlot->hasItem())
            {
                if (!moveItemStackTo(item, 6, 7, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 3 && !legsSlot->hasItem())
            {
                if (!moveItemStackTo(item, 7, 8, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (ArmorRecipes::GetArmorType(item->getItem()) == 4 && !feetSlot->hasItem())
            {
                if (!moveItemStackTo(item, 8, 9, false, true, true))
                {
                    return ItemInstance::EMPTY;
                }
            }
            else if (!moveItemStackTo(item, smallInventory ? 36 : 9, 36, false, true, true))
            {
                return ItemInstance::EMPTY;
            }
        }
        else if (!moveItemStackTo(item, smallInventory ? 36 : 9, 45, false, true, true))
        {
            return ItemInstance::EMPTY;
        }

        if (item->isEmpty())
        {
            slot->set(ItemInstance::EMPTY);
        }
        else
        {
            slot->setChanged();
        }

        if (item->getCount() == result->getCount())
        {
            return ItemInstance::EMPTY;
        }

        not_null_ptr<ItemInstance> taken = slot->onTake(player, item);
        if (index == 0)
        {
            player->drop(taken, false);
        }
    }

    return result;
}

bool InventoryMenu::mayCombine(Slot* slot, not_null_ptr<ItemInstance> item)
{
    return slot->mayCombine(item);
}

bool InventoryMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot->mContainer != mResultSlots && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}

not_null_ptr<ItemInstance> InventoryMenu::clicked(int slot, int button, ClickType::eClickType type,
                                                  boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> result = AbstractContainerMenu::clicked(slot, button, type, player);

    static Item* ironArmor[4] = { Items::ironHelmet, Items::ironChestplate, Items::ironLeggings, Items::ironBoots };

    for (int armorSlot = 5; armorSlot < 9; ++armorSlot)
    {
        Slot* armorSlotObj = m_slots.at(armorSlot);
        bool wearingIron = false;
        if (armorSlotObj != NULL && armorSlotObj->hasItem())
        {
            not_null_ptr<ItemInstance> armorItem = armorSlotObj->getItem();
            if (armorItem->getItem() == ironArmor[armorSlot - 5])
            {
                wearingIron = true;
            }
        }
        if (!wearingIron)
        {
            return result;
        }
    }

    player->awardStat(GenericStats::ironMan(), GenericStats::param_ironMan());
    return result;
}

InventoryMenu::OffhandSlot::OffhandSlot(boost::shared_ptr<Container> container, int index, int x, int y)
    : Slot(container, index, x, y)
{
}

bool InventoryMenu::OffhandSlot::mayPlace(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
        return false;
    Item* itemType = item->getItem();
    return itemType == Items::filledMap || itemType == Items::arrow || itemType == Items::tippedArrow
        || itemType == Items::totemOfUndying;
}
