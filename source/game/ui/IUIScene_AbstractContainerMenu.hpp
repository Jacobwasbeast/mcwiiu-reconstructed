#ifndef GAME_UI_IUISCENE_ABSTRACTCONTAINERMENU_HPP
#define GAME_UI_IUISCENE_ABSTRACTCONTAINERMENU_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/ClickType.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class Container;
class HtmlString;
class ItemInstance;
class Slot;
struct _UIVec2D;

class IUIScene_AbstractContainerMenu
{
public:
    enum ESceneSection
    {
        ESceneSection_None = -1
    };

    enum ETapState
    {
        ETapState_None = 0,
        ETapState_Up = 1,
        ETapState_Down = 2,
        ETapState_Left = 3,
        ETapState_Right = 4
    };

    enum EToolTipButton
    {
        EToolTipButton_First = 0,
        EToolTipButton_Second = 1,
        EToolTipButton_Third = 2,
        EToolTipButton_Fourth = 3
    };

    enum EToolTipItem
    {
        EToolTipItem_None = -1
    };

    AbstractContainerMenu* m_menu;
    unsigned char m_initialized;
    unsigned char m_takeAllEnabled;
    unsigned char m_quickCraftDisabled;
    int m_containerId;
    float m_touchX;
    float m_touchY;
    int m_sectionStart;
    int m_sectionSize;
    int m_sectionRows;
    int m_sectionColumns;
    int m_selectedSlot;
    unsigned char m_tooltipButtons[4];
    unsigned char m_tooltipItems[4];
    int m_activeTooltip;
    int m_slotIndex;
    int m_slotCount;
    int m_currentIndex;
    int m_dragStart;
    int m_dragEnd;
    int m_quickCraftRemaining;
    int m_quickCraftSlots;
    unsigned char m_inputState[8];
    int m_pointerX;
    int m_pointerY;
    int m_pointerSection;
    int m_pointerSlot;
    int m_pointerButton;
    int m_pointerItem;
    int m_pointerFlags;
    int m_lastInput;
    unsigned char m_inventorySmall;
    int m_dragSection;
    int m_dragSlot;
    int m_dragButton;
    int m_dragCount;
    int m_hoverSection;
    int m_hoverSlot;
    int m_hoverX;
    int m_hoverY;
    int m_hoverFlags;

    IUIScene_AbstractContainerMenu();
    virtual bool IsSectionSlotList(ESceneSection section);
    virtual unsigned int CanHaveFocus(ESceneSection section);
    virtual unsigned int IsVisible(ESceneSection section);
    virtual unsigned int getSectionColumns(ESceneSection section) = 0;
    virtual unsigned int getSectionRows(ESceneSection section) = 0;
    virtual unsigned int GetSectionAndSlotInDirection(ESceneSection section, ETapState state,
                                                        int* x, int* y) = 0;
    virtual void GetPositionOfSection(ESceneSection section, _UIVec2D* position) = 0;
    virtual void GetItemScreenData(ESceneSection section, int slot, _UIVec2D* position,
                                   _UIVec2D* size) = 0;
    virtual void PlatformInitialize(bool enabled, int playerIndex, int slotIndex) = 0;
    virtual void InitDataAssociations(int playerIndex, AbstractContainerMenu* menu, int slotIndex) = 0;
    virtual unsigned int handleValidKeyPress(int key, int value, int player);
    virtual void handleOutsideClicked(int x, int y, int button);
    virtual void handleOtherClicked(int button, ESceneSection section, int slot, bool pressed);
    virtual void handleAdditionalKeyPress(int key);
    virtual void handleSlotListClicked(ESceneSection section, int slot, int button);
    virtual void handleTakeAll(int button);
    virtual void handleSectionClick(ESceneSection section) = 0;
    virtual int getSectionStartOffset(ESceneSection section) = 0;
    virtual bool doesSectionTreeHaveFocus(ESceneSection section) = 0;
    virtual void setSectionFocus(ESceneSection section, int direction) = 0;
    virtual void setSectionSelectedSlot(ESceneSection section, int slot, int direction, bool wrap) = 0;
    virtual void setFocusToPointer(int pointer) = 0;
    virtual void SetPointerText(std::vector<HtmlString>* text, bool visible) = 0;
    virtual int GetSectionHoverText(ESceneSection section);
    virtual not_null_ptr<ItemInstance> getSlotItem(ESceneSection section, int slot) = 0;
    virtual Slot* getSlot(ESceneSection section, int slot) = 0;
    virtual bool isSlotEmpty(ESceneSection section, int slot) = 0;
    virtual void adjustPointerForSafeZone() = 0;
    virtual unsigned int overrideTooltips(ESceneSection section, not_null_ptr<ItemInstance> item,
                                          bool itemValid, bool canTake, bool canPlace, int count,
                                          EToolTipItem& first, EToolTipItem& second,
                                          EToolTipItem& third, EToolTipItem& fourth,
                                          EToolTipItem& fifth, EToolTipItem& sixth,
                                          EToolTipItem& seventh);
    virtual ~IUIScene_AbstractContainerMenu();
    virtual int getPad() = 0;

    unsigned int canTakeAll() const;
    unsigned int canQuickCraft() const;
    int GetSectionDimensions(ESceneSection section, int* columns, int* rows);
    void updateSlotPosition(ESceneSection from, ESceneSection to, ETapState state,
                            int* x, int* y, int dx, int dy);
    unsigned int GetTapInputType(float x, float y);
    void UpdateTooltips();
    void SetToolTip(EToolTipButton button, EToolTipItem item);
    void Initialize(bool takeAll, int containerId, AbstractContainerMenu* menu,
                    bool smallInventory, int slotIndex, ESceneSection firstSection,
                    ESceneSection lastSection, bool touchEnabled);
    std::vector<HtmlString>* GetItemDescription(Slot* slot);
    unsigned int IsSameItemAs(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second);
    int GetEmptyStackSpace(Slot* slot);
    void recalculateQuickCraftRemaining();
    void onMouseTick();
    int getCurrentIndex(ESceneSection section);
    void slotClicked(int slot, int button, ClickType::eClickType clickType);
    void handleQuickCraft();
    unsigned int handleKeyDown(int key, int value, bool repeat);
    int GetInventoryRows();
};

#endif
