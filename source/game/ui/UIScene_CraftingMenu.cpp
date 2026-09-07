#include "game/ui/UIScene_CraftingMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/ConsoleUIController.hpp"
#include "game/item/ItemInstance.hpp"
#include "game/ui/FJ_CraftingMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/ui/fuiRect.hpp"

UIScene_CraftingMenu::_SlotInfo::_SlotInfo() : item(), displayValue(0x1f), visible(true)
{
}

UIScene_CraftingMenu::_SlotInfo::~_SlotInfo()
{
}

UIScene_CraftingMenu::UIScene_CraftingMenu(int type, void* data, UILayer* layer)
    : UIScene(type, layer), IUIScene_CraftingMenu(), m_touchDragging(false),
      m_inventoryUpdatesDeferred(true)
{
    IUIScene_CraftingMenu::Init(static_cast<_CraftingPanelScreenInput*>(data));
    initialiseMovie();
    addTimer(1, 100);
}

UIScene_CraftingMenu::~UIScene_CraftingMenu()
{
}

std::wstring UIScene_CraftingMenu::getMoviePath()
{
    if (g_pApp != 0 && g_pApp->GetLocalPlayerCount() >= 2)
        return L"CraftingMenuSplit";
    return L"CraftingMenu";
}

unsigned int UIScene_CraftingMenu::mapElementsAndNames()
{
    UIScene::mapElementsAndNames();
    return 1;
}

int UIScene_CraftingMenu::getSceneType()
{
    return 7;
}

int UIScene_CraftingMenu::getPad()
{
    return m_sceneType;
}

void UIScene_CraftingMenu::hideAllHSlots()
{
    for (int i = 0; i < 5; ++i)
        m_horizontalTab[i].setVisible(false);
}

void UIScene_CraftingMenu::hideAllVSlots()
{
    for (int i = 0; i < 6; ++i)
        m_verticalSlots[i].setVisible(false);
}

void UIScene_CraftingMenu::hideAllIngredientsSlots()
{
    for (int i = 0; i < 2; ++i)
        m_ingredientSlots[i].setVisible(false);
}

void UIScene_CraftingMenu::SetItemDescription(int index, const wchar_t* text)
{
    if (index >= 0 && index < 4)
        m_itemText[index].setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setCraftHSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue)
{
    m_horizontalSlotItems[index].item = item;
    m_horizontalSlotItems[index].visible = true;
    m_horizontalSlotItems[index].displayValue = displayValue;
}

void UIScene_CraftingMenu::setSpecialSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue)
{
    m_horizontalSlotItems[index].item = item;
    m_horizontalSlotItems[index].visible = true;
    m_horizontalSlotItems[index].displayValue = displayValue;
}

void UIScene_CraftingMenu::setCraftVSlotItem(int, int index, not_null_ptr<ItemInstance> item, unsigned int displayValue)
{
    m_verticalSlotItems[index].item = item;
    m_verticalSlotItems[index].visible = true;
    m_verticalSlotItems[index].displayValue = displayValue;
}

void UIScene_CraftingMenu::setCraftingOutputSlotItem(int, not_null_ptr<ItemInstance> item)
{
    m_outputSlotItem.item = item;
    m_outputSlotItem.displayValue = 0x1f;
    m_outputSlotItem.visible = !item->isEmpty();
}

void UIScene_CraftingMenu::setCraftingOutputSlotRedBox(bool show)
{
    m_outputSlots.showSlotRedBox(0, show);
}

void UIScene_CraftingMenu::setIngredientSlotItem(int, int index, not_null_ptr<ItemInstance> item)
{
    m_ingredientSlotItems[index].item = item;
    m_ingredientSlotItems[index].displayValue = 0x1f;
    m_ingredientSlotItems[index].visible = !item->isEmpty();
}

void UIScene_CraftingMenu::setIngredientSlotRedBox(int index, bool show)
{
    if (index >= 0 && index < 2)
        m_ingredientSlots[index].showSlotRedBox(0, show);
}

void UIScene_CraftingMenu::setIngredientSlotHighlighted(int index)
{
    m_ingredientSlots[0].setHighlightSlot(index);
}

void UIScene_CraftingMenu::setInventorySlotHighlighted(int index)
{
    m_inventorySlots[0].setHighlightSlot(index);
}

void UIScene_CraftingMenu::setIngredientDescriptionItem(int, int index, not_null_ptr<ItemInstance> item)
{
    m_ingredientDescriptionItems[index].item = item;
    m_ingredientDescriptionItems[index].displayValue = 0x1f;
    m_ingredientDescriptionItems[index].visible = !item->isEmpty();
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->ShowIngredient(
        index, m_ingredientDescriptionItems[index].visible);
}

void UIScene_CraftingMenu::setIngredientDescriptionRedBox(int index, bool show)
{
    setIngredientSlotRedBox(index, show);
}

void UIScene_CraftingMenu::setIngredientDescriptionText(int index, const wchar_t* text)
{
    SetItemDescription(index, text);
}

