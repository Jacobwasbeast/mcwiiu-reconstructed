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

const int InventoryMenu::ARMOR_SLOT_START = 5;
const int InventoryMenu::INV_SLOT_START = 9;
const int InventoryMenu::INV_SLOT_END = 36;
const int InventoryMenu::USE_ROW_SLOT_START = 36;

static const EquipmentSlot* gInventoryMenuHead = NULL;
static const EquipmentSlot* gInventoryMenuLegs = NULL;
static const EquipmentSlot* gInventoryMenuChest = NULL;
static const EquipmentSlot* gInventoryMenuFeet = NULL;

InventoryMenu::InventoryMenu(boost::shared_ptr<Inventory> inventory, bool active, Player* player)
    : mPlayer(player), mActive(active)
{
    _init(inventory, active);
}

void InventoryMenu::staticCtor()
{
    gInventoryMenuHead = EquipmentSlot::HEAD;
    gInventoryMenuLegs = EquipmentSlot::LEGS;
    gInventoryMenuChest = EquipmentSlot::CHEST;
    gInventoryMenuFeet = EquipmentSlot::FEET;
}

void InventoryMenu::_init(boost::shared_ptr<Inventory> inventory, bool active)
{
    mActive = active;
    mCraftSlots.reset(new CraftingContainer(this, 2, 2));
    mResultSlots.reset(new ResultContainer());

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

    addSlot(new ArmorSlot(gInventoryMenuHead, inventory, 39, 62, 8));
    addSlot(new ArmorSlot(gInventoryMenuChest, inventory, 38, 62, 26));
    addSlot(new ArmorSlot(gInventoryMenuLegs, inventory, 37, 62, 44));
    addSlot(new ArmorSlot(gInventoryMenuFeet, inventory, 36, 62, 62));

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
    Slot* slot = getSlot(index);
    if (slot == NULL || !slot->hasItem())
    {
        return ItemInstance::EMPTY;
    }

    not_null_ptr<ItemInstance> item = slot->getItem();
    not_null_ptr<ItemInstance> result = item->copy();
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
    else if ((unsigned int)(index - 1) < 4 || (unsigned int)(index - 5) < 4)
    {
        if (!moveItemStackTo(item, smallInventory ? 36 : 9, 45, false, true, true))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if (equipmentSlot->getType() == EquipmentSlot::ARMOR && !getSlot(8 - equipmentSlot->getIndex())->hasItem())
    {
        if (!moveItemStackTo(item, 8 - equipmentSlot->getIndex(), 9 - equipmentSlot->getIndex(), false, true, true))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if (equipmentSlot == EquipmentSlot::OFFHAND && !getSlot(45)->hasItem())
    {
        if (!moveItemStackTo(item, 45, 46, false, true, true))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if ((unsigned int)(index - 9) < 27)
    {
        int armorType = ArmorRecipes::GetArmorType(item->getItem());
        if (armorType == 1 && !getSlot(5)->hasItem())
        {
            if (!moveItemStackTo(item, 5, 6, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (armorType == 2 && !getSlot(6)->hasItem())
        {
            if (!moveItemStackTo(item, 6, 7, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (armorType == 3 && !getSlot(7)->hasItem())
        {
            if (!moveItemStackTo(item, 7, 8, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (armorType == 4 && !getSlot(8)->hasItem())
        {
            if (!moveItemStackTo(item, 8, 9, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (!moveItemStackTo(item, 36, 45, false, true, true))
        {
            return ItemInstance::EMPTY;
        }
    }
    else if ((unsigned int)(index - 36) < 9)
    {
        int armorType = ArmorRecipes::GetArmorType(item->getItem());
        if (armorType == 1 && !getSlot(5)->hasItem())
        {
            if (!moveItemStackTo(item, 5, 6, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (armorType == 2 && !getSlot(6)->hasItem())
        {
            if (!moveItemStackTo(item, 6, 7, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (armorType == 3 && !getSlot(7)->hasItem())
        {
            if (!moveItemStackTo(item, 7, 8, false, true, true)) return ItemInstance::EMPTY;
        }
        else if (armorType == 4 && !getSlot(8)->hasItem())
        {
            if (!moveItemStackTo(item, 8, 9, false, true, true)) return ItemInstance::EMPTY;
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

    if (item->getCount() != result->getCount())
    {
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
    return slot != NULL && slot->mayCombine(item);
}

bool InventoryMenu::canTakeItemForPickAll(not_null_ptr<ItemInstance> item, Slot* slot)
{
    return slot != NULL && slot != getSlot(0) && AbstractContainerMenu::canTakeItemForPickAll(item, slot);
}

not_null_ptr<ItemInstance> InventoryMenu::clicked(int slot, int button, ClickType::eClickType type,
                                                  boost::shared_ptr<Player> player)
{
    not_null_ptr<ItemInstance> result = AbstractContainerMenu::clicked(slot, button, type, player);

    static Item* const ironArmor[4] = { Items::ironHelmet, Items::ironChestplate, Items::ironLeggings, Items::ironBoots };

    for (int armorSlot = 5; armorSlot < 9; ++armorSlot)
    {
        Slot* armorSlotObj = getSlot(armorSlot);
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

InventoryMenu::OffhandSlot::~OffhandSlot()
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
