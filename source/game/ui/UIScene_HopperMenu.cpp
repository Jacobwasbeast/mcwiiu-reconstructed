#include "game/ui/UIScene_HopperMenu.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/InteractionObject.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/item/Container.hpp"
#include "game/item/Inventory.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/ui/HopperMenu.hpp"
#include "game/ui/UIControl.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/Player.hpp"
#include "game/ui/UILayer.hpp"

extern CMinecraftApp* g_pApp;

namespace
{
struct HopperMenuSceneData
{
    boost::shared_ptr<Inventory> inventory;
    boost::shared_ptr<Container> container;
    int playerIndex;
    bool splitscreen;
};
}

UIScene_HopperMenu::UIScene_HopperMenu(int type, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(type, layer), m_hopperSlot(), m_hopperLabel(), m_sceneInterface(0)
{
    m_sceneInterface = static_cast<IUIScene_AbstractContainerMenu*>(this);
    initialiseMovie();

    HopperMenuSceneData* sceneData = static_cast<HopperMenuSceneData*>(data);
    const std::wstring containerName = sceneData->container->getDisplayName();
    if (containerName.empty())
    {
        boost::shared_ptr<InteractionObject> interaction =
            boost::dynamic_pointer_cast<InteractionObject>(sceneData->container);
        if (interaction == 0 || interaction->getTypeId(false) != ClientboundContainerOpenPacket::MINECART_HOPPER)
            m_hopperLabel.init(UIString(L10N::GetString(IDS_CONTAINER_HOPPER)));
        else
            m_hopperLabel.init(UIString(L10N::GetString(IDS_CONTAINER_MINECART)));
    }
    else
    {
        m_hopperLabel.init(UIString(containerName));
    }

    MultiPlayerGameMode* gameMode = Minecraft::GetInstance()->mGameModes[sceneData->playerIndex];
    if (gameMode != 0)
    {
        Tutorial* tutorial = gameMode->getTutorial();
        if (tutorial != 0)
        {
            m_sceneInterface->m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_HopperMenu, this);
        }
    }

    boost::shared_ptr<Container> playerInventory =
        boost::dynamic_pointer_cast<Container>(sceneData->inventory);
    boost::shared_ptr<Player> player =
        Minecraft::GetInstance()->mLocalPlayers[sceneData->playerIndex];
    HopperMenu* menu = new HopperMenu(playerInventory, sceneData->container, player);
    m_containerSize = static_cast<int>(sceneData->container->getContainerSize());
    m_sceneInterface->Initialize(false, sceneData->playerIndex, menu, true, m_containerSize,
                                 IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_HopperMax, false);
    m_hopperSlot.addSlots(0, 9);
    delete sceneData;
}

UIScene_HopperMenu::~UIScene_HopperMenu()
{
}

unsigned int UIScene_HopperMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        return 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
        return 5;
    return 0;
}

unsigned int UIScene_HopperMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        return GetInventoryRows();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
        return 1;
    return 0;
}

unsigned int UIScene_HopperMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    const IUIScene_AbstractContainerMenu::ESceneSection hotbar =
        IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar;
    const IUIScene_AbstractContainerMenu::ESceneSection inventory =
        IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory;
    const IUIScene_AbstractContainerMenu::ESceneSection hopper =
        IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots;

    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Down)
    {
        m_sceneInterface->updateSlotPosition(section, hopper, state, x, y, 2, 0);
        return hopper;
    }
    if (section == hotbar && state == IUIScene_AbstractContainerMenu::ETapState_Up)
    {
        const bool smallInventory = m_sceneInterface->m_smallInventory != 0;
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
            m_sceneInterface->m_smallInventory ? hotbar : inventory;
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
        control = &m_hopperSlot;
    const unsigned int columns = getSectionColumns(section);
    const unsigned int rows = getSectionRows(section);
    if (control != 0 && position != 0 && size != 0 && columns != 0 && rows != 0)
    {
        size->x = static_cast<float>(control->getWidth() / static_cast<float>(columns));
        size->y = static_cast<float>(control->getHeight() / static_cast<float>(rows));
        position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
        position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
    }
}

void UIScene_HopperMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    UIControl_SlotList* list = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        list = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        list = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
        list = &m_hopperSlot;
    if (wrap && list != 0)
        list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

unsigned int UIScene_HopperMenu::mapElementsAndNames()
{
    UIScene_AbstractContainerMenu::mapElementsAndNames();
    fuiRenderNode* panel = m_mainPanel.getFuiRenderNode();
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
    m_sceneInterface->Initialize(true, m_playerId, m_sceneInterface->m_menu, true,
                                 m_containerSize, IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_HopperMax, false);
    m_hopperSlot.addSlots(0, 9);
}

UIControl* UIScene_HopperMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
        return &m_hopperSlot;
    return 0;
}

void UIScene_HopperMenu::handleSectionClick(
    IUIScene_AbstractContainerMenu::ESceneSection)
{
}

int UIScene_HopperMenu::getSceneType()
{
    return EUIScene_HopperMenu;
}

int UIScene_HopperMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperHotbar)
        return 32;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperInventory)
        return 5;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_HopperSlots)
        return 0;
    return 0;
}
