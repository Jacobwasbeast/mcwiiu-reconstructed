#ifndef GAME_UI_IUISCENE_CRAFTINGMENU_HPP
#define GAME_UI_IUISCENE_CRAFTINGMENU_HPP

#include <utility>
#include <vector>

#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class _CraftingPanelScreenInput;

class IUIScene_CraftingMenu
{
private:
    unsigned char m_sceneState[0x998];
    std::vector<int> m_recipeIds;
    std::vector<int> m_recipeCounts;
    std::vector<std::pair<int, bool> > m_recipeFlags;
    int m_groupCount;
    int m_currentRecipe;
    int m_currentVSlot;
    std::vector<std::pair<int, int> > m_baseSlots;
    std::vector<std::pair<int, int> > m_spinnerSlots;
    std::vector<std::pair<not_null_ptr<ItemInstance>, bool> > m_customIngredients;
    std::vector<not_null_ptr<ItemInstance> > m_maintainedInventory;
    std::vector<int> m_tabIndices;
    unsigned char m_tailState[0x5b8];
    void* m_virtualTable;

public:
    IUIScene_CraftingMenu();
    ~IUIScene_CraftingMenu();
    unsigned int getGroupAtIndex(int index);
    unsigned int getCurrentGroup();
    unsigned int GetRecipeId(int group, int recipe);
    unsigned int isSpinnerGroup();
    unsigned int isInventoryItem();
    unsigned int isDuplicateItem();
    unsigned int isCustomRecipe();
    unsigned int GetRecipeCount(int group);
    int getCurrentVSlotCount();
    unsigned int isItemSelected(int slot);
    unsigned long long GetBaseItemSlot(unsigned int group, int slot, unsigned int unused, unsigned int flag);
    unsigned long long GetSpinnerSlot(unsigned int group, int slot, int index, unsigned int flag);
    int GetCustomIngredientCount(not_null_ptr<ItemInstance> item);
    unsigned int isValidCustomRecipeIngredient(int slot, not_null_ptr<ItemInstance> item);
    unsigned int GetCustomIngredientIndex(not_null_ptr<ItemInstance> item);
    void UpdateMaintainedInventory();
    unsigned long long DisplayCustomRecipeIngredients(int slot);
    unsigned long long UpdateDescriptionText(bool force);
    void CheckRecipesAvailable();
    unsigned int getNumSpinnerSlots();
    bool canAddMoreStacks();
    void UpdateHorizontalSlots();
    void UpdateVerticalSlots();
    bool stackedCraftingAllowed();
    void DisplayIngredients();
    unsigned int canAddCustomRecipeIngredient(not_null_ptr<ItemInstance> item);
    void UpdateTooltips();
    unsigned long long UpdateHighlight();
    void UpdateSlotsAndHighlight();
    void RemoveLastCustomRecipeIngredient();
    void updateAccumulatedItem();
    int getCurrentVSlotMax();
    unsigned int GetGroupNameText(int group);
    void refreshAllSlots();
    void ClearCustomRecipeIngredients();
    void refreshBaseItemSlots(not_null_ptr<ItemInstance> item);
    void UpdateTabs(bool force);
    void UpdateHTab();
    unsigned long long RemoveStackedCraftRecipe();
    unsigned int isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance> item);
    unsigned long long AddCustomRecipeIngredient(not_null_ptr<ItemInstance> item);
    unsigned int AddStackedCraftRecipe(bool force);
    unsigned long long CraftCurrentItem();
    void UpdateVTab();
    void NavTabUp();
    void NavTabDown();
    void ItemScrollRight();
    unsigned long long ItemScrollLeft();
    void ItemScrollUp();
    void ItemScrollDown();
    unsigned int handleKeyDown(int key, int value, bool repeat);
    void Init(_CraftingPanelScreenInput* input);
    void HandleInventoryUpdated();
};

#endif
