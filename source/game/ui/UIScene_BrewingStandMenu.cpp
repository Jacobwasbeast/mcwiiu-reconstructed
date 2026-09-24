#include "game/ui/UIScene_BrewingStandMenu.hpp"

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

extern CMinecraftApp* g_pApp;

static const unsigned char g_brewingBubbleProgress[7] = { 0, 0x1e, 0x19, 0x14, 0x0f, 0x0a, 0x05 };

struct UIScene_BrewingStandMenuData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int playerIndex;
    bool splitscreen;
};

UIScene_BrewingStandMenu::UIScene_BrewingStandMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_sceneInterface(0), m_container(),
      m_bottleSlots(), m_ingredientSlot(), m_brewingPlate(), m_brewingStandLabel(),
      m_brewingArrow(), m_brewingBubbles(), m_brewingPlateBar()
{
    UIScene_BrewingStandMenuData* sceneData = static_cast<UIScene_BrewingStandMenuData*>(data);
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(
        static_cast<UIScene_AbstractContainerMenu*>(this));
    initialiseMovie();
    m_brewingArrow.init(UIString(L""), 0, 0, 400, 0);
    m_brewingBubbles.init(UIString(L""), 0, 0, 30, 0);
    m_brewingPlateBar.init(UIString(L""), 0, 0, 20, 0);

    m_container = sceneData->container;
    if (m_container->hasCustomName())
        m_brewingStandLabel.init(UIString(m_container->getCustomName()));
    else
        m_brewingStandLabel.init(UIString(L10N::GetString(0x20259a79)));

    if (Minecraft::GetInstance()->mGameModes[sceneData->playerIndex] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[sceneData->playerIndex]->getTutorial();
        if (tutorial != 0)
        {
            m_sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(static_cast<eTutorial_State>(0x12), this);
        }
    }

    BrewingStandMenu* menu = new BrewingStandMenu(sceneData->inventory, sceneData->container);
    m_sceneInterface->Initialize(false, sceneData->playerIndex, menu, true, 5,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x30), false);
    m_bottleSlots[0].addSlots(0, 1);
    m_bottleSlots[1].addSlots(1, 1);
    m_bottleSlots[2].addSlots(2, 1);
    m_ingredientSlot.addSlots(3, 1);
    m_brewingPlate.addSlots(4, 1);

    const int playerIndex = sceneData->playerIndex;
    delete sceneData;
    g_pApp->SetRichPresenceContext(playerIndex, 0xb);
}

UIScene_BrewingStandMenu::~UIScene_BrewingStandMenu()
{
}

unsigned int UIScene_BrewingStandMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f))
        return 0;
    static const unsigned char columns[7] = { 9, 9, 1, 1, 1, 1, 1 };
    return columns[static_cast<int>(section) - 0x29];
}

unsigned int UIScene_BrewingStandMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section < static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29) ||
        section > static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f))
        return 0;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a))
        return m_sceneInterface->GetInventoryRows();
    return 1;
}

unsigned int UIScene_BrewingStandMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection target = section;
    int dx = 0;
    switch (static_cast<int>(section))
    {
    case 0x29:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            if (*x > 1)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2e);
            else
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f);
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (m_sceneInterface->m_inventorySmall == 0)
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a);
            else if (*x > 1)
                target = *x < 4 ? static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2b) :
                         *x < 5 ? static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2c) :
                                  static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2d);
            else
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f);
        }
        break;
    case 0x2a:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            if (*x > 1)
                target = *x < 4 ? static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2b) :
                         *x < 5 ? static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2c) :
                                  static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2d);
            else
                target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f);
        }
        break;
    case 0x2b:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2e);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_sceneInterface->m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a);
            dx = -3;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2d);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2c);
        break;
    case 0x2c:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2e);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_sceneInterface->m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a);
            dx = -4;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2b);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2d);
        break;
    case 0x2d:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2e);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            target = m_sceneInterface->m_inventorySmall != 0 ?
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29) :
                static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a);
            dx = -5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2c);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2b);
        break;
    case 0x2e:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29);
            dx = 5;
        }
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2c);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f);
        break;
    case 0x2f:
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a);
        else if (state == IUIScene_AbstractContainerMenu::ETapState_Left ||
                 state == IUIScene_AbstractContainerMenu::ETapState_Right)
            target = static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2e);
        break;
    default:
        break;
    }
    m_sceneInterface->updateSlotPosition(section, target, state, x, y, dx, 0);
    return target;
}

void UIScene_BrewingStandMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    if (position == 0) return;
    UIControl* control = 0;
    if (section == 0x29) control = static_cast<UIControl*>(&m_hotbarList);
    else if (section == 0x2a) control = static_cast<UIControl*>(&m_inventoryList);
    else if (section >= 0x2b && section <= 0x2d) control = static_cast<UIControl*>(&m_bottleSlots[section - 0x2b]);
    else if (section == 0x2e) control = static_cast<UIControl*>(&m_ingredientSlot);
    else if (section == 0x2f) control = static_cast<UIControl*>(&m_brewingPlate);
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
    if (section == 0x29) control = static_cast<UIControl*>(&m_hotbarList);
    else if (section == 0x2a) control = static_cast<UIControl*>(&m_inventoryList);
    else if (section >= 0x2b && section <= 0x2d) control = static_cast<UIControl*>(&m_bottleSlots[section - 0x2b]);
    else if (section == 0x2e) control = static_cast<UIControl*>(&m_ingredientSlot);
    else if (section == 0x2f) control = static_cast<UIControl*>(&m_brewingPlate);
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

