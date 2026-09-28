#include "game/ui/IUIScene_CraftingMenu.hpp"
#include "game/core/StringIds.hpp"

#include <cstring>
#include <cwchar>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/BannerPattern.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/DyeColor.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/HtmlString.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/data/CompoundTag.hpp"
#include "game/item/ArmorDyeRecipe.hpp"
#include "game/item/ArmorItem.hpp"
#include "game/item/BannerItem.hpp"
#include "game/item/BannerRecipes.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/DyeItem.hpp"
#include "game/item/FireworksRecipe.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Items.hpp"
#include "game/item/Recipe.hpp"
#include "game/item/Recipes.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/world/biome/arrayWithLength.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/block/Block.hpp"
#include "game/world/block/Blocks.hpp"
#include "game/world/block/ShulkerBoxBlock.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/Player.hpp"

namespace
{
struct InventoryItemCount
{
    unsigned int key;
    CompoundTag* tag;
    int count;
};
}

int IUIScene_CraftingMenu::m_craftingTabOrder[2][4] = {
    { 0, 1, 2, 3 },
    { 0, 1, 2, 3 },
};

int IUIScene_CraftingMenu::m_groupMap2x2[5][7] = {
    { 0, 1, 2, 4, 5, 6, -1 },
    { 0, 4, 5, -1, -1, -1, -1 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
};

int IUIScene_CraftingMenu::m_groupMap3x3[5][7] = {
    { 0, 1, 2, 3, 4, 5, 6 },
    { 0, 1, 2, 3, 4, 5, -1 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0 },
};

IUIScene_CraftingMenu::IUIScene_CraftingMenu()
{
    m_currentTab = 0;
    m_horizontalSlot = 0;
    m_verticalColumn = 1;
    for (int group = 0; group < 12; ++group)
    {
        m_recipeTable[group][0] = 0;
        m_recipeTable[group][1] = 0;
    }
    std::memset(m_recipeTable, 0, sizeof(m_recipeTable));
    m_recipePage = 0;
    m_verticalSlots[0] = 0;
    m_currentGroup = 0;
    m_verticalSlots[1] = 1;
    m_verticalSlots[2] = 2;
    m_customGridItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(9, ItemInstance::EMPTY);
    m_savedPanelDisplay = 0;
    m_selectedInventorySlot = -1;
    m_panelDisplay = 0;
    m_customRecipeValid = false;
    m_ingredientCount = 0;
    m_baseRecipeItem = ItemInstance::EMPTY;
    m_accumulatedItem = m_baseRecipeItem;
    m_craftingContainer = boost::shared_ptr<CraftingContainer>(new CraftingContainer(NULL, 3, 3));
}

IUIScene_CraftingMenu::~IUIScene_CraftingMenu()
{
    delete m_inventoryMenu;
}

int IUIScene_CraftingMenu::getGroupAtIndex(int index)
{
    return m_groupMap[m_currentTab * 7 + index];
}

int IUIScene_CraftingMenu::getCurrentGroup()
{
    return m_groupMap[m_currentTab * 7 + m_currentGroup];
}

int IUIScene_CraftingMenu::GetRecipeId(int group, int recipe)
{
    if (m_currentTab == 2)
        return m_recipeTable[m_currentGroup][2];
    if (m_currentTab != 3)
        return m_recipeTable[group][recipe + 2];

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    if (!m_recipeIngredients.empty())
    {
        item = m_recipeIngredients[m_customRecipeSlots[0]];
        item->setCount(1);
    }

    if (m_currentGroup != 0)
    {
        if (item->getItem() == Items::bed)
            return m_recipeTable[m_currentGroup][m_customRecipeSlots[1] + 2];

        int index = 0;
        Block* block = Block::byItem(item->getItem());
        if (block != NULL && dynamic_cast<ShulkerBoxBlock*>(block) != NULL)
            index = m_customRecipeSlots[1] + 16;
        return m_recipeTable[m_currentGroup][index + 2];
    }

    if (item->getItem() == Items::leatherHelmet)
        return m_recipeTable[m_currentGroup][2];
    if (item->getItem() == Items::leatherChestplate)
        return m_recipeTable[m_currentGroup][3];
    if (item->getItem() == Items::leatherLeggings)
        return m_recipeTable[m_currentGroup][4];
    if (item->getItem() == Items::leatherBoots)
        return m_recipeTable[m_currentGroup][5];

    int index = 0;
    if (item->getItem() == Items::leatherHorseArmor)
        index = 4;
    return m_recipeTable[m_currentGroup][index + 2];
}

bool IUIScene_CraftingMenu::isSpinnerGroup()
{
    return (m_currentGroup == 0 && m_currentTab == 1) || m_currentTab == 2 || m_currentTab == 3;
}

bool IUIScene_CraftingMenu::isInventoryItem()
{
    return isSpinnerGroup() && m_currentTab == 1 && m_horizontalSlot == (m_craftingMode == 1 ? 1 : 0);
}

bool IUIScene_CraftingMenu::isDuplicateItem()
{
    return isSpinnerGroup() && m_currentTab == 1 && m_horizontalSlot == (m_craftingMode == 1 ? 2 : 1);
}

bool IUIScene_CraftingMenu::isCustomRecipe()
{
    return m_currentTab == 2 || m_currentTab == 3;
}

int IUIScene_CraftingMenu::GetRecipeCount(int group)
{
    if (m_currentTab == 2 || m_currentTab == 3)
        return m_recipeTable[m_currentGroup][0];
    return m_recipeTable[group][0];
}

int IUIScene_CraftingMenu::getCurrentVSlotCount()
{
    if (isInventoryItem())
        return static_cast<int>(m_baseItemSlots.size());
    if (isDuplicateItem())
        return static_cast<int>(m_duplicateItemSlots.size());

    if (isCustomRecipe())
    {
        if (m_currentTab == 2)
        {
            if (m_currentGroup == 0)
            {
                if (m_horizontalSlot == 0)
                    return static_cast<int>(m_customRecipes[0].size());
                if (m_horizontalSlot == 1)
                    return static_cast<int>(m_customRecipes[1].size());
                return static_cast<int>(m_customRecipes[2].size());
            }
            if (m_currentGroup == 1)
            {
                if (m_horizontalSlot == 0)
                    return static_cast<int>(m_recipeIngredients.size());
                return static_cast<int>(m_customRecipes[0].size());
            }
            if (m_horizontalSlot == 0)
                return 1;
            return static_cast<int>(m_recipeIngredients.size());
        }
        if (m_currentTab == 3)
        {
            if (m_horizontalSlot == 0)
                return static_cast<int>(m_recipeIngredients.size());
            return static_cast<int>(m_customRecipeIngredients.size());
        }
    }
    return GetRecipeCount(m_horizontalSlot);
}

bool IUIScene_CraftingMenu::isItemSelected(int itemId)
{
    if (m_player == NULL || m_player->mInventory == NULL)
        return false;

    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    int count = getCurrentVSlotCount();
    if (count == 0)
        return false;

    int recipe = GetRecipeId(m_horizontalSlot, count > 1 ? m_verticalSlots[m_verticalColumn] : 0);
    return Item::VALIDgetId(ingredients[recipe].recipe->getResultItem()->getItem()) == itemId;
}

not_null_ptr<ItemInstance> IUIScene_CraftingMenu::GetBaseItemSlot(int column, int slot, bool& enabled)
{
    if (m_currentTab == 2)
    {
        if (static_cast<unsigned int>(slot) < m_customRecipes[column].size())
        {
            enabled = m_customRecipes[column][slot].second;
            return m_customRecipes[column][slot].first;
        }
    }
    else if (m_currentTab == 3 && static_cast<unsigned int>(slot) < m_customRecipeIngredients.size())
    {
        enabled = m_customRecipeIngredients[slot].second;
        return m_customRecipeIngredients[slot].first;
    }
    enabled = false;
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> IUIScene_CraftingMenu::GetSpinnerSlot(int spinner, int slot, bool& enabled)
{
    if (m_currentTab == 2)
    {
        if (m_currentGroup == 0)
        {
            if (spinner == 0)
            {
                enabled = m_customRecipes[0][slot].second;
                return m_customRecipes[0][slot].first;
            }
            if (spinner == 1)
            {
                enabled = m_customRecipes[1][slot].second;
                return m_customRecipes[1][slot].first;
            }
            enabled = m_customRecipes[2][slot].second;
            return m_customRecipes[2][slot].first;
        }

        if (m_currentGroup == 1)
        {
            if (spinner == 0)
            {
                not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
                if (!m_recipeIngredients.empty())
                    item = m_recipeIngredients[slot];
                enabled = true;
                return item;
            }
            enabled = m_customRecipes[0][slot].second;
            return m_customRecipes[0][slot].first;
        }

        if (spinner == 0)
        {
            enabled = m_hasGunpowder;
            return not_null_ptr<ItemInstance>(new ItemInstance(Items::gunpowder, 1));
        }
        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        if (!m_recipeIngredients.empty())
            item = m_recipeIngredients[slot];
        enabled = true;
        return item;
    }

    if (spinner == 0)
    {
        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        if (!m_recipeIngredients.empty())
            item = m_recipeIngredients[slot];
        enabled = true;
        return item;
    }
    enabled = m_customRecipeIngredients[slot].second;
    return m_customRecipeIngredients[slot].first;
}

int IUIScene_CraftingMenu::GetCustomIngredientCount(not_null_ptr<ItemInstance> item)
{
    int count = 0;
    if (item == NULL)
        return count;

    for (unsigned int slot = 0; slot < m_maintainedInventory->m_items.size(); ++slot)
    {
        not_null_ptr<ItemInstance>& inventoryItem = m_maintainedInventory->m_items[slot];
        if (inventoryItem->isEmpty())
            continue;
        if (inventoryItem->getItem() != item->getItem())
            continue;
        if (inventoryItem->getAuxValue() != item->getAuxValue())
            continue;
        if (inventoryItem->getTag() != NULL && item->getTag() != NULL && !ItemInstance::tagMatches(inventoryItem, item))
            continue;
        count += inventoryItem->getCount();
    }
    return count;
}

bool IUIScene_CraftingMenu::isValidCustomRecipeIngredient(int index, not_null_ptr<ItemInstance> item)
{
    int itemId = Item::getId(item->getItem());
    if (m_currentTab != 2)
    {
        if (m_currentTab != 3)
            return true;
        return GetCustomIngredientCount(item) != 0;
    }

    if (m_currentGroup == 1 && itemId == Item::getId(Items::fireworkCharge))
        return false;

    int featherId = Item::getId(Items::feather);
    int goldNuggetId = Item::getId(Items::goldNugget);
    int fireChargeId = Item::getId(Items::fireCharge);
    int skullId = Item::getId(Items::skull);
    for (unsigned int i = 0; !m_customIngredientSlots.empty() && i < m_customIngredientSlots.size() - 1; ++i)
    {
        int slot = m_customIngredientSlots[i];
        int slotItemId = Item::getId(m_customGridItems[slot]->getItem());
        if (index == slot || m_customGridItems[slot]->isEmpty())
            continue;
        if (itemId != featherId && itemId != goldNuggetId && itemId != fireChargeId && itemId != skullId)
            continue;
        if (slotItemId == featherId || slotItemId == goldNuggetId || slotItemId == fireChargeId || slotItemId == skullId)
            return false;
    }

    int gunpowderCount = 0;
    int gridSize = m_craftingMode == 0 ? 4 : 9;
    int gunpowderId = Item::getId(Items::gunpowder);
    for (int slot = 0; slot < gridSize; ++slot)
    {
        if (m_customGridItems[slot]->isEmpty() || m_customIngredientSlots.empty() || m_customIngredientSlots.back() == slot)
            continue;
        if (Item::getId(m_customGridItems[slot]->getItem()) == gunpowderId && itemId == gunpowderId)
        {
            ++gunpowderCount;
            if (gunpowderCount > 2)
                return false;
        }
    }
    return GetCustomIngredientCount(item) != 0;
}

int IUIScene_CraftingMenu::GetCustomIngredientIndex(not_null_ptr<ItemInstance> item)
{
    if (item == NULL)
        return -1;

    for (unsigned int slot = 0; slot < m_maintainedInventory->m_items.size(); ++slot)
    {
        not_null_ptr<ItemInstance>& inventoryItem = m_maintainedInventory->m_items[slot];
        if (inventoryItem->isEmpty())
            continue;
        if (inventoryItem->getItem() != item->getItem())
            continue;
        if (inventoryItem->getAuxValue() != item->getAuxValue())
            continue;
        if (ItemInstance::tagMatches(m_maintainedInventory->m_items[slot], item))
            return static_cast<int>(slot);
    }
    return -1;
}

void IUIScene_CraftingMenu::UpdateMaintainedInventory()
{
    m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    for (std::vector<std::pair<int, int> >::iterator it = m_stackedRecipes.begin(); it != m_stackedRecipes.end(); ++it)
        GameMode::RemoveRecipeItemsFromInventory(ingredients[it->first], m_maintainedInventory, ItemInstance::EMPTY, false);

    if (m_customIngredientSlots.empty())
        return;

    int gridSize = m_craftingMode == 0 ? 4 : 9;
    for (int slot = 0; slot < gridSize; ++slot)
    {
        if (m_craftingContainer->getItem(slot)->isEmpty())
            continue;

        int index = GetCustomIngredientIndex(m_craftingContainer->getItem(slot));
        if (index < 0)
        {
            setIngredientSlotRedBox(slot, true);
            m_customRecipeValid = false;
            continue;
        }
        GameMode::RemoveCustomRecipeItemsFromInventory(index, m_maintainedInventory);
    }
}

void IUIScene_CraftingMenu::DisplayCustomRecipeIngredients(int recipe)
{
    int gridSize = m_craftingMode == 0 ? 2 : 3;
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
    m_ingredientCount = required.numIngredients;
    not_null_ptr<ItemInstance> resultItem = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
    m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);

    int dyeSlot = 0;
    m_customRecipeValid = true;
    int baseColour = -1;
    int dyeColour = -1;
    for (int x = 0; x < gridSize; ++x)
    {
        for (int y = 0; y < gridSize; ++y)
        {
            int gridIndex = x + y * 3;
            int slot = x + y * gridSize;
            int gridEntry = required.craftingGrid[gridIndex];
            if (gridEntry == 0)
            {
                if (std::find(m_customIngredientSlots.begin(), m_customIngredientSlots.end(), slot)
                    == m_customIngredientSlots.end())
                {
                    setIngredientSlotRedBox(slot, false);
                    setIngredientSlotItem(getPad(), slot, ItemInstance::EMPTY);
                    m_craftingContainer->setItem(slot, ItemInstance::EMPTY);
                    continue;
                }

                not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
                bool enabled = false;
                not_null_ptr<ItemInstance> spinnerItem =
                    GetSpinnerSlot(m_horizontalSlot, m_customRecipeSlots[m_horizontalSlot], enabled);
                if (m_customIngredientSlots.back() == slot && !spinnerItem->isEmpty())
                {
                    item = spinnerItem->copy();
                    item->setCount(1);
                    if (isValidCustomRecipeIngredient(slot, item))
                    {
                        setIngredientSlotRedBox(slot, !enabled);
                        m_customRecipeValid = enabled;
                        m_customGridItems[slot] = item;
                        setIngredientSlotItem(getPad(), slot, item);
                    }
                    else
                    {
                        enabled = false;
                        setIngredientSlotRedBox(slot, true);
                        m_customRecipeValid = enabled;
                        m_customGridItems[slot] = item;
                        setIngredientSlotItem(getPad(), slot, item);
                        item = ItemInstance::EMPTY;
                    }
                }
                else
                {
                    item = m_customGridItems[slot];
                    setIngredientSlotItem(getPad(), slot, item);
                }
                m_craftingContainer->setItem(slot, item);
                continue;
            }

            not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
            if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1)
                && (required.missingSlotMask[getPad()] & (1 << gridIndex)) != 0)
            {
                setIngredientSlotRedBox(slot, true);
                m_customRecipeValid = false;
            }
            else
            {
                setIngredientSlotRedBox(slot, false);
            }

            unsigned int itemId = gridEntry & 0xffffff;
            int blockId = 0;
            if (m_currentTab == 2)
            {
                if (resultItem->getItem() == Items::fireworkCharge && m_customIngredientSlots.empty())
                {
                    if (itemId == static_cast<unsigned int>(Item::getId(Items::dye)))
                    {
                        if (m_currentGroup == 0 || m_currentGroup == 1)
                        {
                            bool enabled = false;
                            item = GetBaseItemSlot(0, m_customRecipeSlots[m_currentGroup != 0 ? 1 : 0], enabled);
                            setIngredientSlotRedBox(slot, !enabled);
                            m_customRecipeValid = enabled;
                        }
                    }
                    else if (itemId == static_cast<unsigned int>(Item::getId(Items::fireworkCharge))
                             && !m_recipeIngredients.empty() && (m_currentGroup == 1 || m_currentGroup == 2))
                    {
                        item = m_recipeIngredients[m_customRecipeSlots[m_currentGroup != 1 ? 1 : 0]];
                        item->setCount(1);
                    }
                }
            }
            else if (m_currentTab == 3 && m_customIngredientSlots.empty())
            {
                if (!m_recipeIngredients.empty())
                {
                    Block* block = Block::byItem(m_recipeIngredients[m_customRecipeSlots[0]]->getItem());
                    blockId = Block::getId(block);
                    if (static_cast<unsigned int>(blockId - Block::getId(Blocks::white_shulker_box)) < 16)
                        baseColour = static_cast<ShulkerBoxBlock*>(block)->getDyeColor()->getBlockData();
                }

                bool useBaseItem = false;
                if (itemId == static_cast<unsigned int>(Item::getId(Items::bed)))
                {
                    if (!m_recipeIngredients.empty())
                        baseColour = m_recipeIngredients[m_customRecipeSlots[0]]->getAuxValue();
                    useBaseItem = true;
                }
                else if (itemId == static_cast<unsigned int>(Item::getId(Items::dye)))
                {
                    bool enabled = false;
                    item = GetBaseItemSlot(0, m_customRecipeSlots[1], enabled);
                    dyeColour = DyeColor::byItemData(item->getAuxValue())->getBlockData();
                    setIngredientSlotRedBox(slot, !enabled);
                    m_customRecipeValid = m_customRecipeValid && enabled;
                    dyeSlot = slot;
                }
                else if (itemId - Item::getId(Items::leatherHelmet) < 4
                         || static_cast<unsigned int>(blockId - Block::getId(Blocks::white_shulker_box)) < 16
                         || itemId == static_cast<unsigned int>(Item::getId(Items::leatherHorseArmor)))
                {
                    useBaseItem = true;
                }

                if (useBaseItem && !m_recipeIngredients.empty() && (m_currentGroup == 0 || m_currentGroup == 1))
                {
                    item = m_recipeIngredients[m_customRecipeSlots[0]];
                    item->setCount(1);
                }
            }

            if (item->isEmpty())
            {
                if (m_customIngredientSlots.empty())
                    item = not_null_ptr<ItemInstance>(
                        new ItemInstance(Item::byId(itemId), 1, static_cast<unsigned int>(gridEntry) >> 24));
                else
                    item = m_customGridItems[slot];
            }
            m_customGridItems[slot] = item;
            setIngredientSlotItem(getPad(), slot, item);
            m_craftingContainer->setItem(slot, item);
        }
    }

    if (baseColour == dyeColour && m_currentTab == 3)
    {
        setIngredientSlotRedBox(dyeSlot, true);
        m_customRecipeValid = false;
    }

    UpdateMaintainedInventory();
    if (m_currentTab == 2)
    {
        if (m_customRecipeValid)
        {
            bool hasPaper = false;
            bool hasGunpowder = false;
            bool hasFireworkCharge = false;
            FireworksRecipe::updatePossibleRecipes(m_craftingContainer, &hasPaper, &hasGunpowder, &hasFireworkCharge);
            if (!hasPaper && !hasGunpowder && !hasFireworkCharge)
            {
                m_customRecipeValid = false;
                setIngredientSlotHighlighted(-1);
                setCraftingOutputSlotRedBox(!m_customRecipeValid);
                return;
            }
        }
    }
    else if (m_currentTab == 3 && m_customRecipeValid)
    {
        if (required.recipe->assemble(boost::shared_ptr<CraftingContainer>())->isEmpty())
            m_customRecipeValid = false;
    }

    if (!m_customRecipeValid || m_customIngredientSlots.empty())
        setIngredientSlotHighlighted(-1);
    else
        setIngredientSlotHighlighted(m_customIngredientSlots.back());
    setCraftingOutputSlotRedBox(!m_customRecipeValid);
}

