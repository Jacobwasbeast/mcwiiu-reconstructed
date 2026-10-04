#include "game/ui/UIScene_BrewingStandMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Container.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/BrewingStandMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"

extern CConsoleMinecraftApp gApp;

static const unsigned char g_brewingBubbleProgress[7] = { 0, 0x1e, 0x19, 0x14, 0x0f, 0x0a, 0x05 };

struct UIScene_BrewingStandMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int playerIndex;
    bool splitscreen;
};

UIScene_BrewingStandMenu::UIScene_BrewingStandMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_container(),
      m_bottleSlots(), m_ingredientSlot(), m_brewingPlate(), m_brewingStandLabel(),
      m_brewingArrow(), m_brewingBubbles(), m_brewingPlateBar()
{
    UIScene_BrewingStandMenuData* sceneData = static_cast<UIScene_BrewingStandMenuData*>(data);
    initialiseMovie();
    m_brewingArrow.init(UIString(L""), 0, 0, 400, 0);
    m_brewingBubbles.init(UIString(L""), 0, 0, 30, 0);
    m_brewingPlateBar.init(UIString(L""), 0, 0, 20, 0);

    m_container = sceneData->container;
    if (m_container->getName().empty())
        m_brewingStandLabel.init(UIString(L10N::GetString(IDS_TILE_BREWINGSTAND)));
    else
        m_brewingStandLabel.init(UIString(m_container->getName()));

    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[type]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_BrewingMenu, this);
        }
    }

    BrewingStandMenu* menu = new BrewingStandMenu(sceneData->inventory, sceneData->container);
    Initialize(false, sceneData->playerIndex, menu, true, 5,
                                 IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_BrewingMax, false);
    m_ingredientSlot.addSlots(3, 1);
    m_brewingPlate.addSlots(4, 1);
    m_bottleSlots[0].addSlots(0, 1);
    m_bottleSlots[1].addSlots(1, 1);
    m_bottleSlots[2].addSlots(2, 1);

    delete sceneData;
    gApp.SetRichPresenceContext(type, 11);
}

UIScene_BrewingStandMenu::~UIScene_BrewingStandMenu()
{
}

unsigned int UIScene_BrewingStandMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        columns = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        columns = 1;
        break;
    default:
        break;
    }
    return columns;
}

unsigned int UIScene_BrewingStandMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        rows = GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        rows = 1;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_BrewingStandMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x > 1)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_smallInventory == 0)
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            else if (*x > 1)
                target = *x < 4 ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1 :
                         *x < 5 ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2 :
                                  IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x > 1)
                target = *x < 4 ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1 :
                         *x < 5 ? IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2 :
                                  IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
            else
                target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
        }
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            dx = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_smallInventory != 0 ?
                IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
            dx = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar;
            dx = 5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory;
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient;
        break;
    default:
        break;
    }
    updateSlotPosition(section, target, state, x, y, dx, 0);
    return target;
}

void UIScene_BrewingStandMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (position == 0) return;
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar) control = static_cast<UIControl*>(&m_hotbarList);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory) control = static_cast<UIControl*>(&m_inventoryList);
    else if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1 && section <= IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3) control = static_cast<UIControl*>(&m_bottleSlots[section - IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1]);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient) control = static_cast<UIControl*>(&m_ingredientSlot);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel) control = static_cast<UIControl*>(&m_brewingPlate);
    if (control != 0)
    {
        position->x = static_cast<float>(control->getXPos());
        position->y = static_cast<float>(control->getYPos());
    }
}

void UIScene_BrewingStandMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar) control = static_cast<UIControl*>(&m_hotbarList);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory) control = static_cast<UIControl*>(&m_inventoryList);
    else if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1 && section <= IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3) control = static_cast<UIControl*>(&m_bottleSlots[section - IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1]);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient) control = static_cast<UIControl*>(&m_ingredientSlot);
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel) control = static_cast<UIControl*>(&m_brewingPlate);
    const unsigned int rows = getSectionRows(section);
    const unsigned int columns = getSectionColumns(section);
    if (control != 0 && position != 0 && size != 0 && columns != 0 && rows != 0)
    {
        size->x = static_cast<float>(control->getWidth() / static_cast<float>(columns));
        size->y = static_cast<float>(control->getHeight() / static_cast<float>(rows));
        position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
        position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
    }
}

