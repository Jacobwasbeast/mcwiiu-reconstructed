#include "game/ui/IUIScene_CraftingMenu.hpp"

#include <algorithm>
#include <cstring>

#include "game/client/audio/SoundEvents.hpp"
#include "game/client/render/Renderer.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GameModeConstraints.hpp"
#include "game/core/GameSettings.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/CraftingContainer.hpp"
#include "game/item/Inventory.hpp"
#include "game/item/Item.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/item/Recipe.hpp"
#include "game/item/Recipes.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/InventoryMenu.hpp"
#include "game/world/block/BannerBlockEntity.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/Player.hpp"

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
    m_inventoryItems = NonNullList<not_null_ptr<ItemInstance> >::withSize(9, ItemInstance::EMPTY);
    m_tooltipState = 0;
    m_selectedCustomIngredient = -1;
    m_tooltipPlayer = 0;
    m_customRecipeValid = false;
    m_ingredientCount = 0;
    m_baseRecipeItem = ItemInstance::EMPTY;
    m_accumulatedItem = ItemInstance::EMPTY;
    m_craftingContainer = boost::shared_ptr<CraftingContainer>(new CraftingContainer(0, 3, 3));
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
    return getGroupAtIndex(m_currentGroup);
}

int IUIScene_CraftingMenu::GetRecipeId(int group, int recipe)
{
    if (m_currentTab == 2)
        return m_recipeTable[m_currentGroup][2];
    if (m_currentTab != 3)
        return m_recipeTable[group][recipe + 2];
    if (m_currentGroup != 0 && !m_recipeIngredients.empty())
    {
        not_null_ptr<ItemInstance> item = m_recipeIngredients[m_customRecipeSlots[m_horizontalSlot]];
        item->setCount(1);
        return m_recipeTable[m_currentGroup][item->getItem() == 0 ? 2 : m_customRecipeSlots[m_horizontalSlot] + 2];
    }
    return m_recipeTable[group][recipe + 2];
}

bool IUIScene_CraftingMenu::isSpinnerGroup()
{
    return (m_currentGroup == 0 && m_currentTab == 1) || m_currentTab == 2 || m_currentTab == 3;
}

bool IUIScene_CraftingMenu::isInventoryItem()
{
    return isSpinnerGroup() && m_currentTab == 1 &&
           m_horizontalSlot == (m_craftingMode == 1 ? 1 : 0);
}

bool IUIScene_CraftingMenu::isDuplicateItem()
{
    return isSpinnerGroup() && m_currentTab == 1 &&
           m_horizontalSlot == (m_craftingMode == 1 ? 2 : 1);
}

bool IUIScene_CraftingMenu::isCustomRecipe()
{
    return m_currentTab == 2 || m_currentTab == 3;
}

int IUIScene_CraftingMenu::GetRecipeCount(int group)
{
    return m_recipeTable[isCustomRecipe() ? m_currentGroup : group][0];
}

int IUIScene_CraftingMenu::getCurrentVSlotCount()
{
    if (isInventoryItem())
        return (int)m_inventoryRecipes.size();
    if (isDuplicateItem())
        return (int)m_duplicateRecipes.size();
    if (isCustomRecipe())
    {
        if (m_currentTab == 2)
        {
            if (m_currentGroup == 0)
                return (int)m_customRecipes[m_horizontalSlot].size();
            if (m_currentGroup == 1)
                return m_horizontalSlot == 0 ? (int)m_recipeIngredients.size() :
                                               (int)m_customRecipes[0].size();
            return m_horizontalSlot == 0 ? 1 : (int)m_recipeIngredients.size();
        }
        if (m_currentTab == 3)
            return m_horizontalSlot == 0 ? (int)m_recipeIngredients.size() :
                                           (int)m_customRecipeIngredients.size();
    }
    return GetRecipeCount(m_horizontalSlot);
}

bool IUIScene_CraftingMenu::isItemSelected(int itemId)
{
    if (!m_player || !m_player->mInventory)
        return false;
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    int count = getCurrentVSlotCount();
    if (count == 0)
        return false;
    int recipe = GetRecipeId(m_horizontalSlot,
                             count > 1 ? m_verticalSlots[m_verticalColumn] : 0);
    return Item::VALIDgetId(ingredients[recipe].recipe->getResultItem()->getItem()) == itemId;
}

