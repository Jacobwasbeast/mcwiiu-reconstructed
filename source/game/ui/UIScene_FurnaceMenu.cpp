#include "game/ui/UIScene_FurnaceMenu.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/ui/FurnaceMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"

extern CMinecraftApp* g_pApp;

struct FurnaceMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int player;
};

UIScene_FurnaceMenu::UIScene_FurnaceMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_sceneInterface(0), m_container(),
      m_fuelSlot(), m_ingredientSlot(), m_resultSlot(), m_furnaceLabel(),
      m_ingredientLabel(), m_fuelLabel(), m_furnaceFire(), m_furnaceArrow()
{
    FurnaceMenuData* input = static_cast<FurnaceMenuData*>(data);
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(this);
    initialiseMovie();
    if (input != 0)
    {
        m_container = input->container;
        FurnaceMenu* menu = new FurnaceMenu(input->inventory, input->container);
        m_sceneInterface->Initialize(false, input->player, menu, true, 3,
                                     static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(4),
                                     static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(9), false);
        if (m_container.get() != 0)
        {
            const std::wstring name = m_container->getName();
            m_furnaceLabel.init(UIString(name));
            if (name == L10N::GetString(0x8015994f))
                m_furnaceLabel.init(UIString(L10N::GetString(0x8015994f)));
        }
    }
    m_ingredientLabel.init(UIString(L10N::GetString(0x7d7816a1)));
    m_fuelLabel.init(UIString(L10N::GetString(0x9f31c0ee)));
    m_furnaceFire.init(UIString(L""), 0, 0, 0, 0);
    m_furnaceArrow.init(UIString(L""), 0, 0, 0, 0);
    m_fuelSlot.addSlots(1, 1);
    m_ingredientSlot.addSlots(0, 1);
    m_resultSlot.addSlots(2, 1);
}

UIScene_FurnaceMenu::~UIScene_FurnaceMenu()
{
}

unsigned int UIScene_FurnaceMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    static const unsigned char columns[] = { 0, 0, 0, 0, 1, 1, 1, 1, 1 };
    const int value = static_cast<int>(section);
    if (value < 4 || value > 8)
        return 0;
    return columns[value];
}

unsigned int UIScene_FurnaceMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const int value = static_cast<int>(section);
    if (value < 4 || value > 8)
        return 0;
    if (value == 5)
        return m_sceneInterface->GetInventoryRows();
    return 1;
}

unsigned int UIScene_FurnaceMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const int value = static_cast<int>(section);
    const int direction = static_cast<int>(state);
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    if (value < 4 || value > 8)
    {
        m_sceneInterface->updateSlotPosition(section, section, state, x, y, 0, 0);
        return section;
    }
    if (value == 4 && direction == 1)
    {
        if (m_sceneInterface->m_inventorySmall == 0)
        {
            m_sceneInterface->updateSlotPosition(section,
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(5), state,
                x, y, 0, 0);
            return static_cast<unsigned int>(5);
        }
        if (*x < 6)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(7);
        else
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(8);
    }
    else if (value == 5 && direction == 2)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(4);
    else if (value == 4 && direction == 2)
        target = *x < 6 ? static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(6) :
            static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(8);
    else if (value == 5 && direction == 1)
        target = *x < 6 ? static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(7) : section;
    else if (value == 6 && direction == 1)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(4);
    else if (value == 6 && direction == 2)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(7);
    else if (value == 7 && direction == 1)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(4);
    else if (value == 7 && direction == 2)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(m_sceneInterface->m_inventorySmall ? 4 : 5);
    else if (value == 8 && direction == 1)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(4);
    else if (value == 8 && direction == 2)
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(m_sceneInterface->m_inventorySmall ? 4 : 5);
    else if ((direction == 3 || direction == 4) && (value == 8))
        target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(6);
    if (target == section && direction != 3 && direction != 4)
        m_sceneInterface->updateSlotPosition(section, section, state, x, y, 0, 0);
    else
    {
        int horizontalOffset = 0;
        if (value == 6 || value == 7) horizontalOffset = -3;
        else if (value == 8) horizontalOffset = -7;
        m_sceneInterface->updateSlotPosition(section, target, state, x, y, horizontalOffset, 0);
    }
    return static_cast<unsigned int>(target);
}

