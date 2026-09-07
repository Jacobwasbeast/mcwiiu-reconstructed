#ifndef GAME_UI_UISCENE_CRAFTINGMENU_HPP
#define GAME_UI_UISCENE_CRAFTINGMENU_HPP

#include <string>

#include "game/ui/UIScene.hpp"
#include "game/ui/IUIScene_CraftingMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIControl_HTMLLabel.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/item/not_null_ptr.hpp"

class ItemInstance;
class UILayer;
struct fuiRect;

class UIScene_CraftingMenu : public UIScene
{
public:
    class _SlotInfo
    {
    public:
        _SlotInfo();
        ~_SlotInfo();
        int slot;
        int itemId;
        int count;
        bool visible;
    };

    UIScene_CraftingMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_CraftingMenu();
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual int getSceneType();
    virtual void handleTimerComplete(int timerId);
    virtual void updateTooltips();
    virtual unsigned int GetMainPanel();
    virtual void customDraw(const char* name, fuiRect* rect);
    virtual bool allowRepeat(int action);
    virtual void handleInput(int player, int action, bool repeat, bool released, bool pressed, bool& handled);
    virtual void handleDestroy();
    virtual void handleReload();
    virtual void handleTouchBoxRebuild();
    virtual void handleTouchInput(unsigned int player, int action, int x, int y, bool repeat, bool released, bool pressed);

    unsigned int getPad();
    void hideAllHSlots();
    void hideAllVSlots();
    void SetItemDescription(int index, const wchar_t* text);
    void hideAllIngredientsSlots();
    void setCraftHSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setSpecialSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setCraftVSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setCraftingOutputSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setCraftingOutputSlotRedBox(bool show);
    void setIngredientSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setIngredientSlotRedBox(int index, bool show);
    void setIngredientSlotHighlighted(int index);
    void setInventorySlotHighlighted(int index);
    void setIngredientDescriptionItem(int index, not_null_ptr<ItemInstance> item);
    void setIngredientDescriptionRedBox(int index, bool show);
    void setIngredientDescriptionText(int index, const wchar_t* text);
    void setPreviewSlotItem(int index, not_null_ptr<ItemInstance> item);
    void setShowCraftHSlot(int index, bool show);
    void showTabHighlight(int index, bool show);
    void setHTabEnabled(int index, bool enabled);
    void setSpinnerTitle(const wchar_t* text);
    void setGroupText(const wchar_t* text);
    void setDescriptionText(const wchar_t* text);
    void setDescriptionTextTop(const wchar_t* text);
    void SetItemName(const wchar_t* text);
    void setItemText(const wchar_t* text);
    void scrollDescriptionUp();
    void scrollDescriptionDown();
    void updateHighlightAndScrollPositions();
    void updateVSlotPositions(int first, int second);
    void UpdateMultiPanel(bool show);
    void ShowDescriptionPanel(bool show);
    void ShowSuperCraftingSlots(int count);
    void SetCraftingMenu(int category, int item, bool enabled);
    void SetHTabItem(int index, not_null_ptr<ItemInstance> item);
    void handleInventoryUpdated(void* data);
    void HandleMessage(EUIMessage message, void* data);

private:
    IUIScene_CraftingMenu m_sceneInterface;
    UIControl m_horizontalTab[5];
    UIControl_SlotList m_horizontalSlots;
    UIControl_SlotList m_verticalSlots[6];
    UIControl_SlotList m_ingredientSlots[2];
    UIControl_SlotList m_inventorySlots[6];
    UIControl_SlotList m_previewSlots;
    UIControl_SlotList m_outputSlots;
    UIControl_HTMLLabel m_description;
    UIControl_HTMLLabel m_descriptionTop;
    UIControl_Label m_spinnerTitle;
    UIControl_Label m_groupText;
    UIControl_Label m_itemName;
    UIControl_Label m_itemText[4];
    int m_pad;
};

#endif