void IUIScene_CraftingMenu::UpdateDescriptionText(bool show)
{
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    if (!show)
    {
        if (static_cast<unsigned int>(m_currentTab - 1) < 3)
            setDescriptionTextTop(L"");
        else
            setDescriptionText(L"");
        return;
    }

    int verticalSlot = 0;
    if (getCurrentVSlotCount() > 1)
        verticalSlot = m_verticalSlots[m_verticalColumn];

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    if (isInventoryItem())
    {
        item = m_player->mInventory->m_items[m_baseItemSlots[verticalSlot]];
    }
    else if (isDuplicateItem())
    {
        item = m_player->mInventory->m_items[m_duplicateItemSlots[verticalSlot].first];
    }
    else if (m_currentTab == 2)
    {
        item = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mFireworksRecipe,
                                                  Recipes::eRecipeCategory_Crafting);
        if (item->isEmpty())
            item = ingredients[GetRecipeId(m_horizontalSlot, verticalSlot)].recipe->assemble(
                boost::shared_ptr<CraftingContainer>());
    }
    else if (m_currentTab == 3)
    {
        Recipe* customRecipe = m_currentGroup == 0 ? Recipes::mArmorDyeRecipe : NULL;
        item = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, customRecipe,
                                                  Recipes::eRecipeCategory_Crafting);
        if (item->isEmpty())
            item = ingredients[GetRecipeId(m_horizontalSlot, verticalSlot)].recipe->assemble(
                boost::shared_ptr<CraftingContainer>());
    }
    else
    {
        item = ingredients[GetRecipeId(m_horizontalSlot, verticalSlot)].recipe->assemble(
            boost::shared_ptr<CraftingContainer>());
    }

    Item::VALIDgetId(item->getItem());
    int auxValue = item->getAuxValue();
    int baseItemType = item->getItem()->getBaseItemType();
    int descriptionId = item->getUseDescriptionId();
    if (baseItemType != 0 && baseItemType == 17 && auxValue == 0)
    {
        descriptionId = 0xCC953094;
    }
    else if (descriptionId == -1 || (descriptionId == -2 && static_cast<unsigned int>(m_currentTab - 1) >= 3))
    {
        setDescriptionText(L"");
        return;
    }

    std::wstring text = L"";
    if (static_cast<unsigned int>(m_currentTab - 1) < 3)
    {
        std::vector<HtmlString>* lines = item->getTooltipLines(boost::shared_ptr<Player>(), false);
        for (int line = static_cast<int>(lines->size()) - 1; line >= 0; --line)
            lines->at(line).mColour = eMinecraftColour_HTMLColor_Black;
        text = HtmlString::Compose(lines);
    }
    else
    {
        std::wstring description = L10N::GetString(descriptionId);
        text = gApp.FormatHTMLString(getPad(), description, -1);
    }

    bool smallText = m_compactInventory || (!gRenderer->IsHiDef() && !gRenderer->IsWidescreen());
    (void)smallText;

    wchar_t header[64];
    swprintf(header, 64, L"<font color=\"#%08x\"><P ALIGN=LEFT>", gApp.GetHTMLColour(eMinecraftColour_HTMLColor_Black));
    text = std::wstring(header).append(text) + L"</P>";

    if (static_cast<unsigned int>(m_currentTab - 1) < 3)
        setDescriptionTextTop(text.c_str());
    else
        setDescriptionText(text.c_str());
}

