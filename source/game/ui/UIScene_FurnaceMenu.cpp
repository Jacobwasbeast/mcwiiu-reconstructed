#include "game/ui/UIScene_FurnaceMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/FurnaceMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"

extern CConsoleMinecraftApp gApp;

struct FurnaceMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};

UIScene_FurnaceMenu::UIScene_FurnaceMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_container(),
      m_fuelSlot(), m_ingredientSlot(), m_resultSlot(), m_furnaceLabel(),
      m_ingredientLabel(), m_fuelLabel(), m_furnaceFire(), m_furnaceArrow()
{
    FurnaceMenuData* input = static_cast<FurnaceMenuData*>(data);
    initialiseMovie();

    m_container = input->container;
    m_furnaceLabel.init(UIString(m_container->getName()));
    if (m_container->getName().empty())
        m_furnaceLabel.init(UIString(L10N::GetString(IDS_TILE_FURNACE)));
    else
        m_furnaceLabel.init(UIString(m_container->getName()));

    m_ingredientLabel.init(UIString(L10N::GetString(IDS_INGREDIENT)));
    m_fuelLabel.init(UIString(L10N::GetString(IDS_FUEL)));
    m_furnaceFire.init(UIString(L""), 0, 0, 0, 0);
    m_furnaceArrow.init(UIString(L""), 0, 0, 0, 0);

    if (Minecraft::GetInstance()->mGameModes[input->player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[input->player]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_FurnaceMenu, this);
        }
    }

    FurnaceMenu* menu = new FurnaceMenu(input->inventory, input->container);
    Initialize(false, input->player, menu, true, 3,
                                 IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_FurnaceMax, false);
    m_fuelSlot.addSlots(1, 1);
    m_ingredientSlot.addSlots(0, 1);
    m_resultSlot.addSlots(2, 1);
    gApp.SetRichPresenceContext(m_playerId, 6);
    delete input;
}

UIScene_FurnaceMenu::~UIScene_FurnaceMenu()
{
}

unsigned int UIScene_FurnaceMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient:
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel:
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

unsigned int UIScene_FurnaceMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient:
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel:
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_FurnaceMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    if (section < IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar || section > IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult)
    {
        updateSlotPosition(section, section, state, x, y, 0, 0);
        return section;
    }
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        if (m_smallInventory == 0)
        {
            updateSlotPosition(section,
                IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory, state,
                x, y, 0, 0);
            return static_cast<unsigned int>(5);
        }
        if (*x < 6)
            target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel;
        else
            target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult;
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory && state == IUIScene_AbstractContainerMenu::ETapState_Down)
        target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
        target = *x < 6 ? IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient :
            IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory && state == IUIScene_AbstractContainerMenu::ETapState_Up)
        target = *x < 6 ? IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel : section;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient && state == IUIScene_AbstractContainerMenu::ETapState_Up)
        target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient && state == IUIScene_AbstractContainerMenu::ETapState_Down)
        target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel && state == IUIScene_AbstractContainerMenu::ETapState_Up)
        target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel && state == IUIScene_AbstractContainerMenu::ETapState_Down)
        target = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar : IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult && state == IUIScene_AbstractContainerMenu::ETapState_Up)
        target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult && state == IUIScene_AbstractContainerMenu::ETapState_Down)
        target = m_smallInventory ? IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar : IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory;
    else if ((state == IUIScene_AbstractContainerMenu::ETapState_Left || state == IUIScene_AbstractContainerMenu::ETapState_Right) && (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult))
        target = IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient;
    if (target == section && state != IUIScene_AbstractContainerMenu::ETapState_Left && state != IUIScene_AbstractContainerMenu::ETapState_Right)
        updateSlotPosition(section, section, state, x, y, 0, 0);
    else
    {
        int horizontalOffset = 0;
        if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient || section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel) horizontalOffset = -3;
        else if (section == IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult) horizontalOffset = -7;
        updateSlotPosition(section, target, state, x, y, horizontalOffset, 0);
    }
    return static_cast<unsigned int>(target);
}

