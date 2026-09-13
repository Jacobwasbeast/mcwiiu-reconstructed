#ifndef GAME_UI_IUISCENE_CRAFTINGMENU_HPP
#define GAME_UI_IUISCENE_CRAFTINGMENU_HPP

#include <string>
#include <utility>
#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/item/NonNullList.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class CraftingContainer;
class Inventory;
class ItemInstance;
class Player;

struct _CraftingPanelScreenInput
{
    boost::shared_ptr<Player> player;
    int craftingMode;
    bool compactInventory;
};

class IUIScene_CraftingMenu
{
public:
    IUIScene_CraftingMenu();
    virtual ~IUIScene_CraftingMenu();

    int getGroupAtIndex(int index);
    int getCurrentGroup();
    int GetRecipeId(int group, int recipe);
    bool isSpinnerGroup();
    bool isInventoryItem();
    bool isDuplicateItem();
    bool isCustomRecipe();
    int GetRecipeCount(int group);
    int getCurrentVSlotCount();
    bool isItemSelected(int itemId);
    not_null_ptr<ItemInstance> GetBaseItemSlot(int column, int slot, bool& enabled);
    not_null_ptr<ItemInstance> GetSpinnerSlot(int spinner, int slot, bool& enabled);
    int GetCustomIngredientCount(not_null_ptr<ItemInstance> item);
    bool isValidCustomRecipeIngredient(int index, not_null_ptr<ItemInstance> item);
    int GetCustomIngredientIndex(not_null_ptr<ItemInstance> item);
    void UpdateMaintainedInventory();
    void DisplayCustomRecipeIngredients(int selected);
    void UpdateDescriptionText(bool top);
    void CheckRecipesAvailable();
    int getNumSpinnerSlots();
    bool canAddMoreStacks();
    void UpdateHorizontalSlots();
    void UpdateVerticalSlots();
    bool stackedCraftingAllowed();
    void DisplayIngredients();
    bool canAddCustomRecipeIngredient(not_null_ptr<ItemInstance> item);
    void UpdateTooltips();
    void UpdateHighlight();
    void UpdateSlotsAndHighlight();
    void RemoveLastCustomRecipeIngredient();
    void updateAccumulatedItem();
    int getCurrentVSlotMax();
    const wchar_t* GetGroupNameText(int group);
    void refreshAllSlots();
    void ClearCustomRecipeIngredients();
    void refreshBaseItemSlots(not_null_ptr<ItemInstance> item);
    void UpdateTabs(bool force);
    void UpdateHTab();
    void RemoveStackedCraftRecipe();
    bool isCustomIngredientInBaseRecipe(not_null_ptr<ItemInstance> item);
    void AddCustomRecipeIngredient(not_null_ptr<ItemInstance> item);
    bool AddStackedCraftRecipe(bool all);
    void CraftCurrentItem();
    void UpdateVTab();
    void NavTabUp();
    void NavTabDown();
    void ItemScrollRight();
    void ItemScrollLeft();
    void ItemScrollUp();
    void ItemScrollDown();
    bool handleKeyDown(int key, int action, bool repeat);
    void Init(_CraftingPanelScreenInput* input);
    void HandleInventoryUpdated();

