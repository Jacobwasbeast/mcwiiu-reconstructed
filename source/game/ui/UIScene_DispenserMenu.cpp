#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/ui/UIScene_DispenserMenu.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/item/Container.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/TrapMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/ui/UILayer.hpp"


namespace
{
struct DispenserMenuArgs
{
    boost::shared_ptr<Container> inventory;
    boost::shared_ptr<Container> container;
    int player;
    bool splitscreen;
};
}

UIScene_DispenserMenu::UIScene_DispenserMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_dispenserSlot(), m_dispenserLabel()
{
    initialiseMovie();

    DispenserMenuArgs* args = static_cast<DispenserMenuArgs*>(data);
    if (args->container->getName().empty())
    {
        boost::shared_ptr<InteractionObject> interaction =
            boost::dynamic_pointer_cast<InteractionObject>(args->container);
        m_dispenserLabel.init(UIString(L10N::GetString(
            interaction && interaction->getTypeId(false) == ClientboundContainerOpenPacket::DROPPER ? IDS_CONTAINER_DROPPER : IDS_DISPENSER)));
    }
    else
    {
        m_dispenserLabel.init(UIString(args->container->getName()));
    }

    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[args->player] != 0)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[type]->getTutorial();
        if (tutorial != 0)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_DispenserMenu, this);
        }
    }
    AbstractContainerMenu* menu = new TrapMenu(args->inventory, args->container);
    m_containerSize = static_cast<int>(args->container->getContainerSize());
    Initialize(false, args->player, menu, true,
                                m_containerSize, IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar,
                                IUIScene_AbstractContainerMenu::ESceneSection_DispenserMax, false);
    m_dispenserSlot.addSlots(0, 9);
    delete args;
}

UIScene_DispenserMenu::~UIScene_DispenserMenu()
{
}

unsigned int UIScene_DispenserMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int columns = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        columns = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        columns = 3;
        break;
    default:
        break;
    }
    return columns;
}

unsigned int UIScene_DispenserMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int rows = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        rows = 1;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        rows = IUIScene_AbstractContainerMenu::GetInventoryRows();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        rows = 3;
        break;
    default:
        break;
    }
    return rows;
}

unsigned int UIScene_DispenserMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const IUIScene_AbstractContainerMenu::ESceneSection hotbar =
        IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar;
    const IUIScene_AbstractContainerMenu::ESceneSection inventory =
        IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory;
    const IUIScene_AbstractContainerMenu::ESceneSection dispenser =
        IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots;

    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        updateSlotPosition(section, dispenser, state, x, y, 3, 0);
        return dispenser;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const bool smallInventory = m_smallInventory != 0;
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            smallInventory ? dispenser : inventory;
        updateSlotPosition(section, target, state, x, y,
                                             smallInventory ? 3 : 0, 0);
        return target;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        updateSlotPosition(section, hotbar, state, x, y, 0, 0);
        return hotbar;
    }
    if (section == inventory && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        updateSlotPosition(section, dispenser, state, x, y, 3, 0);
        return dispenser;
    }
    if (section == dispenser && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        const IUIScene_AbstractContainerMenu::ESceneSection target =
            m_smallInventory ? hotbar : inventory;
        updateSlotPosition(section, target, state, x, y, -3, 0);
        return target;
    }
    if (section == dispenser && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        updateSlotPosition(section, hotbar, state, x, y, -3, 0);
        return hotbar;
    }
    updateSlotPosition(section, section, state, x, y, 0, 0);
    return section;
}

void UIScene_DispenserMenu::GetPositionOfSection(
    IUIScene_AbstractContainerMenu::ESceneSection section, _UIVec2D* position)
{
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        position->x = static_cast<float>(m_dispenserSlot.getXPos());
        position->y = static_cast<float>(m_dispenserSlot.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
        break;
    default:
        break;
    }
}

void UIScene_DispenserMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    float width;
    float height;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        width = m_dispenserSlot.getWidth();
        height = m_dispenserSlot.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        width = m_inventoryList.getWidth();
        height = m_inventoryList.getHeight();
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        width = m_hotbarList.getWidth();
        height = m_hotbarList.getHeight();
        break;
    default:
        break;
    }
    const int rows = static_cast<int>(getSectionRows(section));
    const int columns = static_cast<int>(getSectionColumns(section));
    const float cellHeight = height / static_cast<float>(rows);
    const float cellWidth = width / static_cast<float>(columns);
    size->y = cellHeight;
    size->x = cellWidth;
    position->x = static_cast<float>(slot % columns) * cellWidth;
    position->y = static_cast<float>(slot / columns) * cellHeight;
}

int UIScene_DispenserMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    int offset = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        offset = 0;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        offset = 9;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        offset = 36;
        break;
    default:
        break;
    }
    return offset;
}

void UIScene_DispenserMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    const int columns = static_cast<int>(getSectionColumns(section));
    UIControl_SlotList* list = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        list = &m_dispenserSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        list = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        list = &m_hotbarList;
        break;
    default:
        break;
    }
    if (wrap)
        list->setHighlightSlot(slot + direction * columns);
}

unsigned int UIScene_DispenserMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
    {
        std::string name("Trap");
        m_dispenserSlot.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_dispenserSlot);
    {
        std::string name("dispenserLabel");
        m_dispenserLabel.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_dispenserLabel);
    return 1;
}

std::wstring UIScene_DispenserMenu::getMoviePath()
{
    return gApp.GetLocalPlayerCount() > 1 ? std::wstring(L"DispenserMenuSplit") :
        std::wstring(L"DispenserMenu");
}

void UIScene_DispenserMenu::handleReload()
{
    Initialize(true, m_playerId, m_menu, true,
                                m_containerSize, IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar,
                                IUIScene_AbstractContainerMenu::ESceneSection_DispenserMax, false);
    m_dispenserSlot.addSlots(0, 9);
}

UIControl* UIScene_DispenserMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    UIControl* control = 0;
    switch (section)
    {
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots:
        control = &m_dispenserSlot;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory:
        control = &m_inventoryList;
        break;
    case IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar:
        control = &m_hotbarList;
        break;
    default:
        break;
    }
    return control;
}

void UIScene_DispenserMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_DispenserMenu::getSceneType()
{
    return EUIScene_DispenserMenu;
}
