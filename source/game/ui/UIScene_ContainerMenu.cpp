#include "game/ui/UIScene_ContainerMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
#include "game/core/CConsoleMinecraftApp.hpp"
#include "game/core/L10N.hpp"
#include "game/core/Minecraft.hpp"
#include "game/minigame/Tutorial.hpp"
#include "game/item/Container.hpp"
#include "game/ui/ChestMenu.hpp"
#include "game/ui/ShulkerBoxMenu.hpp"
#include "game/ui/UIString.hpp"
#include "game/ui/fuiFile.hpp"
#include "game/ui/fuiRenderNode.hpp"
#include "game/world/generation/MultiplayerLocalPlayer.hpp"
#include "game/world/generation/MultiPlayerGameMode.hpp"
#include "game/network/ClientboundContainerOpenPacket.hpp"
#include "game/ui/UILayer.hpp"

namespace
{
const int PLAYER_INVENTORY_SLOTS = 36;

struct _ContainerMenuArgs
{
    boost::shared_ptr<Container> first;
    boost::shared_ptr<Container> second;
    int player;
    bool splitscreen;
};
}

UIScene_ContainerMenu::UIScene_ContainerMenu(int playerId, void* data, UILayer* layer)
    : UIScene_AbstractContainerMenu(playerId, layer),
      m_containerList(),
      m_chestLabel()
{

    _ContainerMenuArgs* args = static_cast<_ContainerMenuArgs*>(data);
    m_largeChest = args->second->getContainerSize() > 27;

    initialiseMovie();

    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<InteractionObject> interaction = boost::dynamic_pointer_cast<InteractionObject>(args->second);
    const int typeId = interaction != NULL ? interaction->getTypeId(false) : -1;
    AbstractContainerMenu* menu = NULL;

    if (args->second->getName().empty())
    {
        if (typeId == ClientboundContainerOpenPacket::BONUS_CHEST)
            m_chestLabel.init(UIString(L10N::GetString(IDS_BONUS_CHEST)));
        else if (typeId == ClientboundContainerOpenPacket::ENDER_CHEST)
            m_chestLabel.init(UIString(L10N::GetString(IDS_TILE_ENDERCHEST)));
        else if (typeId == ClientboundContainerOpenPacket::LARGE_CHEST)
            m_chestLabel.init(UIString(L10N::GetString(IDS_CHEST_LARGE)));
        else if (typeId == ClientboundContainerOpenPacket::MINECART_CHEST)
            m_chestLabel.init(UIString(L10N::GetString(IDS_CONTAINER_MINECART)));
        else if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
            m_chestLabel.init(UIString(L10N::GetString(IDS_CONTAINER_SHULKER_BOX)));
        else
            m_chestLabel.init(UIString(L10N::GetString(IDS_CHEST)));
    }
    else
    {
        m_chestLabel.init(UIString(args->second->getName()));
    }

    if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
        menu = new ShulkerBoxMenu(args->first, args->second,
                                  boost::dynamic_pointer_cast<Player>(minecraft->mLocalPlayers[playerId]));
    if (menu == NULL)
        menu = new ChestMenu(args->first, args->second,
                             boost::dynamic_pointer_cast<Player>(minecraft->mLocalPlayers[playerId]));

    if (minecraft->mGameModes[playerId] != NULL)
    {
        Tutorial* tutorial = Minecraft::GetInstance()->mGameModes[playerId]->getTutorial();
        if (tutorial != NULL)
        {
            m_previousTutorialState = tutorial->currentState;
            tutorial->changeTutorialState(eTutorial_State_ContainerMenu, this);
        }
    }

    const int containerSlots = static_cast<int>(menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
    Initialize(false, args->player, menu, true, containerSlots,
                                 IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
                                 IUIScene_AbstractContainerMenu::ESceneSection_ContainerMax, false);
    m_containerList.addSlots(0, containerSlots);

    delete args;
    m_takeAllEnabled = true;
}

UIScene_ContainerMenu::~UIScene_ContainerMenu()
{
}

unsigned int UIScene_ContainerMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return static_cast<unsigned int>(section) <= IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest ? 9U : 0U;
}