void IUIScene_CraftingMenu::CheckRecipesAvailable()
{
    UpdateMaintainedInventory();
    int groupCount = 0;
    std::memset(m_recipeTable, 0, sizeof(m_recipeTable));
    hideAllHSlots();
    if (m_player == NULL || m_player->mInventory == NULL)
        return;

    m_maintainedInventory->PrintContents();
    std::vector<Recipe*>* recipes = Recipes::getInstance()->getRecipes(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    Recipe::INGREDIENTS_REQUIRED* required =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    int recipeCount = static_cast<int>(recipes->size());

    std::vector<InventoryItemCount> inventoryCounts;
    std::vector<std::wstring>* ignoredTags = new std::vector<std::wstring>();
    ignoredTags->push_back(L"ench");
    ignoredTags->push_back(L"RepairCost");
    ignoredTags->push_back(L"ownerID");
    ignoredTags->push_back(L"createdOnHost");
    ignoredTags->push_back(L"createdByRestricted");
    ignoredTags->push_back(L"display");
    ignoredTags->push_back(L"Name");

    for (unsigned int slot = 0; slot < m_player->mInventory->m_items.size(); ++slot)
    {
        not_null_ptr<ItemInstance>& item = m_player->mInventory->m_items[slot];
        if (item->isEmpty())
            continue;

        int auxValue = item->getAuxValue();
        if (item->isDamageableItem() && item->isDamaged())
            auxValue = 0;
        unsigned int key = Item::VALIDgetId(item->getItem()) | (auxValue << 16);

        bool merged = false;
        for (std::vector<InventoryItemCount>::iterator count = inventoryCounts.begin(); count != inventoryCounts.end(); ++count)
        {
            CompoundTag* countTag = count->tag;
            CompoundTag* itemTag = item->getTag();
            if (countTag != NULL && countTag->GetNumValidTagsAfterException(ignoredTags) == 0)
                countTag = NULL;
            if (itemTag != NULL && itemTag->GetNumValidTagsAfterException(ignoredTags) == 0)
                itemTag = NULL;
            if ((countTag == NULL || (itemTag != NULL && countTag->equalsWithException(itemTag, ignoredTags)))
                && count->key == key)
            {
                count->count += item->getCount();
                merged = true;
                break;
            }
        }
        if (!merged)
        {
            InventoryItemCount count;
            count.key = key;
            count.tag = item->getTag();
            count.count = item->getCount();
            inventoryCounts.push_back(count);
        }
    }

    for (std::vector<std::pair<unsigned int, int> >::iterator used = m_usedIngredients.begin(); used != m_usedIngredients.end();
         ++used)
    {
        unsigned int key = used->first;
        int remaining = used->second;
        for (std::vector<InventoryItemCount>::iterator count = inventoryCounts.begin(); count != inventoryCounts.end(); ++count)
        {
            if ((static_cast<int>(key) >> 16 < 0 && (count->key & 0xffff) == (key & 0xffff)) || count->key == key)
            {
                if (count->count < remaining)
                {
                    remaining -= count->count;
                    count->count = 0;
                }
                else
                {
                    count->count -= remaining;
                    remaining = 0;
                }
            }
        }
    }

    int customRow = 0;
    for (int index = 0; index < recipeCount; ++index)
    {
        Recipe* recipe = (*recipes)[index];
        Recipe::INGREDIENTS_REQUIRED& ingredients = required[index];
        if (!recipe->displayInSimpleCrafting() || recipe->getGroup() != getCurrentGroup()
            || (m_craftingMode == 0 && ingredients.requiresCraftingTable == 1))
        {
            ingredients.available[getPad()] = false;
            continue;
        }

        ingredients.missingSlotMask[getPad()] = 0;
        bool* satisfied = new bool[ingredients.numIngredients];
        not_null_ptr<ItemInstance> resultItem = ingredients.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        for (int i = 0; i < ingredients.numIngredients; ++i)
        {
            satisfied[i] = false;
            int have = 0;
            Item* ingredientItem = Item::byId(ingredients.itemIds[i]);
            bool baseIngredient = false;
            if (ingredientItem != NULL && !m_accumulatedItem->isEmpty() && m_accumulatedItem->getItem() == ingredientItem)
            {
                baseIngredient = true;
            }
            else if (isCustomRecipe())
            {
                Block* block = Block::byItem(resultItem->getItem());
                if (block != NULL && dynamic_cast<ShulkerBoxBlock*>(block) != NULL)
                    baseIngredient = true;
            }

            if (baseIngredient)
            {
                have = 1;
            }
            else if (ingredients.auxValues[i] == -1)
            {
                for (std::vector<InventoryItemCount>::iterator count = inventoryCounts.begin(); count != inventoryCounts.end();
                     ++count)
                {
                    if ((count->key & 0xffff) == static_cast<unsigned int>(ingredients.itemIds[i]))
                        have += count->count;
                }
            }
            else
            {
                unsigned int key = ingredients.itemIds[i] | (ingredients.auxValues[i] << 16);
                CompoundTag* tag = ingredients.itemTags != NULL ? ingredients.itemTags[i] : NULL;
                for (std::vector<InventoryItemCount>::iterator count = inventoryCounts.begin(); count != inventoryCounts.end();
                     ++count)
                {
                    if (count->key != key)
                        continue;
                    if (tag == NULL || (count->tag != NULL && tag->equalsWithException(count->tag, ignoredTags)))
                    {
                        have = count->count;
                        break;
                    }
                }
            }

            int need = ingredients.itemCounts[i];
            if (have >= need)
            {
                satisfied[i] = true;
                continue;
            }
            if (satisfied[i])
                continue;

            int missing = need - have;
            for (int slot = 0; missing != 0; ++slot)
            {
                unsigned int gridEntry = ingredients.craftingGrid[slot];
                if ((gridEntry & 0xffffff) == static_cast<unsigned int>(ingredients.itemIds[i])
                    && (ingredients.auxValues[i] == -1 || static_cast<unsigned int>(ingredients.auxValues[i]) == gridEntry >> 24))
                {
                    ingredients.missingSlotMask[getPad()] |= 1 << slot;
                    --missing;
                }
            }
        }

        bool available = true;
        for (int i = 0; i < ingredients.numIngredients; ++i)
        {
            if (!satisfied[i])
            {
                available = false;
                break;
            }
        }
        ingredients.available[getPad()] = available;

        int baseItemType = recipe->mBaseItemType;
        if (isCustomRecipe())
        {
            if (m_currentTab == 2)
            {
                m_recipeTable[customRow][2] = index;
                ++m_recipeTable[customRow][0];
                ++customRow;
            }
            else if (static_cast<unsigned int>(baseItemType - 7) <= 3 || baseItemType == 60)
            {
                m_recipeTable[0][m_recipeTable[0][0] + 2] = index;
                ++m_recipeTable[0][0];
            }
            else if (baseItemType == 58 || baseItemType == 36)
            {
                m_recipeTable[1][m_recipeTable[1][0] + 2] = index;
                ++m_recipeTable[1][0];
            }
        }
        else if (groupCount <= m_horizontalSlotCount)
        {
            bool grouped = false;
            if (baseItemType != 0 || isSpinnerGroup())
            {
                for (int group = 0; group < groupCount; ++group)
                {
                    if (m_recipeTable[group][1] != baseItemType)
                        continue;
                    if (m_recipeTable[group][0] <= 47)
                    {
                        m_recipeTable[group][m_recipeTable[group][0] + 2] = index;
                        ++m_recipeTable[group][0];
                    }
                    grouped = true;
                    break;
                }
            }
            if (!grouped && groupCount < m_horizontalSlotCount)
            {
                m_recipeTable[groupCount][1] = baseItemType;
                m_recipeTable[groupCount][m_recipeTable[groupCount][0] + 2] = index;
                ++m_recipeTable[groupCount][0];
                ++groupCount;
            }
        }
        delete[] satisfied;
    }
}

int IUIScene_CraftingMenu::getNumSpinnerSlots()
{
    if (!isSpinnerGroup())
        return 0;

    if (m_currentTab == 1)
    {
        if (m_craftingMode == 1)
            return 3;
    }
    else if (m_currentTab == 2)
    {
        if (m_currentGroup == 0)
            return 3;
    }
    else if (m_currentTab != 3)
    {
        return 0;
    }
    return 2;
}

bool IUIScene_CraftingMenu::canAddMoreStacks()
{
    if (isSpinnerGroup()
        && ((!isInventoryItem() && !isDuplicateItem()) || (!isInventoryItem() && isDuplicateItem())))
        return true;
    if (m_accumulatedItem->isEmpty())
        return false;
    return BannerBlockEntity::getPatternCount(m_accumulatedItem) < 6;
}

void IUIScene_CraftingMenu::UpdateHorizontalSlots()
{
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    if (!isSpinnerGroup())
    {
        for (int slot = 0; slot < m_horizontalSlotCount; ++slot)
        {
            if (GetRecipeCount(slot) == 0)
                return;

            not_null_ptr<ItemInstance> item =
                ingredients[GetRecipeId(slot, 0)].recipe->assemble(boost::shared_ptr<CraftingContainer>());
            unsigned int flags = 0x1f;
            if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1)
                && (!ingredients[GetRecipeId(slot, 0)].available[getPad()] || (m_currentTab == 1 && !canAddMoreStacks())))
                flags = 0x10;
            if (item->getItem() == Items::clock || item->getItem() == Items::compass)
                item->setAuxValue(0xff);
            setCraftHSlotItem(getPad(), slot, item, flags);
        }
        return;
    }

    if (m_currentTab == 1)
    {
        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        if (m_craftingMode == 1)
        {
            not_null_ptr<ItemInstance> result =
                ingredients[GetRecipeId(0, 0)].recipe->assemble(boost::shared_ptr<CraftingContainer>());
            unsigned int flags = 0x1f;
            if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1)
                && (!ingredients[GetRecipeId(0, 0)].available[getPad()] || (m_currentTab == 1 && !canAddMoreStacks())))
                flags = 0x10;
            setSpecialSlotItem(getPad(), 0, result, flags);
        }

        if (!m_baseItemSlots.empty())
            item = m_player->mInventory->m_items[m_baseItemSlots[0]];
        setSpecialSlotItem(getPad(), m_craftingMode == 1 ? 1 : 0, item, 0x1f);

        item = ItemInstance::EMPTY;
        if (!m_duplicateItemSlots.empty())
            item = m_player->mInventory->m_items[m_duplicateItemSlots[0].first];
        setSpecialSlotItem(getPad(), m_craftingMode == 1 ? 2 : 1, item, 0x1f);
        return;
    }

    if (m_currentTab != 2 && m_currentTab != 3)
        return;

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int spinnerCount = getNumSpinnerSlots();
    for (int spinner = 0; spinner < spinnerCount; ++spinner)
    {
        bool enabled = false;
        item = GetSpinnerSlot(spinner, 0, enabled);
        unsigned int flags = 0x1f;
        if (!enabled || (m_currentGroup == 0 && spinner == 1 && !isValidCustomRecipeIngredient(8, item)))
            flags = 0x10;
        setSpecialSlotItem(getPad(), spinner, item, flags);
        item = ItemInstance::EMPTY;
    }
}

void IUIScene_CraftingMenu::UpdateVerticalSlots()
{
    hideAllVSlots();
    bool smallScreen = m_compactInventory || (!gRenderer->IsHiDef() && !gRenderer->IsWidescreen());
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));

    if (!isSpinnerGroup())
    {
        if (getCurrentVSlotCount() < 2)
            return;

        setShowCraftHSlot(m_horizontalSlot, false);
        int visibleSlots = getCurrentVSlotCount() > 2 ? 3 : 2;
        for (int slot = 0; slot < visibleSlots; ++slot)
        {
            if (smallScreen && slot != 1)
                continue;

            int recipe = GetRecipeId(m_horizontalSlot, m_verticalSlots[slot]);
            not_null_ptr<ItemInstance> item = ingredients[recipe].recipe->assemble(boost::shared_ptr<CraftingContainer>());
            unsigned int flags = 0x1f;
            if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1)
                && (!ingredients[GetRecipeId(m_horizontalSlot, m_verticalSlots[slot])].available[getPad()]
                    || (m_currentTab == 1 && !canAddMoreStacks())))
                flags = 0x10;
            if (item->getItem() == Items::clock || item->getItem() == Items::compass)
                item->setAuxValue(0xff);
            setCraftVSlotItem(getPad(), slot, item, flags);
            updateVSlotPositions(visibleSlots, slot);
        }
        return;
    }

    if (m_currentTab == 1)
    {
        int inventorySlot = 0;
        int recipeSlot = 0;
        int duplicateSlot = 0;
        if (isInventoryItem())
        {
            inventorySlot = m_verticalSlots[m_verticalColumn];
            setSpinnerTitle(L10N::GetString(IDS_BANNER_SELECT_BANNER));
        }
        else if (isDuplicateItem())
        {
            duplicateSlot = m_verticalSlots[m_verticalColumn];
            setSpinnerTitle(L10N::GetString(IDS_BANNER_COPY_BANNER));
        }
        else
        {
            recipeSlot = m_verticalSlots[m_verticalColumn];
            setSpinnerTitle(L10N::GetString(IDS_BANNER_CREATE_BANNER));
        }

        if (m_craftingMode == 1)
        {
            not_null_ptr<ItemInstance> result =
                ingredients[GetRecipeId(0, recipeSlot)].recipe->assemble(boost::shared_ptr<CraftingContainer>());
            unsigned int flags = 0x1f;
            if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1)
                && !ingredients[GetRecipeId(0, recipeSlot)].available[getPad()])
                flags = 0x10;
            setSpecialSlotItem(getPad(), 0, result, flags);
        }

        not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
        if (!m_baseItemSlots.empty())
            item = m_player->mInventory->m_items[m_baseItemSlots[inventorySlot]];
        setSpecialSlotItem(getPad(), m_craftingMode == 1 ? 1 : 0, item, 0x1f);

        item = ItemInstance::EMPTY;
        unsigned int flags = 0x1f;
        if (!m_duplicateItemSlots.empty())
        {
            item = m_player->mInventory->m_items[m_duplicateItemSlots[duplicateSlot].first];
            flags = m_duplicateItemSlots[duplicateSlot].second ? 0x1f : 0x10;
        }
        setSpecialSlotItem(getPad(), m_craftingMode == 1 ? 2 : 1, item, flags);
        return;
    }

    if (m_currentTab == 2)
    {
        if (m_currentGroup == 0)
        {
            if (m_horizontalSlot == 0)
                setSpinnerTitle(L10N::GetString(0xD0D94A48));
            else if (m_horizontalSlot == 1)
                setSpinnerTitle(L10N::GetString(0xDCCA69BA));
            else
                setSpinnerTitle(L10N::GetString(0x7A3F69FA));
        }
        else if (m_currentGroup == 1)
        {
            if (m_horizontalSlot == 0)
                setSpinnerTitle(L10N::GetString(0x80AC0CE1));
            else
                setSpinnerTitle(L10N::GetString(0x529A620A));
        }
        else
        {
            if (m_horizontalSlot == 0)
                setSpinnerTitle(L10N::GetString(0x3A0766FB));
            else
                setSpinnerTitle(L10N::GetString(0x80AC0CE1));
        }
    }
    else if (m_currentTab == 3)
    {
        if (m_currentGroup == 0)
            setSpinnerTitle(L10N::GetString(m_horizontalSlot == 0 ? 0xABD1187F : 0xD0D94A48));
        else
            setSpinnerTitle(L10N::GetString(m_horizontalSlot == 0 ? 0x47C65CD1 : 0xD0D94A48));
    }
    else
    {
        return;
    }

    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int spinnerCount = getNumSpinnerSlots();
    for (int spinner = 0; spinner < spinnerCount; ++spinner)
    {
        bool enabled = false;
        int slot = spinner == m_horizontalSlot ? m_verticalSlots[m_verticalColumn] : m_customRecipeSlots[spinner];
        item = GetSpinnerSlot(spinner, slot, enabled);
        unsigned int flags = 0x1f;
        if (!enabled || (m_currentGroup == 0 && spinner == 1 && !isValidCustomRecipeIngredient(8, item)))
            flags = 0x10;
        if (!item->isEmpty())
            setSpecialSlotItem(getPad(), spinner, item, flags);
        item = ItemInstance::EMPTY;
        if (spinner == m_horizontalSlot)
            m_customRecipeSlots[m_horizontalSlot] = m_verticalSlots[1];
    }
}

