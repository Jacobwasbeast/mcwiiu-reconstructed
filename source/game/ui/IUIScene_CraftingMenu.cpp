#include "game/ui/IUIScene_CraftingMenu.hpp"

#include <cstddef>

IUIScene_CraftingMenu::IUIScene_CraftingMenu()
    : m_groupCount(0), m_currentRecipe(0), m_currentVSlot(0), m_virtualTable(0)
{
}

IUIScene_CraftingMenu::~IUIScene_CraftingMenu()
{
}

unsigned int IUIScene_CraftingMenu::getGroupAtIndex(int index)
{
    return m_tabIndices[m_groupCount * 7 + index];
}

unsigned int IUIScene_CraftingMenu::getCurrentGroup()
{
    return getGroupAtIndex(m_currentRecipe);
}

unsigned int IUIScene_CraftingMenu::GetRecipeId(int group, int recipe)
{
    if (group < 0 || recipe < 0) return 0;
    if ((std::size_t)group >= m_recipeIds.size()) return 0;
    return (unsigned int)m_recipeIds[(std::size_t)group];
}

unsigned int IUIScene_CraftingMenu::isSpinnerGroup()
{
    return m_currentRecipe != 0 && m_groupCount >= 1 && m_groupCount <= 3;
}

unsigned int IUIScene_CraftingMenu::isInventoryItem()
{
    return isSpinnerGroup() == 0 && m_currentVSlot >= 0 && m_currentVSlot < (int)m_recipeIds.size();
}

unsigned int IUIScene_CraftingMenu::isDuplicateItem()
{
    return m_recipeFlags.size() > 1 && m_recipeFlags[0].first == m_recipeFlags[1].first;
}

unsigned int IUIScene_CraftingMenu::isCustomRecipe()
{
    return m_groupCount == 3;
}

unsigned int IUIScene_CraftingMenu::GetRecipeCount(int group)
{
    if (group < 0 || (std::size_t)group >= m_recipeCounts.size()) return 0;
    return (unsigned int)m_recipeCounts[(std::size_t)group];
}

int IUIScene_CraftingMenu::getCurrentVSlotCount()
{
    return (int)m_spinnerSlots.size();
}

unsigned int IUIScene_CraftingMenu::isItemSelected(int slot)
{
    return slot == m_currentVSlot;
}

unsigned long long IUIScene_CraftingMenu::GetBaseItemSlot(unsigned int, int, unsigned int, unsigned int)
{
    return 0;
}

unsigned long long IUIScene_CraftingMenu::GetSpinnerSlot(unsigned int, int, int, unsigned int)
{
    return 0;
}

int IUIScene_CraftingMenu::GetCustomIngredientCount(not_null_ptr<ItemInstance>)
{
    return (int)m_customIngredients.size();
}

unsigned int IUIScene_CraftingMenu::isValidCustomRecipeIngredient(int, not_null_ptr<ItemInstance>)
{
    return 1;
}

unsigned int IUIScene_CraftingMenu::GetCustomIngredientIndex(not_null_ptr<ItemInstance>)
{
    return (unsigned int)m_customIngredients.size();
}

void IUIScene_CraftingMenu::UpdateMaintainedInventory()
{
    m_maintainedInventory.clear();
}

unsigned long long IUIScene_CraftingMenu::DisplayCustomRecipeIngredients(int)
{
    return 0;
}

unsigned long long IUIScene_CraftingMenu::UpdateDescriptionText(bool)
{
    return 0;
}

void IUIScene_CraftingMenu::CheckRecipesAvailable()
{
}

unsigned int IUIScene_CraftingMenu::getNumSpinnerSlots()
{
    return (unsigned int)m_spinnerSlots.size();
}

bool IUIScene_CraftingMenu::canAddMoreStacks()
{
    return m_recipeCounts.size() < 64;
}

void IUIScene_CraftingMenu::UpdateHorizontalSlots()
{
}

void IUIScene_CraftingMenu::UpdateVerticalSlots()
{
}

