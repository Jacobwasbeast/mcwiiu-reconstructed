#ifndef GAME_UI_UISCENE_CREATIVEMENU_HPP
#define GAME_UI_UISCENE_CREATIVEMENU_HPP

#include <boost/shared_ptr.hpp>
#include <string>
#include <vector>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/AbstractContainerMenu.hpp"
#include "game/ui/UIControl_SlotList.hpp"
#include "game/ui/UIControl_Touch.hpp"

class AbstractContainerMenu;
class Inventory;
class Player;
class SimpleContainer;
class UILayer;

struct _UIVec2D
{
    float x;
    float y;
};

class UIScene_CreativeMenu : public UIScene_AbstractContainerMenu
{
public:
    enum ECreativeInventoryTabs
    {
        ECreativeInventoryTabs_Building = 0,
        ECreativeInventoryTabs_Decoration = 1,
        ECreativeInventoryTabs_Redstone = 2,
        ECreativeInventoryTabs_Transportation = 3,
        ECreativeInventoryTabs_Miscellaneous = 4,
        ECreativeInventoryTabs_Food = 5,
        ECreativeInventoryTabs_Tools = 6,
        ECreativeInventoryTabs_Combat = 7
    };

    enum ECreative_Inventory_Groups
    {
        ECreative_Inventory_Groups_None = -1
    };

    class TabSpec
    {
    public:
        TabSpec(const wchar_t* title, int itemCount, int groupCount,
                ECreative_Inventory_Groups* groups, int specialCount,
                ECreative_Inventory_Groups* specialGroups, int extraCount,
                ECreative_Inventory_Groups* extraGroups);
        unsigned int getPageCount();
        void populateMenu(AbstractContainerMenu* menu, int page, unsigned int offset);
        ~TabSpec();

    private:
        std::wstring m_title;
        int m_itemCount;
        int m_groupCount;
        std::vector<ECreative_Inventory_Groups> m_groups;
        int m_specialCount;
        std::vector<ECreative_Inventory_Groups> m_specialGroups;
        int m_extraCount;
        std::vector<ECreative_Inventory_Groups> m_extraGroups;
        int m_pageCount;
    };

    class ItemPickerMenu : public AbstractContainerMenu
    {
    public:
        ItemPickerMenu(boost::shared_ptr<SimpleContainer> container,
                       boost::shared_ptr<Inventory> inventory);
        virtual ~ItemPickerMenu();
        virtual bool stillValid(boost::shared_ptr<Player> player);
        virtual bool isOverrideResultClick(int slot, int button);
        virtual bool doQuickMoveLoop();

    private:
        boost::shared_ptr<SimpleContainer> m_container;
        boost::shared_ptr<Inventory> m_inventory;
        int m_selectedSlot;
        int m_resultSlot;
    };

    UIScene_CreativeMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_CreativeMenu();

    virtual bool IsSectionSlotList(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int CanHaveFocus(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                                       IUIScene_AbstractContainerMenu::ETapState state,
                                                       int* x, int* y);
    virtual void GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section,
                                      _UIVec2D* position);
    virtual void GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
                                   _UIVec2D* position, _UIVec2D* size);
    virtual unsigned int handleValidKeyPress(int key, int value, int player);
    virtual void handleOutsideClicked(int x, int y, int button);
    virtual void updateTabHighlightAndText(ECreativeInventoryTabs tab);
    virtual void updateScrollCurrentPage(int page, int pageCount);
    virtual void switchTab(ECreativeInventoryTabs tab);
    virtual void ScrollBar(_UIVec2D position);
    virtual void handleOtherClicked(int button, IUIScene_AbstractContainerMenu::ESceneSection section,
                                    int slot, bool pressed);
    virtual void handleAdditionalKeyPress(int key);
    virtual int getEmptyInventorySlot(not_null_ptr<ItemInstance> item, int& slot);
    virtual void handleSlotListClicked(IUIScene_AbstractContainerMenu::ESceneSection section,
                                       int slot, int button);
    virtual int getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section,
                                        int slot, int direction, bool wrap);
    virtual unsigned int overrideTooltips(IUIScene_AbstractContainerMenu::ESceneSection section,
                                          not_null_ptr<ItemInstance> item, bool itemValid, bool canTake,
                                          bool canPlace, int count,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& first,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& second,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& third,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& fourth,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& fifth,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& sixth,
                                          IUIScene_AbstractContainerMenu::EToolTipItem& seventh);
    static void staticCtor();
    virtual unsigned int mapElementsAndNames();
    virtual std::wstring getMoviePath();
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void handleInput(int playerId, int action, bool repeat, bool released, bool pressed,
                             bool& handled);
    virtual void handleTouchInput(unsigned int playerId, int action, int x, int y, bool repeat,
                                  bool released, bool pressed);
    virtual UIControl* GetMainPanel();
    virtual void handleTouchBoxRebuild();
    virtual void handleTimerComplete(int timerId);
    virtual void handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSceneType();

private:
    UIControl_SlotList m_hotbarList;
    UIControl_Touch m_touchPanels[9];
    UIControl_SlotList m_containerList;
    int m_currentTab;
    int m_tabPages[8];
    int m_tabCurrentPages[8];
    AbstractContainerMenu* m_containerMenu;
    IUIScene_AbstractContainerMenu* m_sceneInterface;
};

#endif