unsigned int UIScene_ContainerMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    unsigned int rows = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        rows = 1;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        rows = GetInventoryRows();
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
        rows = (static_cast<unsigned int>(m_menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS) / 9;
    return rows;
}

unsigned int UIScene_ContainerMenu::GetSectionAndSlotInDirection(
    IUIScene_AbstractContainerMenu::ESceneSection section,
    IUIScene_AbstractContainerMenu::ETapState state, int* x, int* y)
{
    IUIScene_AbstractContainerMenu::ESceneSection result = section;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest,
                               state, x, y, 0, 0);
            return 2;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            result = m_smallInventory ?
                IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest :
                IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory;
            updateSlotPosition(section, result, state, x, y, 0, 0);
            return result;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
                               state, x, y, 0, 0);
            return 0;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest,
                               state, x, y, 0, 0);
            return 2;
        }
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
    {
        if (state == IUIScene_AbstractContainerMenu::ETapState_Down)
        {
            result = m_smallInventory ?
                IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar :
                IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory;
            updateSlotPosition(section, result, state, x, y, 0, 0);
            return result;
        }
        if (state == IUIScene_AbstractContainerMenu::ETapState_Up)
        {
            updateSlotPosition(section, IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
    {
        position->x = static_cast<float>(m_hotbarList.getXPos());
        position->y = static_cast<float>(m_hotbarList.getYPos());
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
    {
        position->x = static_cast<float>(m_inventoryList.getXPos());
        position->y = static_cast<float>(m_inventoryList.getYPos());
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
    {
        position->x = static_cast<float>(m_containerList.getXPos());
        position->y = static_cast<float>(m_containerList.getYPos());
    }
}

void UIScene_ContainerMenu::GetItemScreenData(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot,
    _UIVec2D* position, _UIVec2D* size)
{
    _UIVec2D controlSize;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
    {
        controlSize.x = m_hotbarList.getWidth();
        controlSize.y = m_hotbarList.getHeight();
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
    {
        controlSize.x = m_inventoryList.getWidth();
        controlSize.y = m_inventoryList.getHeight();
    }
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
    {
        controlSize.x = m_containerList.getWidth();
        controlSize.y = m_containerList.getHeight();
    }
    const int rows = getSectionRows(section);
    const int columns = getSectionColumns(section);
    size->x = controlSize.x / static_cast<float>(columns);
    size->y = controlSize.y / static_cast<float>(rows);
    position->x = static_cast<float>(slot % columns) * size->x;
    position->y = static_cast<float>(slot / columns) * size->y;
}

void UIScene_ContainerMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    const int highlightSlot = slot + direction * getSectionColumns(section);
    UIControl_SlotList* list = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        list = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        list = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
        list = &m_containerList;
    if (wrap)
        list->setHighlightSlot(highlightSlot);
}

int UIScene_ContainerMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        return static_cast<int>(m_menu->m_slots.size()) - 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        return static_cast<int>(m_menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
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
    {
        std::string name("containerList");
        m_containerList.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_containerList);
    {
        std::string name("chestLabel");
        m_chestLabel.setupControl(this, panel, name);
    }
    m_controls.push_back(&m_chestLabel);
    (void)root;
    return 1;
}

std::wstring UIScene_ContainerMenu::getMoviePath()
{
    if (m_largeChest)
        return 1 < gApp.GetLocalPlayerCount() ?
            std::wstring(L"ChestLargeMenuSplit") : std::wstring(L"ChestLargeMenu");
    return 1 < gApp.GetLocalPlayerCount() ?
        std::wstring(L"ChestMenuSplit") : std::wstring(L"ChestMenu");
}

void UIScene_ContainerMenu::handleReload()
{
    const int count = static_cast<int>(m_menu->m_slots.size()) - PLAYER_INVENTORY_SLOTS;
    Initialize(true, m_playerId, m_menu, true, count,
                                IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
                                IUIScene_AbstractContainerMenu::ESceneSection_ContainerMax, false);
    m_containerList.addSlots(0, count);
}

UIControl* UIScene_ContainerMenu::getSection(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        return &m_hotbarList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        return &m_inventoryList;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
        return &m_containerList;
    return 0;
}

int UIScene_ContainerMenu::getSceneType()
{
    return EUIScene_ContainerMenu;
}