void UIScene_FurnaceMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (position == 0)
        return;
    UIControl* control = static_cast<UIControl*>(0);
    const int value = static_cast<int>(section);
    if (value == 4) control = &m_hotbarList;
    else if (value == 5) control = &m_inventoryList;
    else if (value == 6) control = &m_ingredientSlot;
    else if (value == 7) control = &m_fuelSlot;
    else if (value == 8) control = &m_resultSlot;
    if (control != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_FurnaceMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = static_cast<UIControl*>(0);
    const int value = static_cast<int>(section);
    if (value == 4) control = &m_hotbarList;
    else if (value == 5) control = &m_inventoryList;
    else if (value == 6) control = &m_ingredientSlot;
    else if (value == 7) control = &m_fuelSlot;
    else if (value == 8) control = &m_resultSlot;
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

int UIScene_FurnaceMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const int value = static_cast<int>(section);
    if (value == 4) return 30;
    if (value == 5) return 3;
    if (value == 6) return 0;
    if (value == 7) return 1;
    if (value == 8) return 2;
    return 0;
}

void UIScene_FurnaceMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap)
        return;
    UIControl_SlotList* list = static_cast<UIControl_SlotList*>(0);
    const int value = static_cast<int>(section);
    if (value == 4) list = &m_hotbarList;
    else if (value == 5) list = &m_inventoryList;
    else if (value == 6) list = &m_ingredientSlot;
    else if (value == 7) list = &m_fuelSlot;
    else if (value == 8) list = &m_resultSlot;
    if (list != 0)
        list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

unsigned int UIScene_FurnaceMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_ingredientSlot.setupControl(this, panel, std::string("Ingredient"));
    m_controls.push_back(&m_ingredientSlot);
    m_fuelSlot.setupControl(this, panel, std::string("Fuel"));
    m_controls.push_back(&m_fuelSlot);
    m_resultSlot.setupControl(this, panel, std::string("Result"));
    m_controls.push_back(&m_resultSlot);
    m_furnaceLabel.setupControl(this, panel, std::string("Furnace_text"));
    m_controls.push_back(&m_furnaceLabel);
    m_ingredientLabel.setupControl(this, panel, std::string("Ingredient_Label"));
    m_controls.push_back(&m_ingredientLabel);
    m_fuelLabel.setupControl(this, panel, std::string("Fuel_Label"));
    m_controls.push_back(&m_fuelLabel);
    m_furnaceFire.setupControl(this, panel, std::string("FurnaceFire"));
    m_controls.push_back(&m_furnaceFire);
    m_furnaceArrow.setupControl(this, panel, std::string("FurnaceArrow"));
    m_controls.push_back(&m_furnaceArrow);
    return 1;
}

std::wstring UIScene_FurnaceMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"FurnaceMenu") :
        std::wstring(L"FurnaceMenuSplit");
}

void UIScene_FurnaceMenu::handleReload()
{
    m_sceneInterface->Initialize(true, m_containerId, m_sceneInterface->m_menu, true, 3,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(4),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(9), false);
    m_fuelSlot.addSlots(1, 1);
    m_ingredientSlot.addSlots(0, 1);
    m_resultSlot.addSlots(2, 1);
}

void UIScene_FurnaceMenu::tick()
{
    int value = m_container->getContainerData(1);
    if (m_furnaceFire.getMax() != static_cast<unsigned int>(value))
    {
        m_furnaceFire.setMax(m_container->getContainerData(1));
        m_furnaceFire.ReInit();
    }
    value = m_container->getContainerData(3);
    if (m_furnaceArrow.getMax() != static_cast<unsigned int>(value))
    {
        m_furnaceArrow.setMax(m_container->getContainerData(3));
        m_furnaceArrow.ReInit();
    }
    m_furnaceFire.setProgress(m_container->getContainerData(0));
    m_furnaceArrow.setProgress(m_container->getContainerData(2));
    UIScene_AbstractContainerMenu::tick();
}

UIControl* UIScene_FurnaceMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    const int value = static_cast<int>(section);
    if (value == 4) return &m_hotbarList;
    if (value == 5) return &m_inventoryList;
    if (value == 6) return &m_ingredientSlot;
    if (value == 7) return &m_fuelSlot;
    if (value == 8) return &m_resultSlot;
    return 0;
}

void UIScene_FurnaceMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_FurnaceMenu::getSceneType()
{
    return 8;
}
