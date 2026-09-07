#include "game/ui/UIScene_ContainerMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

UIScene_ContainerMenu::UIScene_ContainerMenu(int type, void*, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer),
      m_largeChest(false),
      m_containerList(),
      m_chestLabel(),
      m_sceneInterface(0)
{
    m_sceneInterface = this;
    initialiseMovie();
    m_chestLabel.init(UIString(L10N::GetString(0x24fee045)));
}

UIScene_ContainerMenu::~UIScene_ContainerMenu()
{
}

unsigned int UIScene_ContainerMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2) ? 9U : 0U;
}

unsigned int UIScene_ContainerMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        return 1;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        return GetInventoryRows();
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        return static_cast<unsigned int>((GetBaseSlotCount() - 36) / 9);
    return 0;
}

unsigned int UIScene_ContainerMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection result = section;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2),
                               state, x, y, 0, 0);
            return 2;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            result = m_inventorySmall ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1);
            updateSlotPosition(section, result, state, x, y, 0, 0);
            return result;
        }
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                               state, x, y, 0, 0);
            return 0;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2),
                               state, x, y, 0, 0);
            return 2;
        }
    }
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            result = m_inventorySmall ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1);
            updateSlotPosition(section, result, state, x, y, 0, 0);
            return result;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            updateSlotPosition(section, static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                               state, x, y, 0, 0);
            return 0;
        }
    }
    updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_ContainerMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        control = &m_containerList;
    if (control != 0 && position != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_ContainerMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        control = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        control = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        control = &m_containerList;
    if (control != 0 && position != 0 && size != 0)
    {
        const unsigned int columns = getSectionColumns(section);
        const unsigned int rows = getSectionRows(section);
        position->x = static_cast<float>(slot % static_cast<int>(columns));
        position->y = static_cast<float>(slot / static_cast<int>(columns));
        size->x = static_cast<float>(control->getWidth() / columns);
        size->y = static_cast<float>(control->getHeight() / rows);
    }
}

void UIScene_ContainerMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    UIControl_SlotList* list = 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        list = &m_hotbarList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        list = &m_inventoryList;
    else if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        list = &m_containerList;
    if (wrap && list != 0)
        list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

int UIScene_ContainerMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        return GetBaseSlotCount() - 9;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        return GetBaseSlotCount() - 36;
    return 0;
}

void UIScene_ContainerMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

unsigned int UIScene_ContainerMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* root = m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_containerList.setupControl(this, panel, std::string("containerList"));
    m_controls.push_back(&m_containerList);
    m_chestLabel.setupControl(this, panel, std::string("chestLabel"));
    m_controls.push_back(&m_chestLabel);
    (void)root;
    return 1;
}

std::wstring UIScene_ContainerMenu::getMoviePath()
{
    if (m_largeChest)
        return g_pApp->GetLocalPlayerCount() < 2 ?
            std::wstring(L"ChestLargeMenu") : std::wstring(L"ChestLargeMenuSplit");
    return g_pApp->GetLocalPlayerCount() < 2 ?
        std::wstring(L"ChestMenu") : std::wstring(L"ChestMenuSplit");
}

void UIScene_ContainerMenu::handleReload()
{
    const int count = GetBaseSlotCount();
    m_sceneInterface->Initialize(true, m_containerId, m_menu, true, count - 36,
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0),
                                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(3), false);
    m_containerList.addSlots(0, count - 36);
}

UIControl* UIScene_ContainerMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0))
        return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(1))
        return &m_inventoryList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(2))
        return &m_containerList;
    return 0;
}

int UIScene_ContainerMenu::getSceneType()
{
    return 9;
}