void UIScene_BrewingStandMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

void UIScene_BrewingStandMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (!wrap) return;
    const int columns = static_cast<int>(getSectionColumns(section));
    switch (static_cast<int>(section))
    {
    case 0x29: m_hotbarList.setHighlightSlot(direction * columns + slot); break;
    case 0x2a: m_inventoryList.setHighlightSlot(direction * columns + slot); break;
    case 0x2b: m_bottleSlots[0].setHighlightSlot(direction * columns + slot); break;
    case 0x2c: m_bottleSlots[1].setHighlightSlot(direction * columns + slot); break;
    case 0x2d: m_bottleSlots[2].setHighlightSlot(direction * columns + slot); break;
    case 0x2e: m_ingredientSlot.setHighlightSlot(direction * columns + slot); break;
    case 0x2f: m_brewingPlate.setHighlightSlot(direction * columns + slot); break;
    default: break;
    }
}

int UIScene_BrewingStandMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == 0x29) return 0;
    if (section == 0x2a) return 1;
    if (section == 0x2b) return 2;
    if (section == 0x2c) return 3;
    if (section == 0x2d) return 4;
    if (section == 0x2e) return 5;
    return 0;
}

unsigned int UIScene_BrewingStandMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    m_bottleSlots[0].setupControl(this, panel, std::string("Bottle1"));
    m_bottleSlots[1].setupControl(this, panel, std::string("Bottle2"));
    m_bottleSlots[2].setupControl(this, panel, std::string("Bottle3"));
    m_ingredientSlot.setupControl(this, panel, std::string("Ingredient"));
    m_brewingPlate.setupControl(this, panel, std::string("BrewingPlate"));
    m_brewingStandLabel.setupControl(this, panel, std::string("BrewingStandText"));
    m_brewingArrow.setupControl(this, panel, std::string("BrewingArrow"));
    m_brewingBubbles.setupControl(this, panel, std::string("BrewingBubbles"));
    m_brewingPlateBar.setupControl(this, panel, std::string("BrewingPlateBar"));
    m_controls.push_back(&m_bottleSlots[0]);
    m_controls.push_back(&m_bottleSlots[1]);
    m_controls.push_back(&m_bottleSlots[2]);
    m_controls.push_back(&m_ingredientSlot);
    m_controls.push_back(&m_brewingPlate);
    m_controls.push_back(&m_brewingStandLabel);
    m_controls.push_back(&m_brewingArrow);
    m_controls.push_back(&m_brewingBubbles);
    m_controls.push_back(&m_brewingPlateBar);
    return 1;
}

std::wstring UIScene_BrewingStandMenu::getMoviePath()
{
    return g_pApp->GetLocalPlayerCount() < 2 ? std::wstring(L"BrewingStandMenu") :
        std::wstring(L"BrewingStandMenuSplit");
}

void UIScene_BrewingStandMenu::handleReload()
{
    m_sceneInterface->Initialize(true, m_playerId, m_menu, true, 5,
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29),
                                 static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x30), false);
    m_ingredientSlot.addSlots(3, 1);
    m_brewingPlate.addSlots(4, 1);
    m_bottleSlots[0].addSlots(0, 1);
    m_bottleSlots[1].addSlots(1, 1);
    m_bottleSlots[2].addSlots(2, 1);
    m_brewingPlateBar.setProgress(0);
}

void UIScene_BrewingStandMenu::tick()
{
    const int brewTime = m_container == 0 ? 0 : m_container->getContainerData(0);
    const int fuel = m_container == 0 ? 0 : m_container->getContainerData(1);
    m_brewingArrow.setProgress(brewTime);
    m_brewingPlateBar.setProgress(fuel);
    const int bubbles = brewTime / 2 % 7;
    m_brewingBubbles.setProgress(bubbles == 0 ? 0 : bubbles < 7 ? g_brewingBubbleProgress[bubbles] : 0);
    UIScene_AbstractContainerMenu::tick();
}

UIControl* UIScene_BrewingStandMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x29))
        return &m_hotbarList;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2a))
        return &m_inventoryList;
    if (section >= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2b) &&
        section <= static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2d))
        return &m_bottleSlots[static_cast<int>(section) - 0x2b];
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2e))
        return &m_ingredientSlot;
    if (section == static_cast<IUIScene_AbstractContainerMenu::ESceneSection>(0x2f))
        return &m_brewingPlate;
    return 0;
}

int UIScene_BrewingStandMenu::getSceneType()
{
    return 0x2f;
}