bool IUIScene_CraftingMenu::stackedCraftingAllowed()
{
    return m_currentTab == 1;
}

void IUIScene_CraftingMenu::DisplayIngredients()
{
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    hideAllIngredientsSlots();
    int slotCount = getCurrentVSlotCount();
    if (slotCount == 0 && m_currentTab != 2 && m_currentTab != 3)
    {
        setCraftingOutputSlotItem(getPad(), ItemInstance::EMPTY);
        setCraftingOutputSlotRedBox(false);
        m_ingredientCount = 0;
        int gridSlots = m_craftingMode == 0 ? 4 : 9;
        for (int slot = 0; slot < gridSlots; ++slot)
        {
            setIngredientSlotRedBox(slot, false);
            setIngredientSlotItem(getPad(), slot, ItemInstance::EMPTY);
        }
        return;
    }

    if (isInventoryItem() || isDuplicateItem())
    {
        int gridSize = m_craftingMode == 0 ? 2 : 3;
        for (int x = 0; x < gridSize; ++x)
        {
            for (int y = 0; y < gridSize; ++y)
            {
                setIngredientSlotRedBox(x + y * gridSize, false);
                setIngredientSlotItem(getPad(), x + y * gridSize, ItemInstance::EMPTY);
            }
        }

        int verticalSlot = m_verticalSlots[m_verticalColumn];
        if (isInventoryItem())
        {
            not_null_ptr<ItemInstance> item = m_player->mInventory->m_items[m_baseItemSlots[verticalSlot]]->copy();
            item->setCount(1);
            setIngredientSlotItem(getPad(), gridSize + 1, item);
            setCraftingOutputSlotItem(getPad(), item);
            setCraftingOutputSlotRedBox(false);
        }
        else
        {
            bool enabled = m_duplicateItemSlots[verticalSlot].second;
            not_null_ptr<ItemInstance> item =
                m_player->mInventory->m_items[m_duplicateItemSlots[verticalSlot].first]->copy();
            item->setCount(1);
            setIngredientSlotItem(getPad(), gridSize + 1, item);
            setCraftingOutputSlotItem(getPad(), item);
            setCraftingOutputSlotRedBox(!enabled);
            item = not_null_ptr<ItemInstance>(new ItemInstance(item->getItem(), 1, item->getAuxValue()));
            setIngredientSlotItem(getPad(), gridSize, item);
            setIngredientSlotRedBox(gridSize, !enabled);
        }
        setInventorySlotHighlighted(-1);
    }
    else if (m_currentTab == 2)
    {
        int recipe = GetRecipeId(-1, -1);
        DisplayCustomRecipeIngredients(recipe);
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
        bool available = required.available[getPad()];
        m_ingredientCount = required.numIngredients;
        not_null_ptr<ItemInstance> result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        if (available)
        {
            bool hasPaper = false;
            bool hasGunpowder = false;
            bool hasFireworkCharge = false;
            FireworksRecipe::updatePossibleRecipes(m_craftingContainer, &hasPaper, &hasGunpowder, &hasFireworkCharge);
            if (!hasPaper && !hasGunpowder && !hasFireworkCharge)
            {
                available = false;
                m_customRecipeValid = false;
            }
            else
            {
                result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mFireworksRecipe,
                                                            Recipes::eRecipeCategory_Crafting);
            }
        }
        setCraftingOutputSlotItem(getPad(), result);
        if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1))
            setCraftingOutputSlotRedBox(!available || !m_customRecipeValid);
        else
            setCraftingOutputSlotRedBox(false);
        setInventorySlotHighlighted(-1);
    }
    else if (m_currentTab == 3)
    {
        int recipe = GetRecipeId(-1, -1);
        DisplayCustomRecipeIngredients(recipe);
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
        bool available = required.available[getPad()];
        m_ingredientCount = required.numIngredients;
        not_null_ptr<ItemInstance> result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        if (available)
        {
            if (!m_customRecipeValid || result->isEmpty())
            {
                available = false;
                m_customRecipeValid = false;
            }
            else if (m_currentGroup == 0)
            {
                result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mArmorDyeRecipe,
                                                            Recipes::eRecipeCategory_Crafting);
            }
            else
            {
                result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, NULL,
                                                            Recipes::eRecipeCategory_Crafting);
            }
        }
        setCraftingOutputSlotItem(getPad(), result);
        if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1))
            setCraftingOutputSlotRedBox(!available || !m_customRecipeValid);
        else
            setCraftingOutputSlotRedBox(false);
        setInventorySlotHighlighted(-1);
    }
    else
    {
        int verticalSlot = slotCount > 1 ? m_verticalSlots[m_verticalColumn] : 0;
        int gridSize = m_craftingMode == 0 ? 2 : 3;
        int recipe = GetRecipeId(m_horizontalSlot, verticalSlot);
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
        bool available = required.available[getPad()];
        m_ingredientCount = required.numIngredients;
        for (int i = 0; i < required.numIngredients; ++i)
        {
            int itemId = required.itemIds[i];
            int auxValue = required.auxValues[i];
            CompoundTag* tag = required.itemTags != NULL ? required.itemTags[i] : NULL;
            not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(itemId), required.itemCounts[i], auxValue));
            if (tag != NULL)
            {
                std::vector<std::wstring>* ignoredTags = new std::vector<std::wstring>();
                ignoredTags->push_back(L"display");
                ignoredTags->push_back(L"Name");
                bool found = false;
                for (int x = 0; x < gridSize && !found; ++x)
                {
                    for (int y = 0; y < gridSize; ++y)
                    {
                        ItemInstance* ingredient = required.recipe->getIngredientItem(x, y);
                        if (!ingredient->isEmpty() && ingredient->hasTag()
                            && ingredient->getTag()->equalsWithException(tag, ignoredTags))
                        {
                            item = ingredient->copy();
                            found = true;
                            break;
                        }
                    }
                }
            }

            int descriptionId;
            if (auxValue != -1)
                descriptionId = item->getDescriptionId(-1);
            else if (itemId == Block::getId(Blocks::wool))
                descriptionId = IDS_ANY_WOOL;
            else if (itemId == Block::getId(Blocks::planks))
                descriptionId = IDS_ANY_PLANKS;
            else if (itemId == Item::getId(Items::dye))
            {
                auxValue = 1;
                descriptionId = IDS_ITEM_DYE_POWDER;
            }
            else if (itemId == Block::getId(Blocks::sandstone))
                descriptionId = IDS_ANY_SANDSTONE;
            else if (itemId == Block::getId(Blocks::red_sandstone))
                descriptionId = IDS_ANY_REDSANDSTONE;
            else if (itemId == Block::getId(Blocks::stonebrick))
                descriptionId = IDS_ANY_STONE_BRICK;
            else if (itemId == Block::getId(Blocks::quartz_block))
                descriptionId = IDS_ANY_QUARTZ_BLOCK;
            else if (itemId == Block::getId(Blocks::stone))
                descriptionId = IDS_TILE_ANY_STONE;
            else if (itemId == Block::getId(Blocks::wooden_slab))
                descriptionId = 0x68615D41;
            else
                descriptionId = item->getDescriptionId(-1);

            if (tag == NULL)
            {
                setIngredientDescriptionText(i, L10N::GetString(descriptionId));
            }
            else
            {
                std::vector<HtmlString>* lines = item->getTooltipLines(m_player, false);
                if (lines->size() < 2)
                {
                    std::wstring name = item->getHoverNameUnfiltered();
                    swprintf(m_ingredientDescriptionText[i], 128, L"%ls", name.c_str());
                    setIngredientDescriptionText(i, m_ingredientDescriptionText[i]);
                }
                else
                {
                    std::vector<HtmlString>* nameLines = new std::vector<HtmlString>();
                    nameLines->push_back(lines->at(0));
                    nameLines->push_back(lines->at(1));
                    swprintf(m_ingredientDescriptionText[i], 128, L"%ls\n%ls", nameLines->at(0).mText.c_str(),
                             nameLines->at(1).mText.c_str());
                    setIngredientDescriptionText(i, m_ingredientDescriptionText[i]);
                }
            }

            if (itemId == Item::getId(Items::clock) || itemId == Item::getId(Items::compass))
                auxValue = 0xff;
            item->setAuxValue(auxValue);
            setIngredientDescriptionItem(getPad(), i, item);
            setIngredientDescriptionRedBox(i, false);
        }

        not_null_ptr<ItemInstance> result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        for (int x = 0; x < gridSize; ++x)
        {
            for (int y = 0; y < gridSize; ++y)
            {
                int gridIndex = x + y * 3;
                int slot = x + y * gridSize;
                unsigned int gridEntry = required.craftingGrid[gridIndex];
                if (gridEntry == 0)
                {
                    setIngredientSlotRedBox(slot, false);
                    setIngredientSlotItem(getPad(), slot, ItemInstance::EMPTY);
                    m_craftingContainer->setItem(slot, ItemInstance::EMPTY);
                    continue;
                }

                int itemId = gridEntry & 0xffffff;
                int auxValue = gridEntry >> 24;
                not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
                if (itemId == Item::getId(Items::clock) || itemId == Item::getId(Items::compass))
                {
                    item = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(itemId), 1, 0xff));
                }
                else
                {
                    if (result->getItem() == Items::fireworkCharge && itemId == Item::getId(Items::dye))
                        auxValue = 1;
                    item = not_null_ptr<ItemInstance>(new ItemInstance(Item::byId(itemId), 1, auxValue));
                }

                if (!stackedCraftingAllowed())
                {
                    ItemInstance* ingredient = required.recipe->getIngredientItem(x, y);
                    if (!ingredient->isEmpty() && ingredient->hasTag())
                        item = ingredient->copy();
                }
                else if (item->getItem() == m_accumulatedItem->getItem())
                {
                    item = m_accumulatedItem;
                }

                setIngredientSlotItem(getPad(), slot, item);
                m_craftingContainer->setItem(slot, item);
                if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1)
                    && (required.missingSlotMask[getPad()] & (1 << gridIndex)) != 0)
                    setIngredientSlotRedBox(slot, true);
                else
                    setIngredientSlotRedBox(slot, false);
            }
        }

        if (m_currentTab == 1 && !isSpinnerGroup())
        {
            if (!canAddMoreStacks())
            {
                available = false;
                result = m_accumulatedItem;
            }
            else
            {
                result = BannerRecipes::mAddPatternRecipe->assemble(m_craftingContainer);
            }
        }
        if (result->getItem() == Items::clock || result->getItem() == Items::compass)
            result->setAuxValue(0xff);
        setCraftingOutputSlotItem(getPad(), result);
        if (!GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1))
            setCraftingOutputSlotRedBox(!available);
        else
            setCraftingOutputSlotRedBox(false);
        setInventorySlotHighlighted(-1);
    }

    if (m_currentTab == 1)
    {
        if (isInventoryItem())
        {
            setInventorySlotHighlighted(m_baseItemSlots[m_verticalSlots[m_verticalColumn]]);
            return;
        }
        int slot = -1;
        if (isDuplicateItem())
            slot = m_duplicateItemSlots[m_verticalSlots[m_verticalColumn]].first;
        setInventorySlotHighlighted(slot);
        return;
    }

    if (m_currentTab != 2 && m_currentTab != 3)
        return;

    bool enabled = false;
    not_null_ptr<ItemInstance> spinnerItem =
        GetSpinnerSlot(m_horizontalSlot, m_customRecipeSlots[m_horizontalSlot], enabled);
    for (unsigned int slot = 0; slot < m_maintainedInventory->m_items.size(); ++slot)
    {
        not_null_ptr<ItemInstance>& inventoryItem = m_maintainedInventory->m_items[slot];
        if (inventoryItem->isEmpty() || inventoryItem->getItem() != spinnerItem->getItem()
            || inventoryItem->getAuxValue() != spinnerItem->getAuxValue())
            continue;
        if (ItemInstance::tagMatches(m_maintainedInventory->m_items[slot], spinnerItem))
        {
            setInventorySlotHighlighted(slot);
            return;
        }
    }
}