void UIScene_BrewingStandMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

void UIScene_BrewingStandMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    const int columns = static_cast<int>(getSectionColumns(section));
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar: m_hotbarList.setHighlightSlot(direction * columns + slot); break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory: m_inventoryList.setHighlightSlot(direction * columns + slot); break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1: m_bottleSlots[0].setHighlightSlot(direction * columns + slot); break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2: m_bottleSlots[1].setHighlightSlot(direction * columns + slot); break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3: m_bottleSlots[2].setHighlightSlot(direction * columns + slot); break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient: m_ingredientSlot.setHighlightSlot(direction * columns + slot); break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel: m_brewingPlate.setHighlightSlot(direction * columns + slot); break;
    default: break;
    }
}

int UIScene_BrewingStandMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar:
        offset = 32;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory:
        offset = 5;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle2:
        offset = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3:
        offset = 2;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient:
        offset = 3;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel:
        offset = 4;
        break;
    default:
        break;
    }
    return offset;
}

unsigned int UIScene_BrewingStandMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        m_bottleSlots[0].setupControl(this, panel, std::string("Bottle1"));
        m_controls.push_back(&m_bottleSlots[0]);
    }
    {
        m_bottleSlots[1].setupControl(this, panel, std::string("Bottle2"));
        m_controls.push_back(&m_bottleSlots[1]);
    }
    {
        m_bottleSlots[2].setupControl(this, panel, std::string("Bottle3"));
        m_controls.push_back(&m_bottleSlots[2]);
    }
    {
        m_ingredientSlot.setupControl(this, panel, std::string("Ingredient"));
        m_controls.push_back(&m_ingredientSlot);
    }
    {
        m_brewingPlate.setupControl(this, panel, std::string("BrewingPlate"));
        m_controls.push_back(&m_brewingPlate);
    }
    {
        m_brewingStandLabel.setupControl(this, panel, std::string("BrewingStandText"));
        m_controls.push_back(&m_brewingStandLabel);
    }
    {
        m_brewingArrow.setupControl(this, panel, std::string("BrewingArrow"));
        m_controls.push_back(&m_brewingArrow);
    }
    {
        m_brewingBubbles.setupControl(this, panel, std::string("BrewingBubbles"));
        m_controls.push_back(&m_brewingBubbles);
    }
    {
        m_brewingPlateBar.setupControl(this, panel, std::string("BrewingPlateBar"));
        m_controls.push_back(&m_brewingPlateBar);
    }
    return 1;
}

std::wstring UIScene_BrewingStandMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"BrewingStandMenuSplit") :
        std::wstring(L"BrewingStandMenu");
}

void UIScene_BrewingStandMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true, 5,
                                 IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_BrewingMax, false);
    m_ingredientSlot.addSlots(3, 1);
    m_brewingPlate.addSlots(4, 1);
    m_bottleSlots[0].addSlots(0, 1);
    m_bottleSlots[1].addSlots(1, 1);
    m_bottleSlots[2].addSlots(2, 1);
    m_brewingPlateBar.setProgress(0);
}

void UIScene_BrewingStandMenu::tick()
{
    m_brewingArrow.setProgress(m_container->getContainerData(0));
    m_brewingPlateBar.setProgress(m_container->getContainerData(1));
    const unsigned int bubbles = m_container->getContainerData(0) / 2 % 7;
    if (bubbles == 0)
        m_brewingBubbles.setProgress(0);
    else if (bubbles > 6)
        m_brewingBubbles.setProgress(0);
    else
        m_brewingBubbles.setProgress(g_brewingBubbleProgress[bubbles]);
    UIScene_AbstractContainerMenu::tick();
}

UIControl* UIScene_BrewingStandMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingHotbar)
        return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingInventory)
        return &m_inventoryList;
    if (section >= IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1 &&
        section <= IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle3)
        return &m_bottleSlots[static_cast<int>(section) - IUIScene_AbstractContainerMenu::ESceneSection_BrewingBottle1];
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingIngredient)
        return &m_ingredientSlot;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_BrewingFuel)
        return &m_brewingPlate;
    return 0;
}

int UIScene_BrewingStandMenu::getSceneType()
{
    return EUIScene_BrewingStandMenu;
}
