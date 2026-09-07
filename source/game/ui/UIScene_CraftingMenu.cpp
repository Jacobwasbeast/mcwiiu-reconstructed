#include "game/ui/UIScene_CraftingMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/ui/FJ_CraftingMenu.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRect.hpp"

UIScene_CraftingMenu::_SlotInfo::_SlotInfo() : slot(0), itemId(0), count(0), visible(false)
{
}

UIScene_CraftingMenu::_SlotInfo::~_SlotInfo()
{
}

UIScene_CraftingMenu::UIScene_CraftingMenu(int type, void*, UILayer* layer)
    : UIScene(type, layer), m_sceneInterface(), m_pad(0)
{
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

unsigned int UIScene_CraftingMenu::getPad()
{
    return static_cast<unsigned int>(m_pad);
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

void UIScene_CraftingMenu::setCraftHSlotItem(int index, not_null_ptr<ItemInstance>)
{
    if (index >= 0 && index < 6)
        m_horizontalSlots.setSlotHighlighted(index, false);
}

void UIScene_CraftingMenu::setSpecialSlotItem(int index, not_null_ptr<ItemInstance> item)
{
    setCraftHSlotItem(index, item);
}

void UIScene_CraftingMenu::setCraftVSlotItem(int index, not_null_ptr<ItemInstance> item)
{
    setPreviewSlotItem(index, item);
}

void UIScene_CraftingMenu::setCraftingOutputSlotItem(int index, not_null_ptr<ItemInstance> item)
{
    setPreviewSlotItem(index, item);
}

void UIScene_CraftingMenu::setCraftingOutputSlotRedBox(bool show)
{
    m_outputSlots.showSlotRedBox(0, show);
}

void UIScene_CraftingMenu::setIngredientSlotItem(int index, not_null_ptr<ItemInstance>)
{
    if (index >= 0 && index < 2)
        m_ingredientSlots[index].setVisible(true);
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

void UIScene_CraftingMenu::setIngredientDescriptionItem(int index, not_null_ptr<ItemInstance> item)
{
    setIngredientSlotItem(index, item);
}

void UIScene_CraftingMenu::setIngredientDescriptionRedBox(int index, bool show)
{
    setIngredientSlotRedBox(index, show);
}

void UIScene_CraftingMenu::setIngredientDescriptionText(int index, const wchar_t* text)
{
    SetItemDescription(index, text);
}

void UIScene_CraftingMenu::setPreviewSlotItem(int index, not_null_ptr<ItemInstance>)
{
    if (index >= 0 && index < 6)
        m_previewSlots.setSlotHighlighted(index, false);
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
    updateTooltips();
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
    setCraftHSlotItem(index, item);
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

void UIScene_CraftingMenu::handleTimerComplete(int)
{
    updateTooltips();
}

unsigned int UIScene_CraftingMenu::GetMainPanel()
{
    return 0;
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

void UIScene_CraftingMenu::handleInput(int, int, bool, bool, bool, bool& handled)
{
    handled = false;
}

bool UIScene_CraftingMenu::allowRepeat(int action)
{
    return action != 0;
}

void UIScene_CraftingMenu::handleInventoryUpdated(void*)
{
    updateHighlightAndScrollPositions();
}

void UIScene_CraftingMenu::HandleMessage(EUIMessage message, void* data)
{
    UIScene::HandleMessage(message, data);
}

void UIScene_CraftingMenu::updateTooltips()
{
}
