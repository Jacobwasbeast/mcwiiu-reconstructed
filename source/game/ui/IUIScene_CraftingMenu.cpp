#include "game/ui/IUIScene_CraftingMenu.hpp"

#include <algorithm>

#include "game/core/ConsoleUIController.hpp"
#include "game/core/GameMode.hpp"
#include "game/core/GameModeConstraints.hpp"
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
    : m_horizontalSlot(0),
      m_verticalColumn(1),
      m_recipePage(0),
      m_currentGroup(0),
      m_groupMap(0),
      m_selectedCustomIngredient(-1),
      m_inventoryItems(NonNullList<not_null_ptr<ItemInstance> >::withSize(9, ItemInstance::EMPTY)),
      m_customRecipeValid(false),
      m_tooltipPlayer(0),
      m_tooltipState(0),
      m_descriptionState(0),
      m_currentTab(0)
{
    std::fill(&m_recipeTable[0][0], &m_recipeTable[0][0] + 600, 0);
    m_verticalSlots[0] = 0;
    m_verticalSlots[1] = 1;
    m_verticalSlots[2] = 2;
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
    return m_currentGroup == 0 && (m_currentTab == 1 || m_currentTab == 2 || m_currentTab == 3);
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
    return GetRecipeCount(m_horizontalSlot);
}

bool IUIScene_CraftingMenu::isItemSelected(int itemId)
{
    if (!m_player || !m_player->mInventory || getCurrentVSlotCount() == 0)
        return false;
    Recipe::INGREDIENTS_REQUIRED* ingredients =
        Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
    int recipe = GetRecipeId(m_horizontalSlot, getCurrentVSlotCount() > 1 ?
                             m_verticalSlots[m_verticalColumn] : 0);
    if (ingredients == 0 || ingredients[recipe].recipe == 0)
        return false;
    ItemInstance* result = ingredients[recipe].recipe->getResultItem();
    return result != 0 && Item::VALIDgetId(result->getItem()) == itemId;
}

not_null_ptr<ItemInstance> IUIScene_CraftingMenu::GetBaseItemSlot(int column, int slot, bool& enabled)
{
    enabled = false;
    if (m_currentTab == 2 && column >= 0 && column < 3 &&
        slot >= 0 && slot < (int)m_customRecipes[column].size())
    {
        enabled = m_customRecipes[column][slot].second;
        return m_customRecipes[column][slot].first;
    }
    if (m_currentTab == 3 && slot >= 0 && slot < (int)m_customRecipeIngredients.size())
    {
        enabled = m_customRecipeIngredients[slot].second;
        return m_customRecipeIngredients[slot].first;
    }
    return ItemInstance::EMPTY;
}