bool IUIScene_CraftingMenu::canAddCustomRecipeIngredient(not_null_ptr<ItemInstance> item)
{
    if (item->isEmpty())
        return false;
    if (!m_customRecipeValid)
        return false;
    if (m_currentTab == 2 && m_currentGroup == 1 && m_horizontalSlot == 0)
        return false;
    if (m_currentTab == 3)
    {
        if (m_currentGroup == 0)
        {
            if (m_horizontalSlot == 0)
                return false;
        }
        else if (m_currentGroup == 1)
        {
            if (m_horizontalSlot == 0)
                return false;
            if (m_horizontalSlot == 1)
                return false;
        }
    }
    unsigned int gridSlots = m_craftingMode == 0 ? 4 : 9;
    return static_cast<unsigned int>(m_customIngredientSlots.size() + m_ingredientCount) < gridSlots;
}

void IUIScene_CraftingMenu::UpdateTooltips()
{
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    int slotCount = getCurrentVSlotCount();
    bool available = false;
    if (slotCount != 0)
    {
        int verticalSlot = slotCount > 1 ? m_verticalSlots[m_verticalColumn] : 0;
        available = ingredients[GetRecipeId(m_horizontalSlot, verticalSlot)].available[getPad()];
    }

    if (m_currentTab != 1)
    {
        if (m_currentTab == 2 || m_currentTab == 3)
        {
            bool enabled = false;
            not_null_ptr<ItemInstance> item =
                GetSpinnerSlot(m_horizontalSlot, m_customRecipeSlots[m_horizontalSlot], enabled);
            bool canAdd = canAddCustomRecipeIngredient(item) && enabled;
            int craftTooltip = m_customRecipeValid ? IDS_TOOLTIPS_CREATE : -1;
            int removeTooltip = m_customIngredientSlots.empty() ? -1 : IDS_TOOLTIPS_REMOVE;
            int addTooltip = canAdd ? IDS_TOOLTIPS_ADD : -1;
            gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, removeTooltip, addTooltip, -2,
                                             IDS_TOOLTIPS_TYPE, -2, IDS_TOOLTIPS_GROUP, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                             false);
        }
        else
        {
            gConsoleUIController.SetTooltips(getPad(), available ? IDS_TOOLTIPS_CREATE : -1, IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_INFO, -1, -2,
                                             IDS_TOOLTIPS_TYPE, -2, IDS_TOOLTIPS_GROUP, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                             false);
        }
        return;
    }

    if (m_currentGroup != 0)
    {
        int craftTooltip = m_stackedRecipes.empty() ? -1 : IDS_TOOLTIPS_CREATE;
        if (available && canAddMoreStacks())
            gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_REMOVE, IDS_TOOLTIPS_ADD, -2,
                                             IDS_TOOLTIPS_TYPE, -2, IDS_TOOLTIPS_GROUP, -1, -1, -1, -1, -1, -1, -1, -1, false,
                                             false);
        else
            gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, IDS_TOOLTIPS_REMOVE, -1, -2, IDS_TOOLTIPS_TYPE,
                                             -2, IDS_TOOLTIPS_GROUP, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
        return;
    }

    unsigned int verticalSlot = m_verticalSlots[m_verticalColumn];
    bool duplicateEnabled = false;
    bool duplicateItem = isDuplicateItem();
    if (duplicateItem && verticalSlot < m_duplicateItemSlots.size())
        duplicateEnabled = m_duplicateItemSlots[verticalSlot].second;

    bool canCraft = available && !isInventoryItem() && !duplicateEnabled;
    bool hasSelection = available || (isInventoryItem() && !m_baseItemSlots.empty())
                        || (duplicateItem && !m_duplicateItemSlots.empty());

    int craftTooltip;
    if (duplicateItem)
        craftTooltip = duplicateEnabled ? IDS_BANNER_COPY : -1;
    else
        craftTooltip = canCraft ? IDS_TOOLTIPS_CREATE : -1;
    if (!hasSelection)
        craftTooltip = -1;

    if (!duplicateItem)
    {
        if (!isInventoryItem())
        {
            if (available && canAddMoreStacks())
            {
                gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, -1, IDS_TOOLTIPS_ADD, -2,
                                                 IDS_TOOLTIPS_TYPE, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
                return;
            }
        }
        else if (!m_baseItemSlots.empty())
        {
            gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, -1, IDS_TOOLTIPS_SELECT, -2, IDS_TOOLTIPS_TYPE,
                                             -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
            return;
        }
    }
    else if (duplicateEnabled)
    {
        not_null_ptr<ItemInstance> banner =
            m_player->mInventory->getItem(m_duplicateItemSlots[verticalSlot].first);
        if (BannerBlockEntity::getPatternCount(banner) < 6)
        {
            gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, -1, IDS_TOOLTIPS_SELECT, -2, IDS_TOOLTIPS_TYPE,
                                             -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, false, false);
            return;
        }
    }
    gConsoleUIController.SetTooltips(getPad(), craftTooltip, IDS_TOOLTIPS_EXIT, -1, -1, -2, IDS_TOOLTIPS_TYPE, -1, -1, -1, -1,
                                     -1, -1, -1, -1, -1, -1, false, false);
}

void IUIScene_CraftingMenu::UpdateHighlight()
{
    updateHighlightAndScrollPositions();
    bool hasSelection = true;
    if (getCurrentVSlotCount() == 0 && m_currentTab != 2 && m_currentTab != 3)
        hasSelection = false;
    if (!hasSelection)
    {
        setItemText(L"");
        DisplayIngredients();
        UpdateMultiPanel(false);
        UpdateTooltips();
        UpdateDescriptionText(false);
        return;
    }

    std::wstring itemText;
    not_null_ptr<ItemInstance> item = ItemInstance::EMPTY;
    int descriptionId;
    if (isInventoryItem())
    {
        item = m_player->mInventory->m_items[m_baseItemSlots[m_verticalSlots[m_verticalColumn]]];
        descriptionId = item->getDescriptionId(-1);
    }
    else if (isDuplicateItem())
    {
        item = m_player->mInventory->m_items[m_duplicateItemSlots[m_verticalSlots[m_verticalColumn]].first];
        descriptionId = item->getDescriptionId(-1);
    }
    else
    {
        Recipe::INGREDIENTS_REQUIRED* ingredients =
            Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
        int verticalSlot = getCurrentVSlotCount() > 1 ? m_verticalSlots[m_verticalColumn] : 0;
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[GetRecipeId(m_horizontalSlot, verticalSlot)];
        item = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        descriptionId = item->getDescriptionId(-1);
        if (descriptionId == static_cast<int>(IDS_TILE_TORCH))
        {
            descriptionId = IDS_TILE_TORCHCOAL;
            if (required.auxValues[0] == 1)
                descriptionId = IDS_TILE_TORCHCHARCOAL;
        }
        else if (descriptionId == static_cast<int>(IDS_ITEM_FIREBALL))
        {
            descriptionId = 0x7B7CBFA4;
            if (required.auxValues[2] == 1)
                descriptionId = 0x44D7620C;
        }
    }

    if (m_currentTab == 1 && m_currentGroup != 0)
    {
        descriptionId = BannerItem::getFirstPatternName(item);
    }
    else if (item->hasTag())
    {
        std::vector<HtmlString>* lines = item->getTooltipLines(m_player, false);
        if (lines->size() > 1)
        {
            std::vector<HtmlString>* nameLines = new std::vector<HtmlString>();
            nameLines->push_back(lines->at(0));
            nameLines->push_back(lines->at(1));
            itemText = nameLines->at(0).mText + L"\n" + nameLines->at(1).mText;
        }
        else
        {
            itemText = item->getHoverNameUnfiltered();
        }
    }

    if (itemText.empty())
        setItemText(L10N::GetString(descriptionId));
    else
        setItemText(itemText.c_str());
    itemText = L"";
    DisplayIngredients();
    UpdateMultiPanel(false);
    UpdateTooltips();
    UpdateDescriptionText(hasSelection);
}

void IUIScene_CraftingMenu::UpdateSlotsAndHighlight()
{
    UpdateHorizontalSlots();
    UpdateVerticalSlots();
    UpdateHighlight();
}

void IUIScene_CraftingMenu::RemoveLastCustomRecipeIngredient()
{
    if (!m_customIngredientSlots.empty())
    {
        m_customGridItems[m_customIngredientSlots.back()] = ItemInstance::EMPTY;
        m_customIngredientSlots.pop_back();
    }
    DisplayCustomRecipeIngredients(GetRecipeId(-1, -1));

    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    if (m_currentTab == 2)
        result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mFireworksRecipe,
                                                    Recipes::eRecipeCategory_Crafting);
    else if (m_currentGroup == 0)
        result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mArmorDyeRecipe,
                                                    Recipes::eRecipeCategory_Crafting);
    else
        result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, NULL,
                                                    Recipes::eRecipeCategory_Crafting);
    setCraftingOutputSlotItem(getPad(), result);
    UpdateDescriptionText(true);
    CheckRecipesAvailable();
    UpdateSlotsAndHighlight();
    UpdateVerticalSlots();
}

void IUIScene_CraftingMenu::updateAccumulatedItem()
{
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    m_accumulatedItem = m_baseRecipeItem;
    int gridSize = m_craftingMode == 0 ? 2 : 3;
    for (std::vector<std::pair<int, int> >::iterator stacked = m_stackedRecipes.begin(); stacked != m_stackedRecipes.end();
         ++stacked)
    {
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[stacked->first];
        for (int x = 0; x < gridSize; ++x)
        {
            for (int y = 0; y < gridSize; ++y)
            {
                int slot = x + y * gridSize;
                unsigned int gridEntry = required.craftingGrid[x + y * 3];
                if (gridEntry == 0)
                {
                    m_craftingContainer->setItem(slot, ItemInstance::EMPTY);
                    continue;
                }

                not_null_ptr<ItemInstance> item(new ItemInstance(Item::byId(gridEntry & 0xffffff), 1, gridEntry >> 24));
                if (m_currentTab == 1 && !m_accumulatedItem->isEmpty() && item->getItem() == Items::banner)
                    item = m_accumulatedItem;
                m_craftingContainer->setItem(slot, item);
            }
        }

        not_null_ptr<ItemInstance> result = BannerRecipes::mAddPatternRecipe->assemble(m_craftingContainer);
        if (!result->isEmpty())
            m_accumulatedItem = result;
        else
            m_accumulatedItem = m_baseRecipeItem;
    }
    setPreviewSlotItem(getPad(), m_accumulatedItem);
}

int IUIScene_CraftingMenu::getCurrentVSlotMax()
{
    int slotCount = getCurrentVSlotCount();
    if (slotCount != 0)
        --slotCount;
    return slotCount;
}

const wchar_t* IUIScene_CraftingMenu::GetGroupNameText(int group)
{
    if (m_currentTab == 1)
    {
        switch (group)
        {
        case 1:
            return L10N::GetString(0x133E5CD4);
        case 2:
            return L10N::GetString(0xB35EE68B);
        case 3:
            return L10N::GetString(0xDD9417BA);
        case 4:
            return L10N::GetString(0xF8E4117F);
        case 5:
            return L10N::GetString(0x8078384B);
        default:
            return L"";
        }
    }
    if (m_currentTab == 2 || m_currentTab == 3)
        return L"";

    switch (group)
    {
    case 0:
        return L10N::GetString(0xE6A4F105);
    case 1:
        return L10N::GetString(0xA00B4CF0);
    case 2:
        return L10N::GetString(0xEF31F329);
    case 3:
        return L10N::GetString(0xF187DB6D);
    case 4:
        return L10N::GetString(0x89F72B83);
    case 5:
        return L10N::GetString(0x415962D0);
    default:
        if (group < 3)
            return L10N::GetString(0xEF31F329);
        return L10N::GetString(0xAC698282);
    }
}

