#ifndef GAME_UI_UISCENE_CLASSICCRAFTINGMENU_HPP
#define GAME_UI_UISCENE_CLASSICCRAFTINGMENU_HPP

#include <string>

#include "game/ui/UIScene_AbstractContainerMenu.hpp"
#include "game/ui/UIControl_Label.hpp"
#include "game/ui/UIControl_SlotList.hpp"

class UILayer;
struct _UIVec2D
{
    float x;
    float y;
};

class UIScene_ClassicCraftingMenu : public UIScene_AbstractContainerMenu
{
public:
    UIScene_ClassicCraftingMenu(int type, void* data, UILayer* layer);
    virtual ~UIScene_ClassicCraftingMenu();
    virtual unsigned int getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual unsigned int GetSectionAndSlotInDirection(IUIScene_AbstractContainerMenu::ESceneSection section, IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y);
    virtual void GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position);
    virtual void GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position, _UIVec2D* size);
    virtual void handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual void setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap);
    virtual std::wstring getMoviePath();
    virtual unsigned int mapElementsAndNames();
    virtual void handleReload();
    virtual UIControl* getSection(IUIScene_AbstractContainerMenu::ESceneSection section);
    virtual int getSceneType();

private:
    UIControl_SlotList m_resultSlot;
    UIControl_SlotList m_craftingSlots;
    UIControl_Label m_titleLabel;
};

#endif
