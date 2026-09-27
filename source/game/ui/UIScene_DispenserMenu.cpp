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

extern CMinecraftApp* g_pApp;

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
    : UIScene_AbstractContainerMenu(type, layer), m_dispenserSlot(), m_dispenserLabel(), m_sceneInterface(0)
{
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(
        static_cast<UIScene_AbstractContainerMenu*>(this));
    initialiseMovie();

    DispenserMenuArgs* args = static_cast<DispenserMenuArgs*>(data);
    const std::wstring name = args->container->getName();
    if (name.empty())
    {
        boost::shared_ptr<InteractionObject> interaction =
            boost::dynamic_pointer_cast<InteractionObject>(args->container);
        m_dispenserLabel.init(UIString(L10N::GetString(
            interaction && interaction->getTypeId(false) == ClientboundContainerOpenPacket::DROPPER ? IDS_CONTAINER_DROPPER : IDS_DISPENSER)));
    }
    else
    {
        m_dispenserLabel.init(UIString(name));
    }

    m_sceneInterface->m_menu = new TrapMenu(args->inventory, args->container);
    m_containerSize = static_cast<int>(args->container->getContainerSize());
    Minecraft* minecraft = Minecraft::GetInstance();
    if (minecraft->mGameModes[args->player] != 0)
    {
        Tutorial* tutorial = minecraft->mGameModes[args->player]->getTutorial();
        if (tutorial != 0)
        {
            m_sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_DispenserMenu, this);
        }
    }
    m_sceneInterface->Initialize(false, args->player, m_sceneInterface->m_menu, true,
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
        return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
        return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
        return 3;
    return 0;
}

unsigned int UIScene_DispenserMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
        return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
        return IUIScene_AbstractContainerMenu::GetInventoryRows();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
        return 3;
    return 0;
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
        m_sceneInterface->updateSlotPosition(section, dispenser, state, x, y, 3, 0);
        return dispenser;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const bool smallInventory = m_sceneInterface->m_smallInventory != 0;
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
            m_sceneInterface->m_smallInventory ? hotbar : inventory;
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
        control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
        control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
        control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
        control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
        control = &m_dispenserSlot;
    if (control != 0 && position != 0 && size != 0 && columns != 0 && rows != 0)
    {
        size->x = static_cast<float>(control->getWidth() / static_cast<float>(columns));
        size->y = static_cast<float>(control->getHeight() / static_cast<float>(rows));
        position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
        position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
    }
}

int UIScene_DispenserMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
        return 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
        return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
        return 36;
    return 0;
}

void UIScene_DispenserMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    if (wrap)
    {
        if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
            m_hotbarList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
        else if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
            m_inventoryList.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
        else if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
            m_dispenserSlot.setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
    }
}

unsigned int UIScene_DispenserMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    m_movie->getRootNode();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
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
    m_sceneInterface->Initialize(true, m_playerId, m_sceneInterface->m_menu, true,
                                m_containerSize, IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar,
                                IUIScene_AbstractContainerMenu::ESceneSection_DispenserMax, false);
    m_dispenserSlot.addSlots(0, 9);
}

UIControl* UIScene_DispenserMenu::getSection(IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserHotbar)
        return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserInventory)
        return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_DispenserSlots)
        return &m_dispenserSlot;
    return 0;
}

void UIScene_DispenserMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_DispenserMenu::getSceneType()
{
    return EUIScene_DispenserMenu;
}