void IUIScene_CraftingMenu::refreshAllSlots()
{
    CheckRecipesAvailable();
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[1] = 0;
    m_verticalSlots[2] = 1;
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    UpdateSlotsAndHighlight();
    setGroupText(GetGroupNameText(getCurrentGroup()));
}

void IUIScene_CraftingMenu::ClearCustomRecipeIngredients()
{
    if (m_customIngredientSlots.empty())
        return;

    m_customIngredientSlots.clear();
    for (int slot = 0; slot < 9; ++slot)
        m_customGridItems[slot] = ItemInstance::EMPTY;
    if (isCustomRecipe())
    {
        DisplayCustomRecipeIngredients(GetRecipeId(-1, -1));
        return;
    }
    setIngredientSlotHighlighted(-1);
}

void IUIScene_CraftingMenu::refreshBaseItemSlots(not_null_ptr<ItemInstance> item)
{
    m_baseItemSlots.clear();
    m_duplicateItemSlots.clear();
    m_availableRecipes.clear();
    NonNullList<not_null_ptr<ItemInstance> >& inventory = m_player->mInventory->m_items;

    if (m_currentTab == 1)
    {
        for (unsigned int slot = 0; slot < inventory.size(); ++slot)
        {
            if (inventory[slot]->getItem() != Items::banner)
                continue;

            if (BannerBlockEntity::getPatternCount(inventory[slot]) < 6)
                m_baseItemSlots.push_back(slot);
            if (BannerBlockEntity::getPatternCount(inventory[slot]) > 0)
            {
                const DyeColor* baseColor = BannerBlockEntity::getBaseColor(inventory[slot]);
                bool hasBlankBanner = false;
                for (unsigned int other = 0; other < inventory.size(); ++other)
                {
                    if (other != slot && inventory[other]->getItem() == Items::banner
                        && BannerBlockEntity::getPatternCount(inventory[other]) < 1
                        && BannerBlockEntity::getBaseColor(inventory[other]) == baseColor)
                    {
                        hasBlankBanner = true;
                        break;
                    }
                }
                m_duplicateItemSlots.push_back(std::pair<int, bool>(slot, hasBlankBanner));
            }
        }
        return;
    }

    if (m_currentTab == 3)
    {
        not_null_ptr<ItemInstance> dye = ItemInstance::EMPTY;
        not_null_ptr<ItemInstance> selected = ItemInstance::EMPTY;
        if ((m_currentGroup == 0 || m_currentGroup == 1) && !m_recipeIngredients.empty())
            selected = item;

        m_customRecipeIngredients.clear();
        m_recipeIngredients.clear();
        not_null_ptr<ItemInstance> ingredient(ItemInstance::EMPTY);
        not_null_ptr<ItemInstance> existing(ItemInstance::EMPTY);
        for (unsigned int slot = 0; slot < inventory.size(); ++slot)
        {
            bool dyeable = false;
            if (m_currentGroup == 0)
            {
                Item* armor = inventory[slot]->getItem();
                dyeable = armor == Items::leatherHelmet || armor == Items::leatherChestplate
                          || armor == Items::leatherLeggings || armor == Items::leatherBoots
                          || armor == Items::leatherHorseArmor;
            }
            else if (m_currentGroup == 1)
            {
                if (inventory[slot]->getItem() == Items::bed)
                {
                    dyeable = true;
                }
                else
                {
                    Block* block = Block::byItem(inventory[slot]->getItem());
                    dyeable = block != NULL && dynamic_cast<ShulkerBoxBlock*>(block) != NULL;
                }
            }
            if (!dyeable)
                continue;

            ingredient = inventory[slot]->copy();
            ingredient->setCount(1);
            bool alreadyListed = false;
            for (unsigned int index = 0; index < m_recipeIngredients.size(); ++index)
            {
                if (index == slot)
                    continue;
                existing = m_recipeIngredients[index];
                if (ingredient->sameItemWithTags(existing, NULL))
                {
                    alreadyListed = true;
                    break;
                }
            }
            if (!alreadyListed)
                m_recipeIngredients.push_back(ingredient);
        }

        if (m_currentGroup == 0 || m_currentGroup == 1)
        {
            bool found = false;
            if (!selected->isEmpty())
            {
                for (unsigned int index = 0; index < m_recipeIngredients.size(); ++index)
                {
                    if (selected->sameItemWithTags(m_recipeIngredients[index], NULL))
                    {
                        m_customRecipeSlots[0] = index;
                        if ((m_currentGroup == 0 || m_currentGroup == 1) && m_horizontalSlot == 0)
                            m_verticalSlots[1] = index;
                        found = true;
                        break;
                    }
                }
            }
            if (!found && static_cast<unsigned int>(m_customRecipeSlots[0]) >= m_recipeIngredients.size())
            {
                m_customRecipeSlots[0] = m_recipeIngredients.empty() ? 0 : static_cast<int>(m_recipeIngredients.size()) - 1;
                if ((m_currentGroup == 0 || m_currentGroup == 1) && m_horizontalSlot == 0)
                    m_verticalSlots[1] = m_customRecipeSlots[0];
            }
        }

        for (std::vector<const DyeColor*>::iterator color = DyeColor::COLORS.begin(); color != DyeColor::COLORS.end();
             ++color)
        {
            dye = not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, (*color)->getItemData()));
            bool owned = false;
            for (unsigned int slot = 0; slot < inventory.size(); ++slot)
            {
                if (inventory[slot]->getItem() == Items::dye
                    && inventory[slot]->getAuxValue() == (*color)->getItemData())
                {
                    owned = true;
                    break;
                }
            }
            m_customRecipeIngredients.push_back(std::pair<not_null_ptr<ItemInstance>, bool>(dye, owned));
        }
        return;
    }

    if (m_currentTab != 2)
        return;

    not_null_ptr<ItemInstance> choice = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> selected = ItemInstance::EMPTY;
    if ((m_currentGroup == 1 || m_currentGroup == 2) && !m_recipeIngredients.empty())
        selected = item;

    m_customRecipes[0].clear();
    m_customRecipes[1].clear();
    m_customRecipes[2].clear();
    m_recipeIngredients.clear();

    not_null_ptr<ItemInstance> ingredient = ItemInstance::EMPTY;
    not_null_ptr<ItemInstance> existing = ItemInstance::EMPTY;
    for (unsigned int slot = 0; slot < inventory.size(); ++slot)
    {
        if (inventory[slot]->getItem() != Items::fireworkCharge)
            continue;

        ingredient = inventory[slot]->copy();
        ingredient->setCount(1);
        bool alreadyListed = false;
        for (unsigned int index = 0; index < m_recipeIngredients.size(); ++index)
        {
            if (index == slot)
                continue;
            existing = m_recipeIngredients[index];
            if (ingredient->sameItemWithTags(existing, NULL))
            {
                alreadyListed = true;
                break;
            }
        }
        if (!alreadyListed)
            m_recipeIngredients.push_back(ingredient);
    }

    if (m_currentGroup == 1 || m_currentGroup == 2)
    {
        int& customSlot = m_customRecipeSlots[m_currentGroup != 1 ? 1 : 0];
        bool found = false;
        if (!selected->isEmpty())
        {
            for (unsigned int index = 0; index < m_recipeIngredients.size(); ++index)
            {
                if (selected->sameItemWithTags(m_recipeIngredients[index], NULL))
                {
                    customSlot = index;
                    if ((m_currentGroup == 1 && m_horizontalSlot == 0) || (m_currentGroup == 2 && m_horizontalSlot == 1))
                        m_verticalSlots[1] = index;
                    found = true;
                    break;
                }
            }
        }
        if (!found && static_cast<unsigned int>(customSlot) >= m_recipeIngredients.size())
        {
            customSlot = m_recipeIngredients.empty() ? 0 : static_cast<int>(m_recipeIngredients.size()) - 1;
            if ((m_currentGroup == 1 && m_horizontalSlot == 0) || (m_currentGroup == 2 && m_horizontalSlot == 1))
                m_verticalSlots[1] = customSlot;
        }
    }

    for (std::vector<const DyeColor*>::iterator color = DyeColor::COLORS.begin(); color != DyeColor::COLORS.end(); ++color)
    {
        choice = not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, (*color)->getItemData()));
        bool owned = false;
        for (unsigned int slot = 0; slot < inventory.size(); ++slot)
        {
            if (inventory[slot]->getItem() == Items::dye && inventory[slot]->getAuxValue() == (*color)->getItemData())
            {
                owned = true;
                break;
            }
        }
        m_customRecipes[0].push_back(std::pair<not_null_ptr<ItemInstance>, bool>(choice, owned));
    }

    std::vector<std::pair<Item*, int> > shapes;
    shapes.push_back(std::pair<Item*, int>(Items::fireCharge, 0));
    shapes.push_back(std::pair<Item*, int>(Items::goldNugget, 0));
    shapes.push_back(std::pair<Item*, int>(Items::skull, 0));
    shapes.push_back(std::pair<Item*, int>(Items::skull, 1));
    shapes.push_back(std::pair<Item*, int>(Items::skull, 2));
    shapes.push_back(std::pair<Item*, int>(Items::skull, 3));
    shapes.push_back(std::pair<Item*, int>(Items::skull, 4));
    shapes.push_back(std::pair<Item*, int>(Items::skull, 5));
    shapes.push_back(std::pair<Item*, int>(Items::feather, 0));
    for (unsigned int shape = 0; shape < shapes.size(); ++shape)
    {
        choice = not_null_ptr<ItemInstance>(new ItemInstance(shapes[shape].first, 1, shapes[shape].second));
        bool owned = false;
        for (unsigned int slot = 0; slot < inventory.size(); ++slot)
        {
            if (inventory[slot]->getItem() == shapes[shape].first
                && inventory[slot]->getAuxValue() == shapes[shape].second)
            {
                owned = true;
                break;
            }
        }
        m_customRecipes[1].push_back(std::pair<not_null_ptr<ItemInstance>, bool>(choice, owned));
    }

    std::vector<Item*> effects;
    effects.push_back(Items::glowstoneDust);
    effects.push_back(Items::diamond);
    for (unsigned int effect = 0; effect < effects.size(); ++effect)
    {
        choice = not_null_ptr<ItemInstance>(new ItemInstance(effects[effect], 1));
        bool owned = false;
        for (unsigned int slot = 0; slot < inventory.size(); ++slot)
        {
            if (inventory[slot]->getItem() == effects[effect])
            {
                owned = true;
                break;
            }
        }
        m_customRecipes[2].push_back(std::pair<not_null_ptr<ItemInstance>, bool>(choice, owned));
    }

    m_hasGunpowder = false;
    for (unsigned int slot = 0; slot < inventory.size(); ++slot)
    {
        if (inventory[slot]->getItem() == Items::gunpowder)
        {
            m_hasGunpowder = true;
            break;
        }
    }
}