bool IUIScene_CraftingMenu::stackedCraftingAllowed()
{
    return isCustomRecipe() == 0;
}

void IUIScene_CraftingMenu::DisplayIngredients()
{
}

unsigned int IUIScene_CraftingMenu::canAddCustomRecipeIngredient(not_null_ptr<ItemInstance>)
{
    return m_customIngredients.size() < 9;
}

void IUIScene_CraftingMenu::UpdateTooltips()
{
}

unsigned long long IUIScene_CraftingMenu::UpdateHighlight()
{
    return 0;
}

void IUIScene_CraftingMenu::UpdateSlotsAndHighlight()
{
    UpdateHorizontalSlots();
    UpdateVerticalSlots();
    UpdateHighlight();
}

void IUIScene_CraftingMenu::RemoveLastCustomRecipeIngredient()
{
    if (!m_customIngredients.empty()) m_customIngredients.pop_back();
}

void IUIScene_CraftingMenu::updateAccumulatedItem()
{
}

int IUIScene_CraftingMenu::getCurrentVSlotMax()
{
    return getCurrentVSlotCount();
}

unsigned int IUIScene_CraftingMenu::GetGroupNameText(int group)
{
    return (unsigned int)group;
}

void IUIScene_CraftingMenu::refreshAllSlots()
{
    UpdateSlotsAndHighlight();
}

void IUIScene_CraftingMenu::ClearCustomRecipeIngredients()
{
    m_customIngredients.clear();
}

void IUIScene_CraftingMenu::refreshBaseItemSlots(not_null_ptr<ItemInstance>)
{
}

void IUIScene_CraftingMenu::UpdateTabs(bool)
{
}

void IUIScene_CraftingMenu::UpdateHTab()
{
}

unsigned long long IUIScene_CraftingMenu::RemoveStackedCraftRecipe()
{
    return 0;
}

unsigned int IUIScene_CraftingMenu::isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance>)
{
    return 0;
}

unsigned long long IUIScene_CraftingMenu::AddCustomRecipeIngredient(not_null_ptr<ItemInstance> item)
{
    if (canAddCustomRecipeIngredient(item) != 0) m_customIngredients.push_back(std::make_pair(item, true));
    return 0;
}

unsigned int IUIScene_CraftingMenu::AddStackedCraftRecipe(bool)
{
    return canAddMoreStacks() ? 1 : 0;
}

unsigned long long IUIScene_CraftingMenu::CraftCurrentItem()
{
    return 0;
}

void IUIScene_CraftingMenu::UpdateVTab()
{
}

void IUIScene_CraftingMenu::NavTabUp()
{
    if (m_currentRecipe > 0) --m_currentRecipe;
}

void IUIScene_CraftingMenu::NavTabDown()
{
    if (m_currentRecipe + 1 < m_groupCount) ++m_currentRecipe;
}

void IUIScene_CraftingMenu::ItemScrollRight()
{
    ++m_currentVSlot;
}

unsigned long long IUIScene_CraftingMenu::ItemScrollLeft()
{
    if (m_currentVSlot > 0) --m_currentVSlot;
    return (unsigned long long)(unsigned int)m_currentVSlot;
}

void IUIScene_CraftingMenu::ItemScrollUp()
{
    NavTabUp();
}

void IUIScene_CraftingMenu::ItemScrollDown()
{
    NavTabDown();
}

unsigned int IUIScene_CraftingMenu::handleKeyDown(int key, int value, bool)
{
    if (key == 0) NavTabUp();
    else if (key == 1) NavTabDown();
    else if (value < 0) ItemScrollLeft();
    else if (value > 0) ItemScrollRight();
    return 1;
}

void IUIScene_CraftingMenu::Init(_CraftingPanelScreenInput*)
{
}

void IUIScene_CraftingMenu::HandleInventoryUpdated()
{
    CheckRecipesAvailable();
    UpdateSlotsAndHighlight();
    UpdateTooltips();
}