void UIScene_CraftingMenu::setPreviewSlotItem(int, not_null_ptr<ItemInstance> item)
{
    m_previewSlotItem.item = item;
    m_previewSlotItem.displayValue = 0x1f;
    m_previewSlotItem.visible = !item->isEmpty();
}

void UIScene_CraftingMenu::setShowCraftHSlot(int index, bool show)
{
    if (index >= 0 && index < 5)
        m_horizontalTab[index].setVisible(show);
}

void UIScene_CraftingMenu::showTabHighlight(int index, bool show)
{
    if (index >= 0 && index < 5)
        m_horizontalTab[index].setFocus(show);
}

void UIScene_CraftingMenu::setHTabEnabled(int index, bool enabled)
{
    setShowCraftHSlot(index, enabled);
}

void UIScene_CraftingMenu::setSpinnerTitle(const wchar_t* text)
{
    m_spinnerTitle.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setGroupText(const wchar_t* text)
{
    m_groupText.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setDescriptionText(const wchar_t* text)
{
    m_description.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setDescriptionTextTop(const wchar_t* text)
{
    m_descriptionTop.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::SetItemName(const wchar_t* text)
{
    m_itemName.setLabel(UIString(text), false, false);
}

void UIScene_CraftingMenu::setItemText(const wchar_t* text)
{
    SetItemName(text);
}

void UIScene_CraftingMenu::scrollDescriptionUp()
{
}

void UIScene_CraftingMenu::scrollDescriptionDown()
{
}

void UIScene_CraftingMenu::updateHighlightAndScrollPositions()
{
    int vertical = 0;
    if (IUIScene_CraftingMenu::getCurrentVSlotCount() == 2)
        vertical = 1;
    else if (IUIScene_CraftingMenu::getCurrentVSlotCount() > 2)
        vertical = 2;
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->MoveSelector(
        m_horizontalSlot, vertical);
    static_cast<FJ_CraftingMenu*>(m_movie->getRootNode()->getFuiNode())->SelectVerticalItem(
        m_verticalColumn);
}

void UIScene_CraftingMenu::updateVSlotPositions(int first, int second)
{
    if (first >= 0 && first < 6)
        m_verticalSlots[first].setHighlightSlot(second);
}

void UIScene_CraftingMenu::UpdateMultiPanel(bool show)
{
    m_description.setVisible(show);
}

void UIScene_CraftingMenu::ShowDescriptionPanel(bool show)
{
    UpdateMultiPanel(show);
}

void UIScene_CraftingMenu::ShowSuperCraftingSlots(int count)
{
    for (int i = 0; i < 6; ++i)
        m_verticalSlots[i].setVisible(i < count);
}

void UIScene_CraftingMenu::SetCraftingMenu(int category, int item, bool enabled)
{
    setShowCraftHSlot(category, enabled);
    updateVSlotPositions(category, item);
}

void UIScene_CraftingMenu::SetHTabItem(int index, not_null_ptr<ItemInstance> item)
{
    m_tabItems[index] = item;
}

void UIScene_CraftingMenu::handleDestroy()
{
    UIScene::handleDestroy();
}

void UIScene_CraftingMenu::handleReload()
{
    m_horizontalSlots.addSlots(0, 6);
    m_previewSlots.addSlots(0, 6);
}

void UIScene_CraftingMenu::handleTimerComplete(int timerId)
{
    if (timerId == 0)
    {
        GetMainPanel()->UpdateControl();
        gConsoleUIController.TouchBoxRebuild(this);
        killTimer(0);
        return;
    }
    if (timerId == 1)
    {
        killTimer(1);
        m_inventoryUpdatesDeferred = false;
    }
}

UIControl* UIScene_CraftingMenu::GetMainPanel()
{
    return &m_horizontalTab[0];
}

void UIScene_CraftingMenu::customDraw(const char*, fuiRect*)
{
}

void UIScene_CraftingMenu::handleTouchBoxRebuild()
{
    addTimer(0, 100);
}

void UIScene_CraftingMenu::handleTouchInput(unsigned int, int, int, int, bool, bool, bool)
{
}

void UIScene_CraftingMenu::handleInput(int, int action, bool repeat, bool released, bool pressed, bool& handled)
{
    gConsoleUIController.AnimateKeyPress(m_sceneType, action, repeat, released, pressed);
    if (action == 14 || action == 15)
    {
        sendInputToMovie(action, repeat, released, pressed);
        return;
    }
    if (released)
        handled = IUIScene_CraftingMenu::handleKeyDown(m_sceneType, action, repeat);
}

bool UIScene_CraftingMenu::allowRepeat(int action)
{
    return action != 2;
}

void UIScene_CraftingMenu::handleInventoryUpdated(void*)
{
    if (!m_inventoryUpdatesDeferred)
        IUIScene_CraftingMenu::HandleInventoryUpdated();
}

void UIScene_CraftingMenu::HandleMessage(EUIMessage message, void* data)
{
    if (message == 0)
        handleInventoryUpdated(data);
}

void UIScene_CraftingMenu::updateTooltips()
{
    IUIScene_CraftingMenu::UpdateTooltips();
}