void IUIScene_CraftingMenu::UpdateTabs(bool force)
{
    if (m_currentTab == 1)
    {
        if (m_currentGroup == 0)
        {
            if (m_panelDisplay != 3)
                m_savedPanelDisplay = m_panelDisplay;
            m_horizontalSlotCount = m_craftingMode == 1 ? 3 : 2;
            m_panelDisplay = 0;
        }
        else
        {
            m_horizontalSlotCount = 12;
            m_panelDisplay = 3;
        }
        m_groupCount = m_craftingMode == 1 ? 6 : 3;
    }
    else if (m_currentTab == 2)
    {
        m_horizontalSlotCount = m_currentGroup == 0 ? 3 : 2;
        m_panelDisplay = 0;
        m_groupCount = 3;
    }
    else if (m_currentTab == 3)
    {
        m_groupCount = 2;
        m_horizontalSlotCount = 2;
        m_panelDisplay = 0;
    }
    else
    {
        m_panelDisplay = m_savedPanelDisplay;
        m_groupCount = m_craftingMode == 1 ? 7 : 6;
        m_horizontalSlotCount = 12;
    }

    if (force)
    {
        showTabHighlight(m_currentGroup, false);
        m_currentGroup = 0;
        SetCraftingMenu(m_groupCount, 4, static_cast<unsigned int>(m_currentTab - 1) < 3);
        showTabHighlight(m_currentGroup, true);

        for (int group = 0; group < m_groupCount; ++group)
        {
            not_null_ptr<ItemInstance> tabItem = ItemInstance::EMPTY;
            if (m_currentTab == 1)
            {
                tabItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::banner, 1, DyeColor::WHITE->getItemData()));
                const BannerPattern* pattern = NULL;
                if (m_craftingMode == 0)
                {
                    if (group == 1)
                        pattern = BannerPattern::BORDER;
                    else if (group == 2)
                        pattern = BannerPattern::CREEPER;
                }
                else
                {
                    switch (group)
                    {
                    case 1:
                        pattern = BannerPattern::CROSS;
                        break;
                    case 2:
                        pattern = BannerPattern::DIAGONAL_LEFT;
                        break;
                    case 3:
                        pattern = BannerPattern::CIRCLE_MIDDLE;
                        break;
                    case 4:
                        pattern = BannerPattern::BORDER;
                        break;
                    case 5:
                        pattern = BannerPattern::CREEPER;
                        break;
                    default:
                        break;
                    }
                }
                if (pattern != NULL)
                    BannerBlockEntity::fjAddPatternToItem(tabItem, pattern, DyeColor::BLACK);
            }
            else if (m_currentTab == 2)
            {
                if (group == 0)
                {
                    tabItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::fireworkCharge, 1));
                }
                else if (group == 1)
                {
                    CompoundTag* tag = new CompoundTag();
                    CompoundTag* explosion = new CompoundTag();
                    tabItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::fireworkCharge, 1));
                    arrayWithLength<int> colors(1, true);
                    colors.data[0] = DyeItem::DYE_COLORS[DyeColor::RED->getItemData()];
                    explosion->putIntArray(L"Colors", colors);
                    tag->put(L"Explosion", explosion);
                    tabItem->setTag(tag);
                }
                else if (group == 2)
                {
                    tabItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::fireworks, 1));
                }
            }
            else if (m_currentTab == 3)
            {
                if (group == 0)
                {
                    tabItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::leatherChestplate, 1));
                    ArmorItem* armor = static_cast<ArmorItem*>(tabItem->getItem());
                    arrayWithLength<const float> diffuse =
                        DyeColor::byItemData(DyeColor::GREEN->getItemData())->getTextureDiffuseColors();
                    int red = static_cast<int>(diffuse.data[0] * 255.0f);
                    int green = static_cast<int>(diffuse.data[1] * 255.0f);
                    int blue = static_cast<int>(diffuse.data[2] * 255.0f);
                    armor->setColor(tabItem, (red * 256 + green) * 256 + blue);
                }
                else if (group == 1)
                {
                    tabItem = not_null_ptr<ItemInstance>(new ItemInstance(Items::dye, 1, DyeColor::CYAN->getItemData()));
                }
            }
            SetHTabItem(group, tabItem);
        }
    }

    for (int group = 0; group < m_groupCount; ++group)
    {
        if (m_currentTab == 1)
        {
            if (m_currentGroup == 0)
                setHTabEnabled(group, group == 0);
            else
                setHTabEnabled(group, group > 0);
        }
        else
        {
            setHTabEnabled(group, true);
        }
    }
    UpdateMultiPanel(true);
    UpdateTooltips();
    ShowSuperCraftingSlots(getNumSpinnerSlots());
    ShowDescriptionPanel(isSpinnerGroup());
}

void IUIScene_CraftingMenu::UpdateHTab()
{
    m_customRecipeSlots[2] = 0;
    m_customRecipeSlots[1] = 0;
    m_customRecipeSlots[0] = 0;
    ClearCustomRecipeIngredients();
    if (m_currentTab == 3)
        refreshBaseItemSlots(ItemInstance::EMPTY);
    showTabHighlight(m_currentGroup, true);
    if (!isCustomRecipe())
    {
        int horizontalSlot = m_horizontalSlot;
        if (GetRecipeCount(horizontalSlot) > 0)
            setShowCraftHSlot(horizontalSlot, true);
    }
    m_verticalColumn = 1;
    m_horizontalSlot = 0;
    UpdateTabs(false);
    refreshAllSlots();
}

bool IUIScene_CraftingMenu::isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance> item)
{
    unsigned int gridSlots = m_craftingMode == 0 ? 4 : 9;
    for (unsigned int slot = 0; slot < gridSlots; ++slot)
    {
        bool customSlot = false;
        for (unsigned int index = 0; index < m_customIngredientSlots.size(); ++index)
        {
            if (m_customIngredientSlots[index] == static_cast<int>(slot))
            {
                customSlot = true;
                break;
            }
        }
        if (!customSlot && ItemInstance::matches(m_craftingContainer->getItem(slot), item, true))
            return true;
    }
    return false;
}

void IUIScene_CraftingMenu::AddCustomRecipeIngredient(not_null_ptr<ItemInstance> item)
{
    int gridSize = m_craftingMode == 0 ? 2 : 3;
    bool placed = false;
    for (int y = 0; y < gridSize; ++y)
    {
        for (int x = 0; x < gridSize && !placed; ++x)
        {
            int slot = y * gridSize + x;
            if (!m_craftingContainer->getItem(slot)->isEmpty())
                continue;

            item->setCount(1);
            setIngredientSlotItem(getPad(), slot, item);
            m_craftingContainer->setItem(slot, item);
            m_customGridItems[slot] = item;
            m_customIngredientSlots.push_back(slot);
            placed = true;
        }
    }

    if (m_customIngredientSlots.size() == 1 && isCustomIngredientInBaseRecipe(item))
        GameMode::RemoveCustomRecipeItemsFromInventory(GetCustomIngredientIndex(item), m_maintainedInventory);

    DisplayCustomRecipeIngredients(GetRecipeId(-1, -1));
    not_null_ptr<ItemInstance> result = ItemInstance::EMPTY;
    if (m_currentTab == 2)
        result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mFireworksRecipe,
                                                    Recipes::eRecipeCategory_Crafting);
    else if (m_currentGroup == 0)
        result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, Recipes::mArmorDyeRecipe,
                                                    Recipes::eRecipeCategory_Crafting);
    else
        result = Recipes::getInstance()->getItemFor(m_craftingContainer, m_player->mLevel, NULL,
                                                    Recipes::eRecipeCategory_Crafting);
    setCraftingOutputSlotItem(getPad(), result);
    UpdateDescriptionText(true);
}

void IUIScene_CraftingMenu::CraftCurrentItem()
{
    Minecraft* minecraft = Minecraft::GetInstance();
    if (m_player == NULL || m_player->mInventory == NULL)
        return;

    bool stackedCraft = false;
    if (stackedCraftingAllowed())
    {
        if (!isSpinnerGroup())
        {
            stackedCraft = true;
        }
        else if (isInventoryItem() || isDuplicateItem())
        {
            unsigned int verticalSlot = m_verticalSlots[m_verticalColumn];
            if (isDuplicateItem() && verticalSlot < m_duplicateItemSlots.size()
                && m_duplicateItemSlots[verticalSlot].second)
                AddStackedCraftRecipe(false);
            stackedCraft = true;
        }
    }

    if (stackedCraft)
    {
        if (m_stackedRecipes.empty())
            return;

        minecraft->mGameModes[getPad()]->handleStackedCraftItem(m_stackedRecipes, m_currentTab, m_player);
        m_selectedInventorySlot = -1;
        m_baseRecipeItem = ItemInstance::EMPTY;
        m_accumulatedItem = ItemInstance::EMPTY;
        m_stackedRecipes.clear();
        m_usedIngredients.clear();
        refreshBaseItemSlots(ItemInstance::EMPTY);
        gConsoleUIController.PlayUISFX(SoundEvents::UI_CRAFT);
        showTabHighlight(m_currentGroup, false);
        m_currentGroup = 0;
        UpdateHTab();
        CheckRecipesAvailable();
        UpdateSlotsAndHighlight();
        refreshAllSlots();
        return;
    }

    if (!isCustomRecipe())
    {
        if (getCurrentVSlotCount() == 0)
            return;

        Recipe::INGREDIENTS_REQUIRED* ingredients =
            Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
        int recipe = GetRecipeId(m_horizontalSlot, m_verticalSlots[m_verticalColumn]);
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
        if (GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1) || required.available[getPad()])
        {
            minecraft->mGameModes[getPad()]->handleCraftItem(recipe, m_currentTab, m_player);
            gConsoleUIController.PlayUISFX(SoundEvents::UI_CRAFT);
            if (isSpinnerGroup())
                refreshBaseItemSlots(ItemInstance::EMPTY);
            CheckRecipesAvailable();
            UpdateSlotsAndHighlight();
            return;
        }

        not_null_ptr<ItemInstance> result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        if (minecraft->mGameModes[getPad()] != NULL)
        {
            Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
            if (tutorial != NULL)
                tutorial->createItemSelected(result, required.available[getPad()]);
        }
        gConsoleUIController.PlayUISFX(SoundEvents::UI_CRAFT_FAIL);
        return;
    }

    if (!m_customRecipeValid && !GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1))
    {
        gConsoleUIController.PlayUISFX(SoundEvents::UI_CRAFT_FAIL);
        return;
    }

    std::vector<int> customIngredients;
    m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);
    unsigned int gridSlots = m_craftingMode == 0 ? 4 : 9;
    for (unsigned int slot = 0; slot < gridSlots; ++slot)
    {
        int ingredient = GetCustomIngredientIndex(m_craftingContainer->getItem(slot));
        if (ingredient >= 0)
        {
            customIngredients.push_back(ingredient);
            GameMode::RemoveCustomRecipeItemsFromInventory(ingredient, m_maintainedInventory);
        }
    }
    not_null_ptr<ItemInstance> crafted =
        minecraft->mGameModes[getPad()]->handleCustomCraftItem(customIngredients, m_player);
    not_null_ptr<ItemInstance> selected = crafted->copy();
    selected->setCount(1);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_CRAFT);
    refreshBaseItemSlots(selected);
    CheckRecipesAvailable();
    UpdateSlotsAndHighlight();
}

void IUIScene_CraftingMenu::UpdateVTab()
{
    m_customRecipeSlots[2] = 0;
    m_customRecipeSlots[1] = 0;
    m_customRecipeSlots[0] = 0;
    ClearCustomRecipeIngredients();
    showTabHighlight(m_currentGroup, false);
    m_currentGroup = 0;
    refreshBaseItemSlots(ItemInstance::EMPTY);
    m_stackedRecipes.clear();
    m_usedIngredients.clear();
    if (!isCustomRecipe())
    {
        int horizontalSlot = m_horizontalSlot;
        if (GetRecipeCount(horizontalSlot) > 0)
            setShowCraftHSlot(horizontalSlot, true);
    }
    m_verticalColumn = 1;
    m_horizontalSlot = 0;
    UpdateTabs(true);
    refreshAllSlots();
    if (m_currentTab == 2)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[getPad()]->getTutorial();
        if (tutorial != NULL)
            tutorial->changeTutorialState(eTutorial_State_FireworksMenu, NULL);
    }
}