void UIScene_FurnaceMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult:
        position->x = static_cast<float>(m_resultSlot.getXPos());
        position->y = static_cast<float>(m_resultSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel:
        position->x = static_cast<float>(m_fuelSlot.getXPos());
        position->y = static_cast<float>(m_fuelSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient:
        position->x = static_cast<float>(m_ingredientSlot.getXPos());
        position->y = static_cast<float>(m_ingredientSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    default:
        break;
    }
}

void UIScene_FurnaceMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D controlSize;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult:
        controlSize.x = m_resultSlot.getWidth();
        controlSize.y = m_resultSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel:
        controlSize.x = m_fuelSlot.getWidth();
        controlSize.y = m_fuelSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient:
        controlSize.x = m_ingredientSlot.getWidth();
        controlSize.y = m_ingredientSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory:
        controlSize.x = m_inventoryList.getWidth();
        controlSize.y = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar:
        controlSize.x = m_hotbarList.getWidth();
        controlSize.y = m_hotbarList.getHeight();
        break;
    default:
        break;
    }
    const int rows = getSectionRows(section);
    const int columns = getSectionColumns(section);
    size->x = controlSize.x / static_cast<float>(columns);
    size->y = controlSize.y / static_cast<float>(rows);
    position->x = static_cast<float>(slot % columns) * size->x;
    position->y = static_cast<float>(slot / columns) * size->y;
}

int UIScene_FurnaceMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar: offset = 30; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory: offset = 3; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient: offset = 0; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel: offset = 1; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult: offset = 2; break;
    default: break;
    }
    return offset;
}

void UIScene_FurnaceMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    const int highlightSlot = direction * getSectionColumns(section) + slot;
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult: list = &m_resultSlot; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel: list = &m_fuelSlot; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient: list = &m_ingredientSlot; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory: list = &m_inventoryList; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar: list = &m_hotbarList; break;
    default: break;
    }
    if (wrap)
        list->setHighlightSlot(highlightSlot);
}

unsigned int UIScene_FurnaceMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_ingredientSlot.setupControl(this, panel, std::string("Ingredient"));
        m_controls.push_back(&m_ingredientSlot);
    }
    {
        m_fuelSlot.setupControl(this, panel, std::string("Fuel"));
        m_controls.push_back(&m_fuelSlot);
    }
    {
        m_resultSlot.setupControl(this, panel, std::string("Result"));
        m_controls.push_back(&m_resultSlot);
    }
    {
        m_furnaceLabel.setupControl(this, panel, std::string("Furnace_text"));
        m_controls.push_back(&m_furnaceLabel);
    }
    {
        m_ingredientLabel.setupControl(this, panel, std::string("Ingredient_Label"));
        m_controls.push_back(&m_ingredientLabel);
    }
    {
        m_fuelLabel.setupControl(this, panel, std::string("Fuel_Label"));
        m_controls.push_back(&m_fuelLabel);
    }
    {
        m_furnaceFire.setupControl(this, panel, std::string("FurnaceFire"));
        m_controls.push_back(&m_furnaceFire);
    }
    {
        m_furnaceArrow.setupControl(this, panel, std::string("FurnaceArrow"));
        m_controls.push_back(&m_furnaceArrow);
    }
    return 1;
}

std::wstring UIScene_FurnaceMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"FurnaceMenuSplit") :
        std::wstring(L"FurnaceMenu");
}

void UIScene_FurnaceMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 3,
                                 IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_FurnaceMax, false);
    m_fuelSlot.addSlots(1, 1);
    m_ingredientSlot.addSlots(0, 1);
    m_resultSlot.addSlots(2, 1);
}

void UIScene_FurnaceMenu::tick()
{
    if (m_furnaceFire.getMax() != m_container->getContainerData(1))
    {
        m_furnaceFire.setMax(m_container->getContainerData(1));
        m_furnaceFire.ReInit();
    }
    if (m_furnaceArrow.getMax() != m_container->getContainerData(3))
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
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceResult: control = &m_resultSlot; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceFuel: control = &m_fuelSlot; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceIngredient: control = &m_ingredientSlot; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceInventory: control = &m_inventoryList; break;
    case IUIScene_AbstractContainerMenu::ESceneSection_FurnaceHotbar: control = &m_hotbarList; break;
    default: break;
    }
    return control;
}

void UIScene_FurnaceMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_FurnaceMenu::getSceneType()
{
    return EUIScene_FurnaceMenu;
}