    virtual int getPad() = 0;
    virtual void hideAllHSlots() = 0;
    virtual void hideAllVSlots() = 0;
    virtual void hideAllIngredientsSlots() = 0;
    virtual void setCraftHSlotItem(int slot, int recipe, not_null_ptr<ItemInstance> item, unsigned int count) = 0;
    virtual void setSpecialSlotItem(int slot, int recipe, not_null_ptr<ItemInstance> item, unsigned int count) = 0;
    virtual void setCraftVSlotItem(int slot, int recipe, not_null_ptr<ItemInstance> item, unsigned int count) = 0;
    virtual void setCraftingOutputSlotItem(int slot, not_null_ptr<ItemInstance> item) = 0;
    virtual void setCraftingOutputSlotRedBox(bool red) = 0;
    virtual void setIngredientSlotItem(int slot, int count, not_null_ptr<ItemInstance> item) = 0;
    virtual void setIngredientSlotRedBox(int slot, bool red) = 0;
    virtual void setIngredientSlotHighlighted(int slot) = 0;
    virtual void setInventorySlotHighlighted(int slot) = 0;
    virtual void setIngredientDescriptionItem(int slot, int count, not_null_ptr<ItemInstance> item) = 0;
    virtual void setIngredientDescriptionRedBox(int slot, bool red) = 0;
    virtual void setIngredientDescriptionText(int slot, const wchar_t* text) = 0;
    virtual void setPreviewSlotItem(int slot, not_null_ptr<ItemInstance> item) = 0;
    virtual void setShowCraftHSlot(int slot, bool show) = 0;
    virtual void showTabHighlight(int tab, bool show) = 0;
    virtual void setHTabEnabled(int tab, bool enabled) = 0;
    virtual void setSpinnerTitle(const wchar_t* title) = 0;
    virtual void setGroupText(const wchar_t* text) = 0;
    virtual void setDescriptionText(const wchar_t* text) = 0;
    virtual void setDescriptionTextTop(const wchar_t* text) = 0;
    virtual void setItemText(const wchar_t* text) = 0;
    virtual void scrollDescriptionUp() = 0;
    virtual void scrollDescriptionDown() = 0;
    virtual void updateHighlightAndScrollPositions() = 0;
    virtual void updateVSlotPositions(int first, int count) = 0;
    virtual void UpdateMultiPanel(bool show) = 0;
    virtual void ShowDescriptionPanel(bool show) = 0;
    virtual void ShowSuperCraftingSlots(int count) = 0;
    virtual void SetCraftingMenu(int horizontalSlot, int verticalSlot, bool spinner) = 0;
    virtual void SetHTabItem(int tab, not_null_ptr<ItemInstance> item) = 0;

protected:
    int m_recipeTable[12][50];
    int m_horizontalSlot;
    int m_verticalColumn;
    int m_recipePage;
    int m_craftingMode;
    boost::shared_ptr<Player> m_player;
    int m_currentGroup;
    int m_groupPage;
    int m_verticalSlots[3];
    int* m_groupMap;
    boost::shared_ptr<CraftingContainer> m_craftingContainer;
    std::vector<int> m_inventoryRecipes;
    std::vector<int> m_availableRecipes;
    std::vector<std::pair<int, bool> > m_duplicateRecipes;
    not_null_ptr<ItemInstance> m_baseRecipeItem;
    int m_selectedCustomIngredient;
    std::vector<std::pair<int, int> > m_customIngredientCounts;
    not_null_ptr<ItemInstance> m_accumulatedItem;
    std::vector<std::pair<int, int> > m_baseRecipeIngredients;
    int m_hasPaper;
    bool m_hasGunpowder;
    std::vector<std::pair<not_null_ptr<ItemInstance>, bool> > m_customRecipes[3];
    std::vector<std::pair<not_null_ptr<ItemInstance>, bool> > m_customRecipeIngredients;
    std::vector<not_null_ptr<ItemInstance> > m_recipeIngredients;
    NonNullList<not_null_ptr<ItemInstance> > m_inventoryItems;
    int m_customRecipeSlots[3];
    int m_customRecipeValid;
    std::vector<int> m_stackedRecipeCounts;
    int m_inventoryStart;
    bool m_inventoryEnd;
    boost::shared_ptr<Inventory> m_maintainedInventory;
    AbstractContainerMenu* m_inventoryMenu;
    int m_horizontalSlotCount;
    int m_lastRecipe;
    int m_tooltipPlayer;
    int m_tooltipState;
    int m_ingredientCount;
    int m_tabsDirty;
    int m_compactInventory;
    wchar_t m_ingredientDescriptionText[128][5];
    int m_lastTab;
    int m_currentTab;
};

#endif