not_null_ptr<ItemInstance> IUIScene_CraftingMenu::GetBaseItemSlot(int column, int slot, bool& enabled)
{
    if (m_currentTab == 2 && (unsigned int)slot < m_customRecipes[column].size())
    {
        enabled = m_customRecipes[column][slot].second;
        return m_customRecipes[column][slot].first;
    }
    if (m_currentTab == 3 && (unsigned int)slot < m_customRecipeIngredients.size())
    {
        enabled = m_customRecipeIngredients[slot].second;
        return m_customRecipeIngredients[slot].first;
    }
    enabled = false;
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> IUIScene_CraftingMenu::GetSpinnerSlot(int spinner, int slot, bool& enabled)
{
    enabled = false;
    if (m_currentTab == 2)
    {
        if (m_currentGroup == 0)
        {
            if (spinner >= 0 && spinner < 3 &&
                slot >= 0 && slot < (int)m_customRecipes[spinner].size())
            {
                enabled = m_customRecipes[spinner][slot].second;
                return m_customRecipes[spinner][slot].first;
            }
        }
        else if (m_currentGroup == 1)
        {
            if (spinner == 0)
            {
                if (slot >= 0 && slot < (int)m_recipeIngredients.size())
                {
                    enabled = true;
                    return m_recipeIngredients[slot];
                }
            }
            else if (slot >= 0 && slot < (int)m_customRecipes[0].size())
            {
                enabled = m_customRecipes[0][slot].second;
                return m_customRecipes[0][slot].first;
            }
        }
        else if (slot >= 0 && slot < (int)m_customRecipes[0].size())
        {
            enabled = m_customRecipes[0][slot].second;
            return m_customRecipes[0][slot].first;
        }
    }
    else
    {
        if (spinner == 0)
        {
            if (slot >= 0 && slot < (int)m_recipeIngredients.size())
            {
                enabled = true;
                return m_recipeIngredients[slot];
            }
        }
        else if (slot >= 0 && slot < (int)m_customRecipeIngredients.size())
        {
            enabled = m_customRecipeIngredients[slot].second;
            return m_customRecipeIngredients[slot].first;
        }
    }
    return ItemInstance::EMPTY;
}

int IUIScene_CraftingMenu::GetCustomIngredientCount(not_null_ptr<ItemInstance> item)
{
    int count = 0;
    if (!item)
        return 0;
    for (unsigned int i = 0; i < m_maintainedInventory->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> slot = m_maintainedInventory->getItem(i);
        if (slot->isEmpty())
            continue;
        if (slot->getItem() != item->getItem())
            continue;
        if (slot->getAuxValue() != item->getAuxValue())
            continue;
        if (slot->getTag() != 0 && item->getTag() != 0 && !ItemInstance::tagMatches(slot, item))
            continue;
        count += slot->getCount();
    }
    return count;
}

bool IUIScene_CraftingMenu::isValidCustomRecipeIngredient(int index, not_null_ptr<ItemInstance> item)
{
    if (!item || item->isEmpty() || index < 0)
        return false;
    if (index >= (int)m_recipeIngredients.size())
        return true;
    not_null_ptr<ItemInstance> required = m_recipeIngredients[index];
    return ItemInstance::isSame(required, item) &&
           GetCustomIngredientCount(item) < required->getCount();
}

int IUIScene_CraftingMenu::GetCustomIngredientIndex(not_null_ptr<ItemInstance> item)
{
    if (!item)
        return -1;
    for (unsigned int i = 0; i < m_maintainedInventory->getContainerSize(); ++i)
    {
        not_null_ptr<ItemInstance> slot = m_maintainedInventory->getItem(i);
        if (slot->isEmpty())
            continue;
        if (slot->getItem() != item->getItem())
            continue;
        if (slot->getAuxValue() != item->getAuxValue())
            continue;
        if (ItemInstance::tagMatches(slot, item))
            return (int)i;
    }
    return -1;
}

void IUIScene_CraftingMenu::UpdateMaintainedInventory()
{
    m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    for (std::vector<std::pair<int, int> >::const_iterator it = m_baseRecipeIngredients.begin();
         it != m_baseRecipeIngredients.end(); ++it)
        GameMode::RemoveRecipeItemsFromInventory(ingredients[it->first], m_maintainedInventory,
                                                 ItemInstance::EMPTY, false);
    if (!m_stackedRecipeCounts.empty())
    {
        int count = m_craftingMode == 0 ? 4 : 9;
        for (int slot = 0; slot < count; ++slot)
        {
            not_null_ptr<ItemInstance> item = m_maintainedInventory->getItem(slot);
            if (item->isEmpty())
                continue;
            int index = GetCustomIngredientIndex(item);
            if (index < 0)
            {
                setIngredientSlotRedBox(slot, true);
                m_customRecipeValid = false;
                continue;
            }
            GameMode::RemoveCustomRecipeItemsFromInventory(index, m_maintainedInventory);
        }
    }
}

void IUIScene_CraftingMenu::DisplayCustomRecipeIngredients(int selected)
{
    hideAllIngredientsSlots();
    for (int i = 0; i < (int)m_customRecipeIngredients.size() && i < 9; ++i)
    {
        not_null_ptr<ItemInstance> item = m_customRecipeIngredients[i].first;
        setIngredientSlotItem(i, item->getCount(), item);
        setIngredientSlotRedBox(i, !m_customRecipeIngredients[i].second);
        if (i == selected)
            setIngredientSlotHighlighted(i);
    }
    UpdateMaintainedInventory();
}

void IUIScene_CraftingMenu::UpdateDescriptionText(bool top)
{
    bool enabled = false;
    not_null_ptr<ItemInstance> item = GetSpinnerSlot(m_horizontalSlot,
                                                     m_verticalSlots[m_verticalColumn], enabled);
    const wchar_t* text = L"";
    if (item && !item->isEmpty())
        text = L10N::GetString(item->getDescriptionId(item->getAuxValue()));
    if (top)
        setDescriptionTextTop(text);
    else
        setDescriptionText(text);
}

void IUIScene_CraftingMenu::CheckRecipesAvailable()
{
    m_availableRecipes.clear();
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    int count = GetRecipeCount(m_horizontalSlot);
    for (int i = 0; i < count; ++i)
    {
        int recipe = GetRecipeId(m_horizontalSlot, i);
        if (ingredients != 0 && ingredients[recipe].recipe != 0 &&
            ingredients[recipe].available[m_craftingMode])
            m_availableRecipes.push_back(recipe);
    }
    UpdateMaintainedInventory();
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
    if (isSpinnerGroup() &&
        ((!isInventoryItem() && !isDuplicateItem()) || (!isInventoryItem() && isDuplicateItem())))
        return true;
    if (m_accumulatedItem->isEmpty())
        return false;
    return BannerBlockEntity::getPatternCount(m_accumulatedItem) < 6;
}

void IUIScene_CraftingMenu::UpdateHorizontalSlots()
{
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    if (!isSpinnerGroup())
    {
        for (int slot = 0; slot < m_horizontalSlotCount; ++slot)
        {
            int count = GetRecipeCount(slot);
            if (count == 0)
                return;
            int recipe = GetRecipeId(slot, 0);
            not_null_ptr<ItemInstance> item(new ItemInstance(*ingredients[recipe].recipe->getResultItem()));
            unsigned int displayValue;
            if (GameSettings::IsDebugSettingEnabled((eDebugSetting)0xe, -1))
            {
                displayValue = 0x1f;
            }
            else if (ingredients[recipe].available[m_craftingMode] &&
                     (m_currentTab != 1 || canAddMoreStacks()))
            {
                displayValue = 0x1f;
            }
            else
            {
                displayValue = 0x10;
            }
            setCraftHSlotItem(slot, recipe, item, displayValue);
        }
        return;
    }
    if (m_currentTab != 1 && m_currentTab != 2 && m_currentTab != 3)
        return;
    int numSlots = getNumSpinnerSlots();
    for (int spinner = 0; spinner < numSlots; ++spinner)
    {
        bool enabled = false;
        not_null_ptr<ItemInstance> item = GetSpinnerSlot(spinner, 0, enabled);
        unsigned int displayValue = 0x1f;
        if (!enabled)
        {
            displayValue = 0x10;
        }
        else if (m_currentGroup == 0 && spinner == 1)
        {
            if (!isValidCustomRecipeIngredient(8, item))
                displayValue = 0x10;
        }
        setSpecialSlotItem(getPad(), spinner, item, displayValue);
    }
}

void IUIScene_CraftingMenu::UpdateVerticalSlots()
{
    hideAllVSlots();
    int count = getCurrentVSlotCount();
    int first = m_verticalSlots[m_verticalColumn];
    for (int slot = 0; slot < count; ++slot)
    {
        bool enabled = false;
        not_null_ptr<ItemInstance> item;
        if (isSpinnerGroup())
            item = GetSpinnerSlot(m_horizontalSlot, slot, enabled);
        else
        {
            int recipe = GetRecipeId(m_horizontalSlot, slot);
            Recipe::INGREDIENTS_REQUIRED* ingredients =
                Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
            if (ingredients != 0 && ingredients[recipe].recipe != 0 &&
                ingredients[recipe].recipe->getResultItem() != 0)
            {
                item = not_null_ptr<ItemInstance>(
                    new ItemInstance(*ingredients[recipe].recipe->getResultItem()));
                enabled = ingredients[recipe].available[m_craftingMode];
            }
        }
        if (item)
            setCraftVSlotItem(slot, GetRecipeId(m_horizontalSlot, slot), item, enabled ? 1 : 0);
    }
    updateVSlotPositions(first, count);
}

bool IUIScene_CraftingMenu::stackedCraftingAllowed()
{
    return m_currentTab == 1;
}

void IUIScene_CraftingMenu::DisplayIngredients()
{
    if (isCustomRecipe())
    {
        DisplayCustomRecipeIngredients(m_selectedCustomIngredient);
        return;
    }
    hideAllIngredientsSlots();
    int recipe = GetRecipeId(m_horizontalSlot, m_verticalSlots[m_verticalColumn]);
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    if (ingredients == 0 || ingredients[recipe].recipe == 0)
        return;
    Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
    for (int i = 0; i < required.numIngredients && i < 9; ++i)
    {
        Item* source = Item::byId(required.itemIds[i]);
        not_null_ptr<ItemInstance> item(new ItemInstance(source, required.itemCounts[i],
                                                         required.auxValues[i]));
        setIngredientSlotItem(i, required.itemCounts[i], item);
        setIngredientSlotRedBox(i, !required.available[m_craftingMode]);
        setIngredientDescriptionItem(i, required.itemCounts[i], item);
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
    unsigned int maximum = m_craftingMode == 0 ? 4 : 9;
    return (unsigned int)((int)m_stackedRecipeCounts.size() + m_ingredientCount) < maximum;
}

void IUIScene_CraftingMenu::UpdateTooltips()
{
    int primary = isCustomRecipe() ? 0x9219698b : 0xded2ef45;
    int secondary = canAddMoreStacks() ? 0x91d18454 : -1;
    gConsoleUIController.SetTooltips((unsigned int)getPad(), primary, secondary, -1, -1,
                                     -2, 0xf73b4f2d, -2, 0x06974812, -1, -1, -1,
                                     -1, -1, -1, -1, -1, false, false);
}

void IUIScene_CraftingMenu::UpdateHighlight()
{
    showTabHighlight(m_currentTab, true);
    if (isInventoryItem())
        setInventorySlotHighlighted(m_verticalSlots[m_verticalColumn]);
    else if (isCustomRecipe())
        setIngredientSlotHighlighted(m_selectedCustomIngredient);
    SetCraftingMenu(m_horizontalSlot, m_verticalSlots[m_verticalColumn], isSpinnerGroup());
    DisplayIngredients();
    UpdateDescriptionText(false);
    updateHighlightAndScrollPositions();
}

void IUIScene_CraftingMenu::UpdateSlotsAndHighlight()
{
    UpdateHorizontalSlots();
    UpdateVerticalSlots();
    UpdateHighlight();
}

void IUIScene_CraftingMenu::RemoveLastCustomRecipeIngredient()
{
    if (m_customRecipeIngredients.empty())
        return;
    m_customRecipeIngredients.pop_back();
    m_selectedCustomIngredient = (int)m_customRecipeIngredients.size() - 1;
    UpdateMaintainedInventory();
    CheckRecipesAvailable();
    DisplayCustomRecipeIngredients(m_selectedCustomIngredient);
    UpdateDescriptionText(true);
    UpdateSlotsAndHighlight();
}

void IUIScene_CraftingMenu::updateAccumulatedItem()
{
    if (m_baseRecipeIngredients.empty())
    {
        m_accumulatedItem = ItemInstance::EMPTY;
        return;
    }
    int recipe = m_baseRecipeIngredients.back().first;
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    if (ingredients == 0 || ingredients[recipe].recipe == 0 ||
        ingredients[recipe].recipe->getResultItem() == 0)
    {
        m_accumulatedItem = ItemInstance::EMPTY;
        return;
    }
    m_accumulatedItem =
        not_null_ptr<ItemInstance>(new ItemInstance(*ingredients[recipe].recipe->getResultItem()));
}

int IUIScene_CraftingMenu::getCurrentVSlotMax()
{
    int count = getCurrentVSlotCount();
    return count == 0 ? 0 : count - 1;
}

const wchar_t* IUIScene_CraftingMenu::GetGroupNameText(int group)
{
    if (m_currentTab == 1)
    {
        switch ((unsigned int)group)
        {
        case 1:
            return L10N::GetString((int)0x133e5cd4u);
        case 2:
            return L10N::GetString((int)0xb35ee68bu);
        case 3:
            return L10N::GetString((int)0xdd9417bau);
        case 4:
            return L10N::GetString((int)0xf8e4117fu);
        case 5:
            return L10N::GetString((int)0x8078384bu);
        default:
            return L"";
        }
    }
    if (m_currentTab == 2 || m_currentTab == 3)
        return L"";
    switch ((unsigned int)group)
    {
    case 0:
        return L10N::GetString((int)0xe6a4f105u);
    case 1:
        return L10N::GetString((int)0xa00b4cf0u);
    case 2:
        return L10N::GetString((int)0xef31f329u);
    case 3:
        return L10N::GetString((int)0xf187db6du);
    case 4:
        return L10N::GetString((int)0x89f72b83u);
    case 5:
        return L10N::GetString((int)0x415962d0u);
    default:
        return L10N::GetString((int)0xac698282u);
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
    if (m_stackedRecipeCounts.empty())
        return;
    m_stackedRecipeCounts.clear();
    for (int slot = 0; slot < 9; ++slot)
        m_inventoryItems[slot] = ItemInstance::EMPTY;
    if (isCustomRecipe())
    {
        DisplayCustomRecipeIngredients(GetRecipeId(-1, -1));
        return;
    }
    setIngredientSlotHighlighted(-1);
}

void IUIScene_CraftingMenu::refreshBaseItemSlots(not_null_ptr<ItemInstance> item)
{
    m_inventoryRecipes.clear();
    m_duplicateRecipes.clear();
    m_baseRecipeIngredients.clear();
    if (!m_maintainedInventory)
        return;
    for (unsigned int slot = 0; slot < m_maintainedInventory->getContainerSize(); ++slot)
    {
        not_null_ptr<ItemInstance> inventoryItem = m_maintainedInventory->getItem(slot);
        if (!inventoryItem || inventoryItem->isEmpty())
            continue;
        if (!item || item->isEmpty() || ItemInstance::isSame(item, inventoryItem))
            m_inventoryRecipes.push_back((int)slot);
        else
            m_duplicateRecipes.push_back(std::make_pair((int)slot, true));
    }
}

void IUIScene_CraftingMenu::UpdateTabs(bool force)
{
    m_tabsDirty = force;
    m_horizontalSlotCount = 0;
    for (int tab = 0; tab < 7; ++tab)
    {
        int group = m_groupMap == 0 ? tab : getGroupAtIndex(tab);
        bool enabled = group >= 0 && GetRecipeCount(group) > 0;
        setHTabEnabled(tab, enabled);
        showTabHighlight(tab, tab == m_currentGroup);
        if (enabled)
            ++m_horizontalSlotCount;
    }
    UpdateMultiPanel(m_horizontalSlotCount > 1);
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
    if (!isCustomRecipe() && GetRecipeCount(m_horizontalSlot) > 0)
        setShowCraftHSlot(m_horizontalSlot, true);
    m_verticalColumn = 1;
    m_horizontalSlot = 0;
    UpdateTabs(false);
    refreshAllSlots();
}

void IUIScene_CraftingMenu::RemoveStackedCraftRecipe()
{
    if (!stackedCraftingAllowed())
        return;
    bool stillHasGunpowder = false;
    if (!m_baseRecipeIngredients.empty())
    {
        int recipe = m_baseRecipeIngredients.back().first;
        Recipe::INGREDIENTS_REQUIRED* ingredients =
            Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
        Recipe::INGREDIENTS_REQUIRED& required = ingredients[recipe];
        for (int i = 0; i < required.numIngredients; ++i)
        {
            int key = required.itemIds[i] | (required.auxValues[i] << 16);
            for (std::vector<std::pair<int, int> >::iterator it = m_customIngredientCounts.begin();
                 it != m_customIngredientCounts.end(); ++it)
            {
                if (it->first == key)
                {
                    it->second -= required.itemCounts[i];
                    break;
                }
            }
        }
        m_baseRecipeIngredients.pop_back();
        updateAccumulatedItem();
        CheckRecipesAvailable();
        UpdateSlotsAndHighlight();
        refreshAllSlots();
        stillHasGunpowder = m_hasGunpowder;
    }
    if (m_baseRecipeIngredients.empty() && !stillHasGunpowder)
    {
        m_selectedCustomIngredient = -1;
        m_baseRecipeItem = ItemInstance::EMPTY;
        m_accumulatedItem = ItemInstance::EMPTY;
        showTabHighlight(m_currentGroup, false);
        m_currentGroup = 0;
        UpdateHTab();
    }
}

bool IUIScene_CraftingMenu::isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance> item)
{
    unsigned int slots = m_craftingMode == 0 ? 4 : 9;
    for (unsigned int i = 0; i < slots; ++i)
    {
        not_null_ptr<ItemInstance> slot = m_craftingContainer->getItem(i);
        if (ItemInstance::matches(slot, item, true))
            return true;
    }
    return false;
}

void IUIScene_CraftingMenu::AddCustomRecipeIngredient(not_null_ptr<ItemInstance> item)
{
    if (!canAddCustomRecipeIngredient(item))
        return;
    int index = GetCustomIngredientIndex(item);
    m_customRecipeIngredients.push_back(std::make_pair(item, true));
    m_selectedCustomIngredient = (int)m_customRecipeIngredients.size() - 1;
    if (index >= 0)
        ++m_customRecipeSlots[index % 3];
    UpdateMaintainedInventory();
    CheckRecipesAvailable();
    DisplayCustomRecipeIngredients(m_selectedCustomIngredient);
    UpdateDescriptionText(true);
}

bool IUIScene_CraftingMenu::AddStackedCraftRecipe(bool all)
{
    if (!stackedCraftingAllowed() || !canAddMoreStacks())
        return false;
    int recipe = GetRecipeId(m_horizontalSlot, m_verticalSlots[m_verticalColumn]);
    int amount = all ? GetRecipeCount(m_horizontalSlot) : 1;
    for (int i = 0; i < amount; ++i)
        m_stackedRecipeCounts.push_back(recipe);
    ++m_currentGroup;
    updateAccumulatedItem();
    CheckRecipesAvailable();
    UpdateSlotsAndHighlight();
    return true;
}

void IUIScene_CraftingMenu::CraftCurrentItem()
{
    if (!m_player)
        return;
    GameMode* gameMode = m_player->GetGameMode();
    if (gameMode == 0)
        return;
    if (isCustomRecipe())
        gameMode->handleCustomCraftItem(m_stackedRecipeCounts, m_player);
    else if (stackedCraftingAllowed() && !m_stackedRecipeCounts.empty())
        gameMode->handleStackedCraftItem(m_baseRecipeIngredients, m_currentTab, m_player);
    else
        gameMode->handleCraftItem(m_currentTab,
                                  GetRecipeId(m_horizontalSlot,
                                              m_verticalSlots[m_verticalColumn]),
                                  m_player);
    refreshBaseItemSlots(ItemInstance::EMPTY);
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
    m_stackedRecipeCounts.clear();
    m_customIngredientCounts.clear();
    if (!isCustomRecipe() && GetRecipeCount(m_horizontalSlot) > 0)
        setShowCraftHSlot(m_horizontalSlot, true);
    m_verticalColumn = 1;
    m_horizontalSlot = 0;
    UpdateTabs(false);
    refreshAllSlots();
    if (m_currentTab == 2)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[getPad()]->getTutorial();
        if (tutorial != 0)
            tutorial->changeTutorialState((eTutorial_State)36, NULL);
    }
}

void IUIScene_CraftingMenu::NavTabUp()
{
    if (!isCustomRecipe() && GetRecipeCount(m_horizontalSlot) > 0)
        setShowCraftHSlot(m_horizontalSlot, true);
    m_currentTab = m_currentTab == 0 ? 3 : m_currentTab - 1;
    UpdateVTab();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::NavTabDown()
{
    if (!isCustomRecipe() && GetRecipeCount(m_horizontalSlot) > 0)
        setShowCraftHSlot(m_horizontalSlot, true);
    m_currentTab = m_currentTab < 3 ? m_currentTab + 1 : 0;
    UpdateVTab();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::ItemScrollRight()
{
    int previous = m_horizontalSlot;
    int slot = previous + 1;
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
    if (!isCustomRecipe() && GetRecipeCount(previous) > 0)
        setShowCraftHSlot(previous, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

void IUIScene_CraftingMenu::ItemScrollLeft()
{
    if (m_horizontalSlotCount == 0)
        return;
    int previous = m_horizontalSlot;
    int slot = (previous == 0 ? m_horizontalSlotCount : previous) - 1;
    m_horizontalSlot = slot;
    m_verticalColumn = 1;
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[2] = 1;
    m_verticalSlots[1] = 0;
    if (m_currentTab == 2 || m_currentTab == 3)
        m_verticalSlots[1] = m_customRecipeSlots[slot];
    UpdateVerticalSlots();
    UpdateHighlight();
    if (!isCustomRecipe() && GetRecipeCount(previous) > 0)
        setShowCraftHSlot(previous, true);
    gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
}

void IUIScene_CraftingMenu::ItemScrollUp()
{
    bool singleColumn = m_compactInventory || isSpinnerGroup() ||
                        (!gRenderer->IsHiDef() && !gRenderer->IsWidescreen());
    int count = getCurrentVSlotCount();
    if (count < 2)
        return;
    if (singleColumn)
    {
        if (m_verticalSlots[1] == 0)
            m_verticalSlots[1] = getCurrentVSlotMax();
        else
            --m_verticalSlots[1];
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (count > 2 && m_verticalColumn == 0)
    {
        int top = m_verticalSlots[0];
        m_verticalSlots[2] = m_verticalSlots[1];
        m_verticalSlots[1] = top;
        if (top == 0)
            m_verticalSlots[0] = getCurrentVSlotMax();
        else
            m_verticalSlots[0] = top - 1;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (count > 2 || m_verticalColumn != 1)
    {
        --m_verticalColumn;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    UpdateVerticalSlots();
    UpdateHighlight();
}

void IUIScene_CraftingMenu::ItemScrollDown()
{
    bool singleColumn = m_compactInventory || isSpinnerGroup() ||
                        (!gRenderer->IsHiDef() && !gRenderer->IsWidescreen());
    int count = getCurrentVSlotCount();
    if (count < 2)
        return;
    if (singleColumn)
    {
        if (m_verticalSlots[1] == getCurrentVSlotMax())
            m_verticalSlots[1] = 0;
        else
            ++m_verticalSlots[1];
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (count > 2 && m_verticalColumn == 2)
    {
        m_verticalSlots[0] = m_verticalSlots[1];
        m_verticalSlots[1] = m_verticalSlots[2];
        if (m_verticalSlots[2] == getCurrentVSlotMax())
            m_verticalSlots[2] = 0;
        else
            ++m_verticalSlots[2];
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    else if (count > 2 || m_verticalColumn != count)
    {
        ++m_verticalColumn;
        gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
    }
    UpdateVerticalSlots();
    UpdateHighlight();
}

bool IUIScene_CraftingMenu::handleKeyDown(int key, int action, bool repeat)
{
    if (m_tabsDirty || (repeat && action == 2))
        return false;
    if (action == 0)
        CraftCurrentItem();
    else if (action == 1 || action == 18)
    {
        gConsoleUIController.ShowTooltip((unsigned int)key, 2, false);
        gConsoleUIController.ShowTooltip((unsigned int)key, 1, false);
        gConsoleUIController.ShowTooltip((unsigned int)key, 0, false);
        gConsoleUIController.ShowTooltip((unsigned int)key, 7, false);
        gConsoleUIController.CloseUIScenes(key, false);
    }
    else if (action == 2)
    {
        if (isCustomRecipe())
            RemoveLastCustomRecipeIngredient();
        else if (stackedCraftingAllowed())
        {
            if (m_currentGroup > 0)
                RemoveStackedCraftRecipe();
        }
        else
        {
            m_tooltipState = (m_tooltipState + 1) % 3;
            gConsoleUIController.PlayUISFX(SoundEvents::UI_FOCUS);
            UpdateMultiPanel(false);
        }
        UpdateTooltips();
    }
    else if (action == 3)
    {
        if (isCustomRecipe())
        {
            bool enabled = false;
            not_null_ptr<ItemInstance> item =
                GetSpinnerSlot(m_horizontalSlot, m_customRecipeSlots[m_horizontalSlot], enabled);
            if (enabled && canAddCustomRecipeIngredient(item))
                AddCustomRecipeIngredient(item->copy());
        }
        else
            AddStackedCraftRecipe(true);
    }
    else if (action == 4)
        ItemScrollUp();
    else if (action == 5)
        ItemScrollDown();
    else if (action == 6)
        ItemScrollRight();
    else if (action == 7)
        ItemScrollLeft();
    else if (action == 8)
        NavTabUp();
    else if (action == 9)
        NavTabDown();
    else if (action == 10)
    {
        if (m_craftingMode == 1 && m_baseRecipeItem->isEmpty())
            return true;
        showTabHighlight(m_currentGroup, false);
        if (m_currentGroup + 1 != m_groupPage)
            ++m_currentGroup;
        else
        {
            m_currentGroup = 0;
            if (m_craftingMode == 1 && !m_baseRecipeItem->isEmpty())
                m_currentGroup = 1;
        }
        UpdateHTab();
    }
    else if (action == 11)
    {
        if (m_craftingMode == 1 && m_baseRecipeItem->isEmpty())
            return true;
        showTabHighlight(m_currentGroup, false);
        if (m_currentGroup == 0)
            m_currentGroup = m_groupPage - 1;
        else
        {
            --m_currentGroup;
            if (m_craftingMode == 1 && isSpinnerGroup() && !m_baseRecipeItem->isEmpty())
                m_currentGroup = m_groupPage - 1;
        }
        UpdateHTab();
    }
    else if (action == 14)
        scrollDescriptionUp();
    else if (action == 15)
        scrollDescriptionDown();
    else
        return false;
    return true;
}

void IUIScene_CraftingMenu::Init(_CraftingPanelScreenInput* input)
{
    m_player = input->player;
    m_craftingMode = input->craftingMode;
    m_compactInventory = input->compactInventory;
    m_maintainedInventory = boost::shared_ptr<Inventory>(new Inventory(m_player.get()));
    m_inventoryMenu = new InventoryMenu(m_maintainedInventory, false, m_player.get());
    m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);
    m_inventoryStart = 9;
    m_inventoryEnd = 36;
}

void IUIScene_CraftingMenu::HandleInventoryUpdated()
{
    CheckRecipesAvailable();
    refreshBaseItemSlots(ItemInstance::EMPTY);
    UpdateSlotsAndHighlight();
    UpdateTooltips();
}
