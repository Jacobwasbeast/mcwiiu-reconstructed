#include "game/ui/UIScene_HopperMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CMinecraftApp* g_pApp;

UIScene_HopperMenu::UIScene_HopperMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_hopperSlot(), m_hopperLabel(), m_sceneInterface(0)
{
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(this);
    initialiseMovie();
    m_hopperSlot.addSlots(0, 9);
}

UIScene_HopperMenu::~UIScene_HopperMenu()
{
}

unsigned int UIScene_HopperMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        return 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        return 5;
    return 0;
}

unsigned int UIScene_HopperMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        return GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        return 1;
    return 0;
}

unsigned int UIScene_HopperMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const IUIScene_AbstractContainerMenu::ESceneSection hotbar =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44);
    const IUIScene_AbstractContainerMenu::ESceneSection inventory =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45);
    const IUIScene_AbstractContainerMenu::ESceneSection hopper =
        static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46);

    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, hopper, state, x, y, 2, 0);
        return hopper;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const bool smallInventory = m_sceneInterface->m_inventorySmall != 0;
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            smallInventory ? hopper : inventory;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y,
                                              smallInventory ? 2 : 0, 0);
        return target;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, hotbar, state, x, y, 0, 0);
        return hotbar;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        m_sceneInterface->updateSlotPosition(section, hopper, state, x, y, 2, 0);
        return hopper;
    }
    if (section == hopper && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_sceneInterface->m_inventorySmall ? hotbar : inventory;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y, -2, 0);
        return target;
    }
    if (section == hopper && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        m_sceneInterface->updateSlotPosition(section, hotbar, state, x, y, -2, 0);
        return hotbar;
    }
    m_sceneInterface->updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_HopperMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        control = &m_hopperSlot;
    if (control != 0 && position != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_HopperMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        control = &m_hopperSlot;
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    if (control != 0 && position != 0 && size != 0 && columns != 0 && rows != 0)
    {
        position->x = static_cast<float>(slot % static_cast<int>(columns));
        position->y = static_cast<float>(slot / static_cast<int>(columns));
        size->x = static_cast<float>(control->getWidth() / columns);
        size->y = static_cast<float>(control->getHeight() / rows);
    }
}

void UIScene_HopperMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    UIControl_SlotList* list = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        list = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        list = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        list = &m_hopperSlot;
    if (wrap && list != 0)
        list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

unsigned int UIScene_HopperMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_backgroundPanel.getFuiRenderNode();
    m_hopperSlot.setupControl(this, panel, std::string("Trap"));
    m_controls.push_back(&m_hopperSlot);
    m_hopperLabel.setupControl(this, panel, std::string("dispenserLabel"));
    m_controls.push_back(&m_hopperLabel);
    return 1;
}

std::wstring UIScene_HopperMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"HopperMenu") :
        std::wstring(L"HopperMenuSplit");
}

void UIScene_HopperMenu::handleReload()
{
    m_sceneInterface->Initialize(true, m_containerId, m_sceneInterface->m_menu, true,
                                 m_slotIndex, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x47), false);
    m_hopperSlot.addSlots(0, 9);
}

int UIScene_HopperMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        return 0x128;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        return 0x188;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        return 0x35c;
    return 0;
}

void UIScene_HopperMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_HopperMenu::getSceneType()
{
    return 0x35;
}

int UIScene_HopperMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x44))
        return 32;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x45))
        return 5;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x46))
        return 0;
    return 0;
}