not_null_ptr<ItemInstance> IUIScene_CraftingMenu::GetSpinnerSlot(int spinner, int slot, bool& enabled)
{
    enabled = false;
    if (m_currentTab == 2)
    {
        if (m_currentGroup == 0 && spinner >= 0 && spinner < 3 &&
            slot >= 0 && slot < (int)m_customRecipes[spinner].size())
        {
            enabled = m_customRecipes[spinner][slot].second;
            return m_customRecipes[spinner][slot].first;
        }
        if (m_currentGroup == 1 && spinner == 0 &&
            slot >= 0 && slot < (int)m_recipeIngredients.size())
        {
            enabled = true;
            return m_recipeIngredients[slot];
        }
    }
    if (m_currentTab == 3)
    {
        if (spinner == 0 && slot >= 0 && slot < (int)m_recipeIngredients.size())
        {
            enabled = true;
            return m_recipeIngredients[slot];
        }
        if (slot >= 0 && slot < (int)m_customRecipeIngredients.size())
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
    for (std::vector<std::pair<not_null_ptr<ItemInstance>, bool> >::const_iterator it =
             m_customRecipeIngredients.begin(); it != m_customRecipeIngredients.end(); ++it)
        if (ItemInstance::isSame(it->first, item))
            count += it->first->getCount();
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
    for (std::vector<not_null_ptr<ItemInstance> >::size_type i = 0;
         i < m_recipeIngredients.size(); ++i)
        if (ItemInstance::isSame(m_recipeIngredients[i], item))
            return (int)i;
    return -1;
}

void IUIScene_CraftingMenu::UpdateMaintainedInventory()
{
    if (!m_maintainedInventory)
        return;
    m_maintainedInventory->clearContent();
    for (std::vector<std::pair<not_null_ptr<ItemInstance>, bool> >::const_iterator it =
             m_customRecipeIngredients.begin(); it != m_customRecipeIngredients.end(); ++it)
        if (it->first && !it->first->isEmpty())
            m_maintainedInventory->add(it->first->copy());
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
    if ((m_currentTab == 1 && m_craftingMode == 1) ||
        (m_currentTab == 2 && m_currentGroup == 0))
        return 3;
    return 2;
}

bool IUIScene_CraftingMenu::canAddMoreStacks()
{
    if (isSpinnerGroup() && !isInventoryItem())
        return true;
    return m_baseRecipeItem && !m_baseRecipeItem->isEmpty() &&
           BannerBlockEntity::getPatternCount(m_baseRecipeItem) < 6;
}

void IUIScene_CraftingMenu::UpdateHorizontalSlots()
{
    hideAllHSlots();
    for (int slot = 0; slot < m_horizontalSlotCount; ++slot)
    {
        int count = GetRecipeCount(slot);
        if (count <= 0)
            continue;
        int recipe = GetRecipeId(slot, 0);
        Recipe::INGREDIENTS_REQUIRED* ingredients =
            Recipes::getInstance()->getRecipeIngredientsArray((Recipes::ERecipeCategory)m_currentTab);
        if (ingredients != 0 && ingredients[recipe].recipe != 0)
        {
            ItemInstance* result = ingredients[recipe].recipe->getResultItem();
            if (result != 0)
                setCraftHSlotItem(slot, recipe, not_null_ptr<ItemInstance>(new ItemInstance(*result)),
                                  (unsigned int)count);
        }
        setShowCraftHSlot(slot, true);
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
    if (!item || item->isEmpty() || m_customRecipeIngredients.size() >= 9)
        return false;
    int index = GetCustomIngredientIndex(item);
    return index >= 0 && isValidCustomRecipeIngredient(index, item);
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
    if (m_stackedRecipeCounts.empty())
    {
        m_accumulatedItem = ItemInstance::EMPTY;
        return;
    }
    int recipe = m_stackedRecipeCounts.back();
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
    static const unsigned int normalNames[7] =
        { 0xe6a4f105, 0xa00b4cf0, 0xef31f329, 0xf187db6d,
          0x89f72b83, 0x415962d0, 0xac698282 };
    static const unsigned int customNames[6] =
        { 0, 0x133e5cd4, 0xb35ee68b, 0xdd9417ba, 0xf8e4117f, 0x8078384b };
    if (m_currentTab == 2 || m_currentTab == 3)
        return L"";
    if (m_currentTab == 1)
        return group >= 0 && group < 6 && customNames[group] != 0 ?
               L10N::GetString((int)customNames[group]) : L"";
    return group >= 0 && group < 7 ? L10N::GetString((int)normalNames[group]) : L"";
}

void IUIScene_CraftingMenu::refreshAllSlots()
{
    CheckRecipesAvailable();
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[1] = 0;
    m_verticalSlots[2] = 1;
    UpdateSlotsAndHighlight();
    setGroupText(GetGroupNameText(getCurrentGroup()));
}

void IUIScene_CraftingMenu::ClearCustomRecipeIngredients()
{
    m_customRecipeIngredients.clear();
    m_recipeIngredients.clear();
    m_selectedCustomIngredient = -1;
    m_customRecipeValid = false;
    UpdateMaintainedInventory();
    DisplayCustomRecipeIngredients(-1);
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
    ClearCustomRecipeIngredients();
    m_horizontalSlot = 0;
    m_verticalColumn = 1;
    refreshBaseItemSlots(ItemInstance::EMPTY);
    UpdateTabs(false);
    refreshAllSlots();
}

void IUIScene_CraftingMenu::RemoveStackedCraftRecipe()
{
    if (m_stackedRecipeCounts.empty())
        return;
    m_stackedRecipeCounts.pop_back();
    if (m_currentGroup > 0)
        --m_currentGroup;
    updateAccumulatedItem();
    CheckRecipesAvailable();
    UpdateSlotsAndHighlight();
}

bool IUIScene_CraftingMenu::isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance> item)
{
    for (std::vector<not_null_ptr<ItemInstance> >::const_iterator it =
             m_recipeIngredients.begin(); it != m_recipeIngredients.end(); ++it)
        if (ItemInstance::isSame(*it, item))
            return true;
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
    m_customRecipeSlots[0] = 0;
    m_customRecipeSlots[1] = 0;
    m_customRecipeSlots[2] = 0;
    ClearCustomRecipeIngredients();
    showTabHighlight(m_currentGroup, false);
    m_currentGroup = 0;
    refreshBaseItemSlots(ItemInstance::EMPTY);
    m_customIngredientCounts.clear();
    m_baseRecipeIngredients.clear();
    m_horizontalSlot = 0;
    m_verticalColumn = 1;
    UpdateTabs(false);
    refreshAllSlots();
}

void IUIScene_CraftingMenu::NavTabUp()
{
    if (!isCustomRecipe() && GetRecipeCount(m_horizontalSlot) > 0)
        showTabHighlight(m_horizontalSlot, false);
    if (m_currentTab == 0)
        m_currentTab = 3;
    else
        --m_currentTab;
    UpdateVTab();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::NavTabDown()
{
    if (!isCustomRecipe() && GetRecipeCount(m_horizontalSlot) > 0)
        showTabHighlight(m_horizontalSlot, false);
    m_currentTab = m_currentTab == 3 ? 0 : m_currentTab + 1;
    UpdateVTab();
    UpdateTooltips();
}

void IUIScene_CraftingMenu::ItemScrollRight()
{
    int previous = m_horizontalSlot;
    m_horizontalSlot = m_horizontalSlot + 1 >= m_horizontalSlotCount ? 0 : m_horizontalSlot + 1;
    m_verticalColumn = 1;
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[1] = 0;
    m_verticalSlots[2] = 1;
    if (isCustomRecipe())
        m_verticalSlots[1] = m_customRecipeSlots[m_horizontalSlot];
    UpdateVerticalSlots();
    UpdateHighlight();
    if (!isCustomRecipe() && GetRecipeCount(previous) > 0)
        showTabHighlight(previous, false);
}

void IUIScene_CraftingMenu::ItemScrollLeft()
{
    if (m_horizontalSlotCount == 0)
        return;
    int previous = m_horizontalSlot;
    m_horizontalSlot = m_horizontalSlot == 0 ? m_horizontalSlotCount - 1 : m_horizontalSlot - 1;
    m_verticalColumn = 1;
    m_verticalSlots[0] = getCurrentVSlotMax();
    m_verticalSlots[1] = 0;
    m_verticalSlots[2] = 1;
    if (isCustomRecipe())
        m_verticalSlots[1] = m_customRecipeSlots[m_horizontalSlot];
    UpdateVerticalSlots();
    UpdateHighlight();
    if (!isCustomRecipe() && GetRecipeCount(previous) > 0)
        showTabHighlight(previous, false);
}

void IUIScene_CraftingMenu::ItemScrollUp()
{
    int count = getCurrentVSlotCount();
    if (count < 2)
        return;
    if (m_compactInventory || isSpinnerGroup())
        m_verticalSlots[1] = m_verticalSlots[1] == 0 ? count - 1 : m_verticalSlots[1] - 1;
    else if (m_verticalColumn == 0)
        m_verticalSlots[0] = m_verticalSlots[0] == 0 ? count - 1 : m_verticalSlots[0] - 1;
    else
        --m_verticalColumn;
    UpdateVerticalSlots();
    UpdateHighlight();
}

void IUIScene_CraftingMenu::ItemScrollDown()
{
    int count = getCurrentVSlotCount();
    if (count < 2)
        return;
    if (m_compactInventory || isSpinnerGroup())
        m_verticalSlots[1] = m_verticalSlots[1] == count - 1 ? 0 : m_verticalSlots[1] + 1;
    else if (m_verticalColumn == 2)
        m_verticalSlots[2] = m_verticalSlots[2] == count - 1 ? 0 : m_verticalSlots[2] + 1;
    else
        ++m_verticalColumn;
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
            RemoveStackedCraftRecipe();
        else
            m_tooltipState = (m_tooltipState + 1) % 3;
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
        showTabHighlight(m_currentGroup, false);
        m_currentGroup = m_currentGroup + 1 == m_groupPage ? 0 : m_currentGroup + 1;
        UpdateHTab();
    }
    else if (action == 11)
    {
        showTabHighlight(m_currentGroup, false);
        m_currentGroup = m_currentGroup == 0 ? m_groupPage - 1 : m_currentGroup - 1;
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
    if (m_player && m_player->mInventory)
        m_maintainedInventory->replaceWith_NONRECURSIVE(m_player->mInventory);
    m_inventoryMenu = new InventoryMenu(m_maintainedInventory, false, m_player.get());
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
