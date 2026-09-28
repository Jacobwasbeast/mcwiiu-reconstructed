#include "game/ui/UIScene_ContainerMenu.hpp"
#include "game/core/StringIds.hpp"

#include "game/core/CMinecraftApp.hpp"
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
      m_largeChest(false),
      m_containerList(),
      m_chestLabel()
{

    _ContainerMenuArgs* args = static_cast<_ContainerMenuArgs*>(data);
    m_largeChest = args->second->getContainerSize() > 27;

    initialiseMovie();

    Minecraft* minecraft = Minecraft::GetInstance();
    boost::shared_ptr<InteractionObject> interaction = boost::dynamic_pointer_cast<InteractionObject>(args->second);
    const int typeId = interaction != NULL ? interaction->getTypeId(false) : -1;
    const std::wstring customName = args->second->getCustomName();
    AbstractContainerMenu* menu = NULL;

    if (customName.empty())
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
        m_chestLabel.init(UIString(customName));
    }

    if (typeId == ClientboundContainerOpenPacket::SHULKER_BOX)
        menu = new ShulkerBoxMenu(args->first, args->second,
                                  boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(minecraft->mLocalPlayers[playerId]));
    if (menu == NULL)
        menu = new ChestMenu(args->first, args->second,
                             boost::dynamic_pointer_cast<MultiplayerLocalPlayer>(minecraft->mLocalPlayers[playerId]));

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
}

UIScene_ContainerMenu::~UIScene_ContainerMenu()
{
}

unsigned int UIScene_ContainerMenu::getSectionColumns(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    return section <= IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest ? 9U : 0U;
}

unsigned int UIScene_ContainerMenu::getSectionRows(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        return 1;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        return GetInventoryRows();
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
        return static_cast<unsigned int>((GetBaseSlotCount() - 36) / 9);
    return 0;
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
    UIControl* control = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
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
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        control = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        control = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
        control = &m_containerList;
    if (control != 0 && position != 0 && size != 0)
    {
        const unsigned int columns = getSectionColumns(section);
        const unsigned int rows = getSectionRows(section);
        size->x = static_cast<float>(control->getWidth() / static_cast<float>(columns));
        size->y = static_cast<float>(control->getHeight() / static_cast<float>(rows));
        position->x = static_cast<float>(slot % static_cast<int>(columns)) * size->x;
        position->y = static_cast<float>(slot / static_cast<int>(columns)) * size->y;
    }
}

void UIScene_ContainerMenu::setSectionSelectedSlot(
    IUIScene_AbstractContainerMenu::ESceneSection section, int slot, int direction, bool wrap)
{
    UIControl_SlotList* list = 0;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        list = &m_hotbarList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
        list = &m_inventoryList;
    else if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerChest)
        list = &m_containerList;
    if (wrap && list != 0)
        list->setHighlightSlot(slot + direction * static_cast<int>(getSectionColumns(section)));
}

int UIScene_ContainerMenu::getSectionStartOffset(
    IUIScene_AbstractContainerMenu::ESceneSection section)
{
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar)
        return GetBaseSlotCount() - 9;
    if (section == IUIScene_AbstractContainerMenu::ESceneSection_ContainerInventory)
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
    Initialize(true, m_playerId, m_menu, true, count - 36,
                                IUIScene_AbstractContainerMenu::ESceneSection_ContainerHotbar,
                                IUIScene_AbstractContainerMenu::ESceneSection_ContainerMax, false);
    m_containerList.addSlots(0, count - 36);
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
