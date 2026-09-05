#ifndef GAME_UI_IUISCENE_ABSTRACTCONTAINERMENU_HPP
#define GAME_UI_IUISCENE_ABSTRACTCONTAINERMENU_HPP

#include <vector>
#include <boost/shared_ptr.hpp>

#include "game/core/ClickType.hpp"
#include "game/item/not_null_ptr.hpp"

class AbstractContainerMenu;
class ItemInstance;
class Slot;

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

    Slot* m_menuSlots;
    unsigned char m_initialized;
    unsigned char m_takeAllEnabled;
    unsigned char m_quickCraftDisabled;
    unsigned char m_reserved;
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
    unsigned char m_reserved2[3];
    int m_dragSection;
    int m_dragSlot;
    int m_dragButton;
    int m_dragCount;
    int m_hoverSection;
    int m_hoverSlot;
    int m_hoverX;
    int m_hoverY;
    int m_hoverFlags;
    unsigned int m_virtualMethods;

    IUIScene_AbstractContainerMenu();
    virtual ~IUIScene_AbstractContainerMenu();

    bool IsSectionSlotList(ESceneSection section);
    unsigned int CanHaveFocus(ESceneSection section) const;
    unsigned int IsVisible(ESceneSection section) const;
    virtual unsigned int getSectionColumns(ESceneSection section) = 0;
    virtual unsigned int getSectionRows(ESceneSection section) = 0;
    virtual unsigned int GetSectionAndSlotInDirection(ESceneSection section, ETapState state,
                                                        int* x, int* y) = 0;
    virtual void GetPositionOfSection(ESceneSection section, int* x) = 0;
    virtual void GetItemScreenData(ESceneSection section, int slot, int* x, int* y) = 0;
    virtual int getSectionStartOffset(ESceneSection section) = 0;
    virtual void setSectionSelectedSlot(ESceneSection section, int slot, int direction, bool wrap) = 0;

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
    int GetItemDescription(Slot* slot);
    unsigned int IsSameItemAs(not_null_ptr<ItemInstance> first, not_null_ptr<ItemInstance> second);
    int GetEmptyStackSpace(Slot* slot);
    void recalculateQuickCraftRemaining();
    void onMouseTick();
    int getCurrentIndex(ESceneSection section);
    void slotClicked(int slot, int button, ClickType::eClickType clickType);
    void handleQuickCraft();
    unsigned int handleKeyDown(int key, int value, bool repeat);
    virtual unsigned int handleValidKeyPress(int key, int value, int player);
    virtual void handleOutsideClicked(int x, int y, int button);
    virtual void handleOtherClicked(int button, ESceneSection section, int slot, bool pressed);
    virtual void handleAdditionalKeyPress(int key);
    virtual unsigned long long handleSlotListClicked(ESceneSection section, int slot, int button);
    virtual unsigned long long handleTakeAll(int button);
    virtual int GetSectionHoverText(ESceneSection section);
    virtual int GetInventoryRows();
    virtual unsigned int overrideTooltips(ESceneSection section, not_null_ptr<ItemInstance> item,
                                          bool a, bool b, bool c, int d,
                                          EToolTipItem& e, EToolTipItem& f, EToolTipItem& g,
                                          EToolTipItem& h, EToolTipItem& i, EToolTipItem& j,
                                          EToolTipItem& k);
};

#endif
