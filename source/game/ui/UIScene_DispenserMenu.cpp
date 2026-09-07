#include "game/ui/UIScene_DispenserMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CMinecraftApp* g_pApp;

UIScene_DispenserMenu::UIScene_DispenserMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_dispenserSlot(), m_dispenserLabel(), m_sceneInterface(0)
{
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(
        static_cast<UIScene_AbstractContainerMenu*>(this));
    initialiseMovie();
    m_dispenserSlot.addSlots(0, 9);
}

UIScene_DispenserMenu::~UIScene_DispenserMenu()
{
}

unsigned int UIScene_DispenserMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
        return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
        return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
        return 3;
    return 0;
}

unsigned int UIScene_DispenserMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
        return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
        return IUIScene_AbstractContainerMenu::GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
        return 3;
    return 0;
}

unsigned int UIScene_DispenserMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const IUIScene_AbstractContainerMenu::ESceneSection hotbar =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11);
    const IUIScene_AbstractContainerMenu::ESceneSection inventory =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12);
    const IUIScene_AbstractContainerMenu::ESceneSection dispenser =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13);

    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, dispenser, state, x, y, 3, 0);
        return dispenser;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const bool smallInventory = m_sceneInterface->m_inventorySmall != 0;
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            smallInventory ? dispenser : inventory;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y,
                                             smallInventory ? 3 : 0, 0);
        return target;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, hotbar, state, x, y, 0, 0);
        return hotbar;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        m_sceneInterface->updateSlotPosition(section, dispenser, state, x, y, 3, 0);
        return dispenser;
    }
    if (section == dispenser && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_sceneInterface->m_inventorySmall ? hotbar : inventory;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y, -3, 0);
        return target;
    }
    if (section == dispenser && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        m_sceneInterface->updateSlotPosition(section, hotbar, state, x, y, -3, 0);
        return hotbar;
    }
    m_sceneInterface->updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_DispenserMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (position == 0)
        return;
    UIControl* control = static_cast<UIControl*>(0);
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
        control = &m_dispenserSlot;
    if (control != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_DispenserMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    UIControl* control = static_cast<UIControl*>(0);
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
        control = &m_dispenserSlot;
    if (control != 0 && position != 0 && size != 0 && columns != 0 && rows != 0)
    {
        position->x = static_cast<float>(slot % static_cast<int>(columns));
        position->y = static_cast<float>(slot / static_cast<int>(columns));
        size->x = static_cast<float>(control->getWidth() / columns);
        size->y = static_cast<float>(control->getHeight() / rows);
    }
}

int UIScene_DispenserMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
        return 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
        return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
        return 36;
    return 0;
}

void UIScene_DispenserMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (wrap)
    {
        if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
            m_hotbarList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
        else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
            m_inventoryList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
        else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
            m_dispenserSlot.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    }
}

unsigned int UIScene_DispenserMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_backgroundPanel.getFuiRenderNode();
    m_dispenserSlot.setupControl(this, panel, std::string("Trap"));
    m_controls.push_back(&m_dispenserSlot);
    m_dispenserLabel.setupControl(this, panel, std::string("dispenserLabel"));
    m_controls.push_back(&m_dispenserLabel);
    return 1;
}

std::wstring UIScene_DispenserMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"DispenserMenu") :
        std::wstring(L"DispenserMenuSplit");
}

void UIScene_DispenserMenu::handleReload()
{
    m_sceneInterface->Initialize(true, m_containerId, m_sceneInterface->m_menu, true,
                                m_slotIndex, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11),
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x14), false);
    m_dispenserSlot.addSlots(0, 9);
}

UIControl* UIScene_DispenserMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x11))
        return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x12))
        return &m_inventoryList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x13))
        return &m_dispenserSlot;
    return 0;
}

void UIScene_DispenserMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_DispenserMenu::getSceneType()
{
    return 0x0c;
}
