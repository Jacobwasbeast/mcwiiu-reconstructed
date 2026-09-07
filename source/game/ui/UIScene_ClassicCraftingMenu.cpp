#include "game/ui/UIScene_ClassicCraftingMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIScene_ClassicCraftingMenu::UIScene_ClassicCraftingMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_resultSlot(), m_craftingSlots(), m_titleLabel()
{
    initialiseMovie();
}

UIScene_ClassicCraftingMenu::~UIScene_ClassicCraftingMenu()
{
}

unsigned int UIScene_ClassicCraftingMenu::getSectionColumns(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 9, 9, 1, 3 };
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51))
        return 0;
    return columns[static_cast<int>(section) - 0x4e];
}

unsigned int UIScene_ClassicCraftingMenu::getSectionRows(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e)) return 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e)) return 1;
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50)) return GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50)) return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51)) return 3;
    return 0;
}

unsigned int UIScene_ClassicCraftingMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection result = section;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e) &&
        state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Down)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f);
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Down)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Up)
        result = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e);
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50) &&
        (state == IUIScene_AbstractContainerMenu::ETapState_Down ||
         state == IUIScene_AbstractContainerMenu::ETapState_Up))
        updateSlotPosition(section, result, state, x, y, -6, 0);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Left)
        updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51), state, x, y, 0, -1);
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50) &&
             state == IUIScene_AbstractContainerMenu::ETapState_Right)
    {
        if (x != 0) *x = 0;
        updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51), state, x, y, 0, -1);
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51) &&
             (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
              state == IUIScene_AbstractContainerMenu::ETapState_Right))
        updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50), state, x, y, 0, 0);
    else
        updateSlotPosition(section, result, state, x, y, 0, 0);
    return static_cast<unsigned int>(result);
}

void UIScene_ClassicCraftingMenu::GetPositionOfSection(IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e)) control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f)) control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50)) control = &m_resultSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51)) control = &m_craftingSlots;
    if (control != 0 && position != 0) { position->x = static_cast<float>(control->getXPos()); position->y = static_cast<float>(control->getYPos()); }
}

void UIScene_ClassicCraftingMenu::GetItemScreenData(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e)) control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f)) control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50)) control = &m_resultSlot;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51)) control = &m_craftingSlots;
    unsigned int columns = getSectionColumns(section), rows = getSectionRows(section);
    if (control == 0 || position == 0 || size == 0 || columns == 0 || rows == 0) return;
    size->x = static_cast<float>(control->getWidth()) / columns;
    size->y = static_cast<float>(control->getHeight()) / rows;
    position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
    position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
}

void UIScene_ClassicCraftingMenu::handleSectionClick(IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_ClassicCraftingMenu::getSectionStartOffset(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char offsets[] = { 37, 10, 0, 1 };
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e) || section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51)) return 0;
    return offsets[static_cast<int>(section) - 0x4e];
}

void UIScene_ClassicCraftingMenu::setSectionSelectedSlot(IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e)) m_hotbarList.setHighlightSlot(slot + direction * getSectionColumns(section));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f)) m_inventoryList.setHighlightSlot(slot + direction * getSectionColumns(section));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50)) m_resultSlot.setHighlightSlot(slot + direction * getSectionColumns(section));
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51)) m_craftingSlots.setHighlightSlot(slot + direction * getSectionColumns(section));
}

std::wstring UIScene_ClassicCraftingMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"ClassicCraftingMenu") : std::wstring(L"ClassicCraftingMenuSplit");
}

unsigned int UIScene_ClassicCraftingMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_backgroundPanel.getFuiRenderNode();
    m_resultSlot.setupControl(this, panel, "Result");
    m_controls.push_back(&m_resultSlot);
    m_craftingSlots.setupControl(this, panel, "CraftingList");
    m_controls.push_back(&m_craftingSlots);
    m_titleLabel.setupControl(this, panel, "TitleLabel");
    m_controls.push_back(&m_titleLabel);
    return 1;
}

void UIScene_ClassicCraftingMenu::handleReload()
{
    Initialize(true, m_containerId, m_menu, true, 10,
               static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e),
               static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x52), false);
    m_resultSlot.addSlots(0, 1);
    m_craftingSlots.addSlots(1, 9);
}

UIControl* UIScene_ClassicCraftingMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4e)) return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x4f)) return &m_inventoryList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x50)) return &m_resultSlot;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x51)) return &m_craftingSlots;
    return 0;
}

int UIScene_ClassicCraftingMenu::getSceneType()
{
    return 0x38;
}