void IUIScene_CraftingMenu::NavTabUp()
{
    if (!isCustomRecipe())
    {
        int horizontalSlot = m_horizontalSlot;
        if (GetRecipeCount(horizontalSlot) > 0)
            setShowCraftHSlot(horizontalSlot, true);
    }
    const int* tabOrder = m_craftingTabOrder[m_craftingMode == 0 ? 0 : 1];
    if (m_currentTab < 1)
        m_currentTab = tabOrder[3];
    else
        m_currentTab = tabOrder[m_currentTab - 1];
    UpdateVTab();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::NavTabDown()
{
    if (!isCustomRecipe())
    {
        int horizontalSlot = m_horizontalSlot;
        if (GetRecipeCount(horizontalSlot) > 0)
            setShowCraftHSlot(horizontalSlot, true);
    }
    if (m_currentTab < 3)
        m_currentTab = m_craftingTabOrder[m_craftingMode == 0 ? 0 : 1][m_currentTab + 1];
    else
        m_currentTab = 0;
    UpdateVTab();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::ItemScrollRight()
{
    int previousSlot = m_horizontalSlot;
    int slot = previousSlot + 1;
    if (m_horizontalSlotCount <= slot)
        slot = 0;
    m_horizontalSlot = slot;
    m_verticalColumn = 1;
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[2] = 1;
    m_verticalSlots[1] = 0;
    if (m_currentTab == 2 || m_currentTab == 3)
        m_verticalSlots[1] = m_customRecipeSlots[slot];
    UpdateVerticalSlots();
    UpdateHighlight();
    if (!isCustomRecipe() && GetRecipeCount(previousSlot) > 0)
        setShowCraftHSlot(previousSlot, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

void IUIScene_CraftingMenu::ItemScrollLeft()
{
    if (m_horizontalSlotCount == 0)
        return;

    int previousSlot = m_horizontalSlot;
    int slot = (previousSlot != 0 ? previousSlot : m_horizontalSlotCount) - 1;
    m_horizontalSlot = slot;
    m_verticalColumn = 1;
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[2] = 1;
    m_verticalSlots[1] = 0;
    if (m_currentTab == 2 || m_currentTab == 3)
        m_verticalSlots[1] = m_customRecipeSlots[slot];
    UpdateVerticalSlots();
    UpdateHighlight();
    if (!isCustomRecipe() && GetRecipeCount(previousSlot) > 0)
        setShowCraftHSlot(previousSlot, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

void IUIScene_CraftingMenu::ItemScrollUp()
{
    bool singleColumn = m_compactInventory || isSpinnerGroup() || (!gRenderer->IsHiDef() && !gRenderer->IsWidescreen());
    int slotCount = getCurrentVSlotCount();
    if (slotCount < 2)
        return;

    if (singleColumn)
    {
        if (m_verticalSlots[1] == 0)
            m_verticalSlots[1] = getCurrentVSlotMax();
        else
            --m_verticalSlots[1];
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (slotCount > 2 && m_verticalColumn == 0)
    {
        int topSlot = m_verticalSlots[0];
        m_verticalSlots[2] = m_verticalSlots[1];
        m_verticalSlots[1] = topSlot;
        if (topSlot == 0)
            m_verticalSlots[0] = getCurrentVSlotMax();
        else
            m_verticalSlots[0] = topSlot - 1;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (slotCount > 2 || m_verticalColumn != 1)
    {
        --m_verticalColumn;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    UpdateVerticalSlots();
    UpdateHighlight();
}

void IUIScene_CraftingMenu::ItemScrollDown()
{
    bool singleColumn = m_compactInventory || isSpinnerGroup() || (!gRenderer->IsHiDef() && !gRenderer->IsWidescreen());
    int slotCount = getCurrentVSlotCount();
    if (slotCount < 2)
        return;

    if (singleColumn)
    {
        if (m_verticalSlots[1] == getCurrentVSlotMax())
            m_verticalSlots[1] = 0;
        else
            ++m_verticalSlots[1];
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (slotCount > 2 && m_verticalColumn == 2)
    {
        m_verticalSlots[0] = m_verticalSlots[1];
        m_verticalSlots[1] = m_verticalSlots[2];
        if (m_verticalSlots[2] == getCurrentVSlotMax())
            m_verticalSlots[2] = 0;
        else
            ++m_verticalSlots[2];
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (slotCount > 2 || m_verticalColumn != slotCount)
    {
        ++m_verticalColumn;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    UpdateVerticalSlots();
    UpdateHighlight();
}

bool IUIScene_CraftingMenu::handleKeyDown(int pad, int key, bool repeat)
{
    bool handled = false;
    if (m_tabsDirty)
        return false;
    if (repeat && key == 2)
        return false;

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[getPad()] != NULL)
    {
        Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
        if (tutorial != NULL)
        {
            tutorial->handleUIInput(key);
            if (gConsoleUIController.IsTutorialVisible(getPad()) && !tutorial->popupPending
                && !minecraft->mGameModes[getPad()]->getGameModeConstraints()->isInputAllowed(key))
                return false;
        }
    }

    switch (key)
    {
    case 0:
        CraftCurrentItem();
        handled = true;
        break;
    case 1:
    case 18:
        gConsoleUIController.ShowTooltip(pad, 2, false);
        gConsoleUIController.ShowTooltip(pad, 1, false);
        gConsoleUIController.ShowTooltip(pad, 0, false);
        gConsoleUIController.ShowTooltip(pad, 7, false);
        gConsoleUIController.CloseUIScenes(pad, false);
        handled = true;
        break;
    case 2:
        if (isCustomRecipe())
        {
            RemoveLastCustomRecipeIngredient();
            UpdateTooltips();
        }
        else if (!stackedCraftingAllowed())
        {
            int panelDisplay = m_panelDisplay + 1;
            if (panelDisplay > 2)
                panelDisplay = 0;
            m_panelDisplay = panelDisplay;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
            UpdateMultiPanel(false);
            UpdateTooltips();
        }
        else if (m_currentGroup > 0)
        {
            RemoveStackedCraftRecipe();
        }
        handled = true;
        break;
    case 3:
        if (!isCustomRecipe())
        {
            if (isDuplicateItem() && !m_duplicateItemSlots.empty())
            {
                std::pair<int, bool>& duplicate = m_duplicateItemSlots[m_verticalSlots[m_verticalColumn]];
                bool enabled = duplicate.second;
                not_null_ptr<ItemInstance> banner = m_player->mInventory->getItem(duplicate.first);
                if (!enabled || BannerBlockEntity::getPatternCount(banner) > 5)
                    return true;
            }
            AddStackedCraftRecipe(true);
        }
        else
        {
            bool enabled = false;
            not_null_ptr<ItemInstance> item =
                GetSpinnerSlot(m_horizontalSlot, m_customRecipeSlots[m_horizontalSlot], enabled);
            if (canAddCustomRecipeIngredient(item) && enabled)
            {
                AddCustomRecipeIngredient(item->copy());
                UpdateTooltips();
            }
        }
        handled = true;
        break;
    case 8:
        NavTabUp();
        handled = true;
        break;
    case 9:
        NavTabDown();
        handled = true;
        break;
    case 10:
        if (m_currentTab == 1 && m_baseRecipeItem->isEmpty())
            return true;
        showTabHighlight(m_currentGroup, false);
        if (m_currentGroup + 1 != m_groupCount)
        {
            ++m_currentGroup;
        }
        else
        {
            m_currentGroup = 0;
            if (m_currentTab == 1 && !m_baseRecipeItem->isEmpty())
                m_currentGroup = 1;
        }
        UpdateHTab();
        handled = true;
        break;
    case 11:
        if (m_currentTab == 1 && m_baseRecipeItem->isEmpty())
            return true;
        showTabHighlight(m_currentGroup, false);
        if (m_currentGroup == 0)
        {
            m_currentGroup = m_groupCount - 1;
        }
        else
        {
            --m_currentGroup;
            if (m_currentTab == 1 && isSpinnerGroup() && !m_baseRecipeItem->isEmpty())
                m_currentGroup = m_groupCount - 1;
        }
        UpdateHTab();
        handled = true;
        break;
    default:
        break;
    }

    switch (key)
    {
    case 4:
        ItemScrollUp();
        return true;
    case 5:
        ItemScrollDown();
        return true;
    case 6:
        ItemScrollRight();
        return true;
    case 7:
        ItemScrollLeft();
        return true;
    case 14:
        scrollDescriptionUp();
        break;
    case 15:
        scrollDescriptionDown();
        break;
    default:
        break;
    }
    return handled;
}

void IUIScene_CraftingMenu::Init(_CraftingPanelScreenInput* input)
{
    m_craftingMode = input->craftingMode;
    m_player = input->player;
    m_compactInventory = input->compactInventory;
    m_maintainedInventory = boost::shared_ptr<Inventory>(new Inventory(m_player.get()));
    m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);
    m_inventoryMenu = new InventoryMenu(m_maintainedInventory, false, m_player.get());
    m_inventoryStart = InventoryMenu::INV_SLOT_START;
    m_inventoryEnd = InventoryMenu::INV_SLOT_END;
}

void IUIScene_CraftingMenu::HandleInventoryUpdated()
{
    CheckRecipesAvailable();
    refreshBaseItemSlots(ItemInstance::EMPTY);
    UpdateSlotsAndHighlight();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::RemoveStackedCraftRecipe()
{
    if (!stackedCraftingAllowed())
        return;

    bool keepBaseItem = false;
    if (!m_stackedRecipes.empty())
    {
        Recipe::INGREDIENTS_REQUIRED* ingredients =
            Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[m_stackedRecipes.back().first];
        for (int i = 0; i < required.numIngredients; ++i)
        {
            unsigned int key = required.itemIds[i] | (required.auxValues[i] << 16);
            for (std::vector<std::pair<unsigned int, int> >::iterator used = m_usedIngredients.begin();
                 used != m_usedIngredients.end(); ++used)
            {
                if (used->first == key)
                {
                    used->second -= required.itemCounts[i];
                    break;
                }
            }
        }
        m_stackedRecipes.pop_back();
        updateAccumulatedItem();
        CheckRecipesAvailable();
        UpdateSlotsAndHighlight();
        refreshAllSlots();
        if (m_baseFromInventory)
            keepBaseItem = true;
    }

    if (m_stackedRecipes.empty() && !keepBaseItem)
    {
        m_selectedInventorySlot = -1;
        m_baseRecipeItem = ItemInstance::EMPTY;
        m_accumulatedItem = ItemInstance::EMPTY;
        showTabHighlight(m_currentGroup, false);
        m_currentGroup = 0;
        UpdateHTab();
    }
}

bool IUIScene_CraftingMenu::AddStackedCraftRecipe(bool advanceGroup)
{
    bool added = false;
    if (!stackedCraftingAllowed() || getCurrentVSlotCount() == 0)
        return false;

    Minecraft* minecraft = Minecraft::GetInstance();
    unsigned int verticalSlot = m_verticalSlots[m_verticalColumn];
    if (m_currentGroup == 0)
        m_baseFromInventory = false;

    if (isInventoryItem())
    {
        if (verticalSlot < m_baseItemSlots.size())
        {
            m_selectedInventorySlot = m_baseItemSlots[verticalSlot];
            m_baseRecipeItem = m_player->mInventory->getItem(m_selectedInventorySlot)->copy();
            m_baseRecipeItem->setCount(1);
            m_accumulatedItem = m_baseRecipeItem;
            updateAccumulatedItem();
            showTabHighlight(m_currentGroup, false);
            ++m_currentGroup;
            UpdateHTab();
            m_baseFromInventory = true;
        }
        return added;
    }

    if (!canAddMoreStacks())
        return added;

    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray(static_cast<Recipes::ERecipeCategory>(m_currentTab));
    int recipe = GetRecipeId(m_horizontalSlot, verticalSlot);
    int previousSlot = m_selectedInventorySlot;
    bool debugCrafting = false;
    bool recipeFound = false;
    if (isDuplicateItem())
    {
        m_selectedInventorySlot = m_duplicateItemSlots[verticalSlot].first;
        m_baseRecipeItem = m_player->mInventory->getItem(m_selectedInventorySlot)->copy();
        m_baseRecipeItem->setCount(1);
        m_accumulatedItem = m_baseRecipeItem;
        previousSlot = m_selectedInventorySlot;

        int baseItemId = Item::getId(m_baseRecipeItem->getItem());
        int baseAuxValue = m_baseRecipeItem->getAuxValue();
        int duplicateGroup = m_craftingMode != 0;
        for (int index = 0; index < GetRecipeCount(duplicateGroup); ++index)
        {
            int candidate = GetRecipeId(duplicateGroup, index);
            Recipe::INGREDIENTS_REQUIRED& required = ingredients[candidate];
            int baseItemCount = 0;
            for (int i = 0; i < required.numIngredients; ++i)
            {
                if (required.itemIds[i] == baseItemId && required.auxValues[i] == baseAuxValue)
                    baseItemCount += required.itemCounts[i];
            }
            if (baseItemCount > 1)
            {
                debugCrafting = GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1);
                recipe = candidate;
                recipeFound = true;
                break;
            }
        }
    }
    if (!recipeFound)
        debugCrafting = GameSettings::IsDebugSettingEnabled(static_cast<eDebugSetting>(14), -1);

    Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
    bool available = debugCrafting || required.available[getPad()];
    bool canCraft = available;
    if (isDuplicateItem() && !m_duplicateItemSlots.empty())
        canCraft = available || m_duplicateItemSlots[verticalSlot].second;

    if (!canCraft)
    {
        not_null_ptr<ItemInstance> result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        if (minecraft->mGameModes[getPad()] != NULL)
        {
            Tutorial* tutorial = minecraft->mGameModes[getPad()]->getTutorial();
            if (tutorial != NULL)
                tutorial->createItemSelected(result, required.available[getPad()]);
        }
        gConsoleUIController.PlayUISFX(SoundEvents::UI_CRAFT_FAIL);
        return added;
    }

    added = true;
    m_stackedRecipes.push_back(std::pair<int, int>(recipe, previousSlot));
    for (int i = 0; i < required.numIngredients; ++i)
    {
        unsigned int key = required.itemIds[i] | (required.auxValues[i] << 16);
        bool counted = false;
        for (std::vector<std::pair<unsigned int, int> >::iterator used = m_usedIngredients.begin();
             used != m_usedIngredients.end(); ++used)
        {
            if (used->first == key)
            {
                used->second += required.itemCounts[i];
                counted = true;
                break;
            }
        }
        if (!counted)
            m_usedIngredients.push_back(std::pair<unsigned int, int>(key, required.itemCounts[i]));
    }

    if (!isSpinnerGroup() || isInventoryItem())
    {
        if (!isDuplicateItem())
        {
            updateAccumulatedItem();
            CheckRecipesAvailable();
            UpdateSlotsAndHighlight();
            return added;
        }
    }
    else if (!isDuplicateItem())
    {
        not_null_ptr<ItemInstance> result = required.recipe->assemble(boost::shared_ptr<CraftingContainer>());
        m_baseRecipeItem = result;
        m_accumulatedItem = result;
        updateAccumulatedItem();
        showTabHighlight(m_currentGroup, false);
        ++m_currentGroup;
        UpdateHTab();
        return added;
    }

    if (advanceGroup)
    {
        updateAccumulatedItem();
        showTabHighlight(m_currentGroup, false);
        ++m_currentGroup;
        UpdateHTab();
    }
    return added;
}
